#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "robot_core/action/move_robot.hpp"

using MoveRobot = robot_core::action::MoveRobot;
using GoalHandleMoveRobot = rclcpp_action::ClientGoalHandle<MoveRobot>;

class RobotMoveActionClient : public rclcpp::Node
{
public:
    RobotMoveActionClient()
    : Node("robot_move_action_client")
    {
        this->declare_parameter<double>("target_distance", 5.0);

        target_distance_ =
            this->get_parameter("target_distance").as_double();

        this->declare_parameter<bool>("auto_start_goal", true);

        auto_start_goal_ =
            this->get_parameter("auto_start_goal").as_bool();

        action_client_ =
            rclcpp_action::create_client<MoveRobot>(
                this,
                "move_robot");

        RCLCPP_INFO(
            this->get_logger(),
            "Robot Move Action Client started");

        if (auto_start_goal_)
        {
            send_goal();
        }
    }

    void send_goal();

private:
    void feedback_callback(
        GoalHandleMoveRobot::SharedPtr,
        const std::shared_ptr<const MoveRobot::Feedback> feedback);

    void result_callback(
        const GoalHandleMoveRobot::WrappedResult & result);

    rclcpp_action::Client<MoveRobot>::SharedPtr action_client_;

    double target_distance_;
    bool auto_start_goal_;
};

void RobotMoveActionClient::send_goal()
{
    RCLCPP_INFO(
        this->get_logger(),
        "Waiting for Action Server...");

    if (!action_client_->wait_for_action_server(
            std::chrono::seconds(5)))
    {
        RCLCPP_ERROR(
            this->get_logger(),
            "Action Server not available");

        rclcpp::shutdown();
        return;
    }

    RCLCPP_INFO(
        this->get_logger(),
        "Action Server available");

    MoveRobot::Goal goal_msg;

    goal_msg.target_distance =
        static_cast<float>(target_distance_);

    RCLCPP_INFO(
        this->get_logger(),
        "Sending goal: %.2f meters",
        goal_msg.target_distance);

    auto send_goal_options =
        rclcpp_action::Client<MoveRobot>::SendGoalOptions();

    send_goal_options.goal_response_callback =
        [this](const GoalHandleMoveRobot::SharedPtr & goal_handle)
        {
            if (!goal_handle)
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "Goal was rejected by the Action Server");

                return;
            }

            RCLCPP_INFO(
                this->get_logger(),
                "Goal accepted by the Action Server");
        };

    send_goal_options.feedback_callback =
        [this](
            GoalHandleMoveRobot::SharedPtr,
            const std::shared_ptr<const MoveRobot::Feedback> feedback)
        {
            this->feedback_callback(
                nullptr,
                feedback);
        };

    send_goal_options.result_callback =
        [this](const GoalHandleMoveRobot::WrappedResult & result)
        {
            this->result_callback(result);
        };

    action_client_->async_send_goal(
        goal_msg,
        send_goal_options);
}

void RobotMoveActionClient::feedback_callback(
    GoalHandleMoveRobot::SharedPtr,
    const std::shared_ptr<const MoveRobot::Feedback> feedback)
{
    RCLCPP_INFO(
        this->get_logger(),
        "Feedback: current distance = %.2f m",
        feedback->current_distance);
}

void RobotMoveActionClient::result_callback(
    const GoalHandleMoveRobot::WrappedResult & result)
{
    switch (result.code)
    {
        case rclcpp_action::ResultCode::SUCCEEDED:
            RCLCPP_INFO(
                this->get_logger(),
                "Goal succeeded");
            break;

        case rclcpp_action::ResultCode::ABORTED:
            RCLCPP_ERROR(
                this->get_logger(),
                "Goal aborted");
            break;

        case rclcpp_action::ResultCode::CANCELED:
            RCLCPP_WARN(
                this->get_logger(),
                "Goal canceled");
            break;

        default:
            RCLCPP_ERROR(
                this->get_logger(),
                "Unknown result code");
            break;
    }

    RCLCPP_INFO(
        this->get_logger(),
        "Result: success=%s, message=%s",
        result.result->success ? "true" : "false",
        result.result->message.c_str());

    rclcpp::shutdown();
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node =
        std::make_shared<RobotMoveActionClient>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}