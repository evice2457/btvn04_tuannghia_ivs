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
    CallbackReturn on_configure(const rclcpp_lifecycle::State &) override{
        publisher_ = this->create_lifecycle_publisher<std_msgs::msg::String>("/status", 10);
        RCLCPP_INFO(get_logger(), "on_configure() done");
        return CallbackReturn::SUCCESS;
    }
    CallbackReturn on_activate(const rclcpp_lifecycle::State &) override{
        publisher_->on_activate();
        timer_ = this->create_wall_timer(1s, std::bind(&MinimalLifecycleNode::timer_callback, this));
        RCLCPP_INFO(get_logger(), "on_activate() done");
        return CallbackReturn::SUCCESS;
    }
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override{
        timer_.reset();
        publisher_->on_deactivate();
        RCLCPP_INFO(get_logger(), "on_deactivate() done");
        return CallbackReturn::SUCCESS;
    }
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override{
        timer_.reset();
        publisher_.reset();
        RCLCPP_INFO(get_logger(), "on_cleanup() done");
        return CallbackReturn::SUCCESS;
    }
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override{
        timer_.reset();
        publisher_.reset();
        RCLCPP_INFO(get_logger(), "on_shutdown() done");
        return CallbackReturn::SUCCESS;
    }
private:

  // timer_callback goes here
    void timer_callback(){
        auto message = std_msgs::msg::String();
        message.data = "Hello, Lifecycle!";
        RCLCPP_INFO(get_logger(), "Publishing: '%s'", message.data.c_str());
        publisher_->publish(message);
    }

  // member variables go here
    rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;

};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<MinimalLifecycleNode>();
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node->get_node_base_interface());
  executor.spin();
  rclcpp::shutdown();
  return 0;
}