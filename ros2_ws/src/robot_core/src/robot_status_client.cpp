#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "robot_core/srv/get_robot_status.hpp"

class RobotStatusClient : public rclcpp::Node
{
public:
    RobotStatusClient()
    : Node("robot_status_client")
    {
        client_ = this->create_client<robot_core::srv::GetRobotStatus>(
            "/get_robot_status");

        RCLCPP_INFO(
            this->get_logger(),
            "Robot Status Client started");

        send_request();    
    }

private:
    void send_request()
    {
        while (!client_->wait_for_service(std::chrono::seconds(1)))
        {
            if (!rclcpp::ok())
            {
                RCLCPP_ERROR(
                    this->get_logger(),
                    "ROS 2 shutdown detected");
                return;
            }

            RCLCPP_INFO(
                this->get_logger(),
                "Waiting for /get_robot_status service...");
        }

        auto request =
            std::make_shared<robot_core::srv::GetRobotStatus::Request>();

            auto future = client_->async_send_request(
                request,
                [this](rclcpp::Client<robot_core::srv::GetRobotStatus>::SharedFuture future)
                {
                    auto response = future.get();
            
                    RCLCPP_INFO(
                        this->get_logger(),
                        "Service response received: success=%s, status=%s",
                        response->success ? "true" : "false",
                        response->status.c_str());
            
                    rclcpp::shutdown();
                });

        RCLCPP_INFO(
            this->get_logger(),
            "Service request sent");
    }

    rclcpp::Client<robot_core::srv::GetRobotStatus>::SharedPtr client_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<RobotStatusClient>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}