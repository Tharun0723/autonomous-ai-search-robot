#include <chrono>
#include <functional>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "robot_core/action/move_robot.hpp"

using MoveRobot = robot_core::action::MoveRobot;
using GoalHandleMoveRobot = rclcpp_action::ServerGoalHandle<MoveRobot>;

class RobotMoveActionServer : public rclcpp::Node
{
public:
    RobotMoveActionServer()
    : Node("robot_move_action_server")
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Robot Move Action Server started");

        action_server_ = rclcpp_action::create_server<MoveRobot>(
            this,
            "move_robot",
            std::bind(
                &RobotMoveActionServer::handle_goal,
                this,
                std::placeholders::_1,
                std::placeholders::_2),
            std::bind(
                &RobotMoveActionServer::handle_cancel,
                this,
                std::placeholders::_1),
            std::bind(
                &RobotMoveActionServer::handle_accepted,
                this,
                std::placeholders::_1));
    }

private:
    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const MoveRobot::Goal> goal)
    {
        (void)uuid;

        RCLCPP_INFO(
            this->get_logger(),
            "Received goal: %.2f meters",
            goal->target_distance);

        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandleMoveRobot> goal_handle)
    {
        (void)goal_handle;

        RCLCPP_INFO(
            this->get_logger(),
            "Received request to cancel goal");

        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void handle_accepted(
        const std::shared_ptr<GoalHandleMoveRobot> goal_handle)
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Goal accepted");

        std::thread{std::bind(
            &RobotMoveActionServer::execute,
            this,
            std::placeholders::_1),
            goal_handle}.detach();
    }

    void execute(
        const std::shared_ptr<GoalHandleMoveRobot> goal_handle)
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Executing goal");

        auto goal = goal_handle->get_goal();

        auto feedback = std::make_shared<MoveRobot::Feedback>();
        auto result = std::make_shared<MoveRobot::Result>();

        float current_distance = 0.0f;

        while (current_distance < goal->target_distance)
        {
            current_distance += 1.0f;

            feedback->current_distance = current_distance;

            goal_handle->publish_feedback(feedback);

            RCLCPP_INFO(
                this->get_logger(),
                "Current distance: %.2f m",
                current_distance);

            std::this_thread::sleep_for(
                std::chrono::seconds(1));

            if (goal_handle->is_canceling())
            {
                result->success = false;
                result->message = "Goal canceled";

                goal_handle->canceled(result);

                RCLCPP_INFO(
                    this->get_logger(),
                    "Goal canceled");

                return;
            }
        }

        result->success = true;
        result->message = "Goal reached";

        goal_handle->succeed(result);

        RCLCPP_INFO(
            this->get_logger(),
            "Goal succeeded");
    }

    rclcpp_action::Server<MoveRobot>::SharedPtr action_server_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<RobotMoveActionServer>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
