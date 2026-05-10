#include <chrono>
#include <memory>
#include <string>
#include <rclcpp/rclcpp.hpp>
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;
using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;




class MinimalLifecycleNode : public rclcpp_lifecycle::LifecycleNode
{
public:

  // constructor goes here
    MinimalLifecycleNode() : LifecycleNode("minimal_lifecycle_node")
    {
      RCLCPP_INFO(get_logger(), "Node created. State: Unconfigured");
    }
  // on_configure, on_activate, on_deactivate, on_cleanup, on_shutdown go here
    CallbackReturn on_configure(const rclcpp_lifecycle::State &)
    {
        publisher_ = this->create_lifecycle_publisher<std_msgs::msg::String>("/status", 10);

        timer_ = this->create_wall_timer(1s, std::bind(&MinimalLifecycleNode::timer_callback, this));
        RCLCPP_INFO(get_logger(), "on_conffigure() done");
        return CallbackReturn::SUCCESS;
    }
    CallbackReturn on_activate(const rclcpp_lifecycle::State & State){
        publisher_->on_active()
    })
private:

  // timer_callback goes here

  // member variables go here
    rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

};
