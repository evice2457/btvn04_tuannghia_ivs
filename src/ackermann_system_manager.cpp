#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "lifecycle_msgs/srv/change_state.hpp"
#include "lifecycle_msgs/srv/get_state.hpp"
#include "lifecycle_msgs/msg/transition.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_srvs/srv/trigger.hpp"

class AckermannSystemManager : public rclcpp::Node
{
public:
  AckermannSystemManager()
  : rclcpp::Node("ackermann_system_manager")
  {
    // create service clients to control the managed node
    change_state_client_ = create_client<lifecycle_msgs::srv::ChangeState>(
      "/ackermann_kinematics_node/change_state");

    get_state_client_ = create_client<lifecycle_msgs::srv::GetState>(
      "/ackermann_kinematics_node/get_state");

    // create /system/start and /system/stop services
    start_service_ = create_service<std_srvs::srv::Trigger>(
      "/system/start",
      std::bind(
        &AckermannSystemManager::handle_start, this,
        std::placeholders::_1, std::placeholders::_2));

    stop_service_ = create_service<std_srvs::srv::Trigger>(
      "/system/stop",
      std::bind(
        &AckermannSystemManager::handle_stop, this,
        std::placeholders::_1, std::placeholders::_2));

    // create /system/status publisher
    status_pub_ = create_publisher<std_msgs::msg::String>("/system/status", 10);

    RCLCPP_INFO(get_logger(), "AckermannSystemManager ready");
  }

private:
  bool send_transition(uint8_t transition_id)
  {
    auto request = std::make_shared<lifecycle_msgs::srv::ChangeState::Request>();
    request->transition.id = transition_id;

    if (!change_state_client_->wait_for_service(std::chrono::seconds(3))) {
      RCLCPP_ERROR(get_logger(), "change_state service not available");
      return false;
    }

    auto future = change_state_client_->async_send_request(request);
    if (rclcpp::spin_until_future_complete(
        this->get_node_base_interface(), future) !=
      rclcpp::FutureReturnCode::SUCCESS)
    {
      RCLCPP_ERROR(get_logger(), "Failed to call change_state");
      return false;
    }
    return future.get()->success;
  }

  void publish_status(const std::string & status)
  {
    auto msg = std_msgs::msg::String();
    msg.data = status;
    status_pub_->publish(msg);
    RCLCPP_INFO(get_logger(), "Status: %s", status.c_str());
  }

  void handle_start(
    const std_srvs::srv::Trigger::Request::SharedPtr,
    std_srvs::srv::Trigger::Response::SharedPtr response)
  {
    using lifecycle_msgs::msg::Transition;

    if (!send_transition(Transition::TRANSITION_CONFIGURE)) {
      publish_status("configure failed");
      response->success = false;
      response->message = "configure failed";
      return;
    }

    if (!send_transition(Transition::TRANSITION_ACTIVATE)) {
      publish_status("activate failed");
      send_transition(Transition::TRANSITION_CLEANUP);
      response->success = false;
      response->message = "activate failed";
      return;
    }

    publish_status("active");
    response->success = true;
    response->message = "system started";
  }

  void handle_stop(
    const std_srvs::srv::Trigger::Request::SharedPtr,
    std_srvs::srv::Trigger::Response::SharedPtr response)
  {
    using lifecycle_msgs::msg::Transition;

    if (!send_transition(Transition::TRANSITION_DEACTIVATE)) {
      publish_status("deactivate failed");
      response->success = false;
      response->message = "deactivate failed";
      return;
    }

    if (!send_transition(Transition::TRANSITION_CLEANUP)) {
      publish_status("cleanup failed");
      response->success = false;
      response->message = "cleanup failed";
      return;
    }

    publish_status("inactive");
    response->success = true;
    response->message = "system stopped";
  }

  rclcpp::Client<lifecycle_msgs::srv::ChangeState>::SharedPtr change_state_client_;
  rclcpp::Client<lifecycle_msgs::srv::GetState>::SharedPtr get_state_client_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr start_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr stop_service_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr status_pub_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<AckermannSystemManager>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
