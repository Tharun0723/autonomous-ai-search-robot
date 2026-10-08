#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "robot_core/srv/navigate_to_location.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"

#include <yaml-cpp/yaml.h>

#include <functional>
#include <map>
#include <string>
#include <cmath>

class LocationManager : public rclcpp::Node
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

    rclcpp::Service<robot_core::srv::NavigateToLocation>::SharedPtr
        navigate_service_;

    rclcpp_action::Client<NavigateToPose>::SharedPtr
        nav2_action_client_;

public:

    LocationManager()
        : Node("location_manager")
    {
        this->declare_parameter<std::string>("locations_file", "");

        const std::string locations_file =
            this->get_parameter("locations_file").as_string();

        RCLCPP_INFO(
            this->get_logger(),
            "Location Manager: ONLINE"
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Loading locations from: %s",
            locations_file.c_str()
        );

        try
        {
            YAML::Node config = YAML::LoadFile(locations_file);

            const YAML::Node locations = config["locations"];

            for (const auto &location : locations)
            {
                const std::string name =
                    location.first.as<std::string>();

                const double x =
                    location.second["x"].as<double>();

                const double y =
                    location.second["y"].as<double>();

                const double yaw =
                    location.second["yaw"].as<double>();

                locations_[name] = Location{x, y, yaw};

                RCLCPP_INFO(
                    this->get_logger(),
                    "Location: %s | x: %.4f | y: %.4f | yaw: %.4f",
                    name.c_str(),
                    x,
                    y,
                    yaw
                );
            }
        }
        catch (const YAML::Exception &exception)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Failed to load locations.yaml: %s",
                exception.what()
            );
        }

        nav2_action_client_ =
            rclcpp_action::create_client<NavigateToPose>(
                this,
                "navigate_to_pose"
            );

        navigate_service_ =
            this->create_service<robot_core::srv::NavigateToLocation>(
                "navigate_to_location",
                std::bind(
                    &LocationManager::handle_navigate_request,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2
                )
            );

        RCLCPP_INFO(
            this->get_logger(),
            "Navigate-to-location service: READY"
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Nav2 action client: READY"
        );
    }

private:

    void handle_navigate_request(
        const std::shared_ptr<
            robot_core::srv::NavigateToLocation::Request
        > request,
        std::shared_ptr<
            robot_core::srv::NavigateToLocation::Response
        > response)
    {
        const std::string location_name =
            request->location_name;

        auto it = locations_.find(location_name);

        if (it == locations_.end())
        {
            response->success = false;

            response->message =
                "Unknown location: " + location_name;

            RCLCPP_WARN(
                this->get_logger(),
                "Navigation request rejected: %s",
                location_name.c_str()
            );

            return;
        }

        const Location &location = it->second;

        RCLCPP_INFO(
            this->get_logger(),
            "Navigation request received: %s",
            location_name.c_str()
        );

        RCLCPP_INFO(
            this->get_logger(),
            "Target coordinates: x=%.4f | y=%.4f | yaw=%.4f",
            location.x,
            location.y,
            location.yaw
        );

        if (!nav2_action_client_->wait_for_action_server(
                std::chrono::seconds(2)))
        {
            response->success = false;
            response->message =
                "Nav2 NavigateToPose action server is not available.";

            RCLCPP_ERROR(
                this->get_logger(),
                "Nav2 action server is not available."
            );

            return;
        }

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
            "Sending goal to Nav2: %s",
            location_name.c_str()
        );

        rclcpp_action::Client<NavigateToPose>::SendGoalOptions
            send_goal_options;

        send_goal_options.goal_response_callback =
            std::bind(
                &LocationManager::goal_response_callback,
                this,
                std::placeholders::_1
            );

        send_goal_options.feedback_callback =
            std::bind(
                &LocationManager::feedback_callback,
                this,
                std::placeholders::_1,
                std::placeholders::_2
            );

        send_goal_options.result_callback =
            std::bind(
                &LocationManager::result_callback,
                this,
                std::placeholders::_1
            );

        nav2_action_client_->async_send_goal(
            goal,
            send_goal_options
        );

        response->success = true;
        response->message =
            "Navigation goal sent to " + location_name;
    }

    void goal_response_callback(
        const GoalHandleNavigateToPose::SharedPtr &goal_handle)
    {
        if (!goal_handle)
        {
            RCLCPP_ERROR(
                this->get_logger(),
                "Nav2 rejected the navigation goal."
            );

            return;
        }

        RCLCPP_INFO(
            this->get_logger(),
            "Nav2 accepted the navigation goal."
        );
    }

    void feedback_callback(
        GoalHandleNavigateToPose::SharedPtr,
        const std::shared_ptr<
            const NavigateToPose::Feedback
        > feedback)
    {
        RCLCPP_INFO_THROTTLE(
            this->get_logger(),
            *this->get_clock(),
            2000,
            "Navigation progress: %.2f m remaining",
            feedback->distance_remaining
        );
    }

    void result_callback(
        const GoalHandleNavigateToPose::WrappedResult &result)
    {
        switch (result.code)
        {
            case rclcpp_action::ResultCode::SUCCEEDED:

                RCLCPP_INFO(
                    this->get_logger(),
                    "Navigation completed successfully."
                );

                break;

            case rclcpp_action::ResultCode::ABORTED:

                RCLCPP_ERROR(
                    this->get_logger(),
                    "Navigation was aborted."
                );

                break;

            case rclcpp_action::ResultCode::CANCELED:

                RCLCPP_WARN(
                    this->get_logger(),
                    "Navigation was canceled."
                );

                break;

            default:

                RCLCPP_ERROR(
                    this->get_logger(),
                    "Unknown navigation result."
                );

                break;
        }
    }
};


int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node =
        std::make_shared<LocationManager>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
