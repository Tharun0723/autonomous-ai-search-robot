
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"

#include <yaml-cpp/yaml.h>

#include <chrono>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class MissionController : public rclcpp::Node
{
    using NavigateToPose = nav2_msgs::action::NavigateToPose;

    using GoalHandleNavigateToPose =
        rclcpp_action::ClientGoalHandle<NavigateToPose>;

    struct Location
    {
        double x;
        double y;
        double yaw;
    };

    std::map<std::string, Location> locations_;
    std::vector<std::string> mission_locations_;

    std::string mission_name_;
    bool stop_on_failure_{true};
    bool configuration_valid_{false};
    bool mission_finished_{false};

    std::size_t current_index_{0};

    rclcpp_action::Client<NavigateToPose>::SharedPtr nav_client_;

    rclcpp::TimerBase::SharedPtr startup_timer_;

public:

    MissionController()
        : Node("mission_controller")
    {
        this->declare_parameter<std::string>("mission_file", "");
        this->declare_parameter<std::string>("locations_file", "");

        const auto mission_file =
            this->get_parameter("mission_file").as_string();

        const auto locations_file =
            this->get_parameter("locations_file").as_string();

        RCLCPP_INFO(
            this->get_logger(),
            "Mission Controller: INITIALIZING"
        );

        if (!load_configuration(mission_file, locations_file))
        {
            RCLCPP_FATAL(
                this->get_logger(),
                "Mission configuration is invalid. Mission will not start."
            );
            return;
        }

        nav_client_ =
            rclcpp_action::create_client<NavigateToPose>(
                this,
                "navigate_to_pose"
            );

        RCLCPP_INFO(
            this->get_logger(),
            "Mission: %s",
            mission_name_.c_str()
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Validated %zu mission locations.",
            mission_locations_.size()
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Waiting for Nav2 action server..."
        );

        startup_timer_ = this->create_wall_timer(
            std::chrono::seconds(1),
            std::bind(
                &MissionController::check_nav2_ready,
                this
            )
        );
    }

private:

    bool load_configuration(
        const std::string &mission_file,
        const std::string &locations_file)
    {
        try
        {
            const YAML::Node mission_config =
                YAML::LoadFile(mission_file);

            const YAML::Node mission =
                mission_config["mission"];

            if (!mission || !mission["name"] ||
                !mission["locations"] ||
                !mission["stop_on_failure"])
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "Mission YAML is missing required fields."
                );
                return false;
            }

            mission_name_ = mission["name"].as<std::string>();

            stop_on_failure_ =
                mission["stop_on_failure"].as<bool>();

            for (const auto &item : mission["locations"])
            {
                mission_locations_.push_back(
                    item.as<std::string>()
                );
            }

            if (mission_locations_.empty())
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "Mission contains no locations."
                );
                return false;
            }

            const YAML::Node location_config =
                YAML::LoadFile(locations_file);

            const YAML::Node location_list =
                location_config["locations"];

            if (!location_list || !location_list.IsMap())
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "Locations YAML has no valid locations map."
                );
                return false;
            }

            for (const auto &item : location_list)
            {
                const std::string name =
                    item.first.as<std::string>();

                const YAML::Node data = item.second;

                if (!data["x"] || !data["y"] || !data["yaw"])
                {
                    RCLCPP_ERROR(
                        this->get_logger(),
                        "Location '%s' is missing x, y or yaw.",
                        name.c_str()
                    );
                    return false;
                }

                locations_[name] = Location{
                    data["x"].as<double>(),
                    data["y"].as<double>(),
                    data["yaw"].as<double>()
                };
            }

            // Validate every destination before moving the robot.
            for (const auto &name : mission_locations_)
            {
                if (locations_.find(name) == locations_.end())
                {
                    RCLCPP_ERROR(
                        this->get_logger(),
                        "Unknown mission location: %s",
                        name.c_str()
                    );
                    return false;
                }
            }

            configuration_valid_ = true;

            return true;
        }
        catch (const YAML::Exception &exception)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "YAML configuration error: %s",
                exception.what()
            );
        }
        catch (const std::exception &exception)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Configuration error: %s",
                exception.what()
            );
        }

        return false;
    }

    void check_nav2_ready()
    {
        if (!configuration_valid_ || mission_finished_)
        {
            startup_timer_->cancel();
            return;
        }

        if (!nav_client_->action_server_is_ready())
        {
            return;
        }

        startup_timer_->cancel();

        RCLCPP_INFO(
            this->get_logger(),
            "Nav2 is ready. Starting mission."
        );

        send_next_goal();
    }

    void send_next_goal()
    {
        if (mission_finished_)
        {
            return;
        }

        if (current_index_ >= mission_locations_.size())
        {
            finish_mission(true);
            return;
        }

        const std::string &name =
            mission_locations_[current_index_];

        const Location &location = locations_.at(name);

        NavigateToPose::Goal goal;

        goal.pose.header.frame_id = "map";
        goal.pose.header.stamp = this->now();

        goal.pose.pose.position.x = location.x;
        goal.pose.pose.position.y = location.y;
        goal.pose.pose.position.z = 0.0;

        goal.pose.pose.orientation.x = 0.0;
        goal.pose.pose.orientation.y = 0.0;

        goal.pose.pose.orientation.z =
            std::sin(location.yaw / 2.0);

        goal.pose.pose.orientation.w =
            std::cos(location.yaw / 2.0);

        RCLCPP_INFO(
            this->get_logger(),
            "MISSION [%zu/%zu]: Navigating to %s",
            current_index_ + 1,
            mission_locations_.size(),
            name.c_str()
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Target: x=%.3f, y=%.3f, yaw=%.3f",
            location.x,
            location.y,
            location.yaw
        );

        rclcpp_action::Client<NavigateToPose>::SendGoalOptions options;

        options.goal_response_callback =
            std::bind(
                &MissionController::goal_response_callback,
                this,
                std::placeholders::_1
            );

        options.feedback_callback =
            std::bind(
                &MissionController::feedback_callback,
                this,
                std::placeholders::_1,
                std::placeholders::_2
            );

        options.result_callback =
            std::bind(
                &MissionController::result_callback,
                this,
                std::placeholders::_1
            );

        nav_client_->async_send_goal(goal, options);
    }

    void goal_response_callback(
        const GoalHandleNavigateToPose::SharedPtr &goal_handle)
    {
        if (!goal_handle)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Nav2 rejected the current goal."
            );

            handle_failure("Navigation goal rejected");
            return;
        }

        RCLCPP_INFO(
            this->get_logger(),
            "Nav2 accepted the goal."
        );
    }

    void feedback_callback(
        GoalHandleNavigateToPose::SharedPtr,
        const std::shared_ptr<const NavigateToPose::Feedback> feedback)
    {
        RCLCPP_INFO_THROTTLE(
            this->get_logger(),
            *this->get_clock(),
            2000,
            "Distance remaining: %.2f m",
            feedback->distance_remaining
        );
    }

    void result_callback(
        const GoalHandleNavigateToPose::WrappedResult &result)
    {
        if (mission_finished_)
        {
            return;
        }

        switch (result.code)
        {
            case rclcpp_action::ResultCode::SUCCEEDED:
            {
                const std::string completed_location =
                    mission_locations_[current_index_];

                RCLCPP_INFO(
                    this->get_logger(),
                    "SUCCESS: Reached %s",
                    completed_location.c_str()
                );

                ++current_index_;
                send_next_goal();
                break;
            }

            case rclcpp_action::ResultCode::ABORTED:
                handle_failure("Navigation aborted");
                break;

            case rclcpp_action::ResultCode::CANCELED:
                handle_failure("Navigation canceled");
                break;

            default:
                handle_failure("Unknown navigation result");
                break;
        }
    }

    void handle_failure(const std::string &reason)
    {
        if (mission_finished_)
        {
            return;
        }

        const std::string failed_location =
            mission_locations_[current_index_];

        RCLCPP_ERROR(
            this->get_logger(),
            "FAILED at %s: %s",
            failed_location.c_str(),
            reason.c_str()
        );

        if (stop_on_failure_)
        {
            finish_mission(false);
            return;
        }

        RCLCPP_WARN(
            this->get_logger(),
            "stop_on_failure is false. Skipping to the next location."
        );

        ++current_index_;
        send_next_goal();
    }

    void finish_mission(bool success)
    {
        mission_finished_ = true;

        if (success)
        {
            RCLCPP_INFO(
                this->get_logger(),
                "======================================"
            );

            RCLCPP_INFO(
                this->get_logger(),
                "MISSION COMPLETED SUCCESSFULLY"
            );

            RCLCPP_INFO(
                this->get_logger(),
                "All %zu locations visited.",
                mission_locations_.size()
            );

            RCLCPP_INFO(
                this->get_logger(),
                "======================================"
            );
        }
        else
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "MISSION STOPPED. Check the errors above."
            );
        }
    }
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<MissionController>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
