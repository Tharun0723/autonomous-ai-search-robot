#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "robot_core/srv/get_robot_status.hpp"

class RobotStatusServer : public rclcpp::Node
{
public:
    RobotStatusServer()
    : Node("robot_status_server")
    {
        service_ = this->create_service<robot_core::srv::GetRobotStatus>(
            "/get_robot_status",
            std::bind(
                &RobotStatusServer::handle_status,
                this,
                std::placeholders::_1,
                std::placeholders::_2));

        RCLCPP_INFO(
            this->get_logger(),
            "Robot Status Service Server started");
    }

private:
    void handle_status(
        const std::shared_ptr<robot_core::srv::GetRobotStatus::Request> request,
        std::shared_ptr<robot_core::srv::GetRobotStatus::Response> response)
    {
        (void)request;

        response->success = true;
        response->status = "ONLINE";

        RCLCPP_INFO(
            this->get_logger(),
            "Status request received -> ONLINE");
    }

    rclcpp::Service<robot_core::srv::GetRobotStatus>::SharedPtr service_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<RobotStatusServer>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
