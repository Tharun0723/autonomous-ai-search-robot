#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class RobotMonitorNode : public rclcpp::Node
{
public:
    RobotMonitorNode()
    : Node("robot_monitor_node")
    {
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "/robot/status",
            10,
            std::bind(
                &RobotMonitorNode::status_callback,
                this,
                std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "Robot Monitor Node started");
    }

private:
    void status_callback(const std_msgs::msg::String::SharedPtr message)
    {
        RCLCPP_INFO(
            this->get_logger(),
            "Robot Status Received: %s",
            message->data.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    auto node = std::make_shared<RobotMonitorNode>();

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
