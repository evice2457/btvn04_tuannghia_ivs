#include <chrono>
#include <memory>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "ackermann_msgs/msg/ackermann_drive.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "nav_msgs/msg/path.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "ackermann_lifecycle_pkg/ackermann_kinematics.hpp"
#include <cmath>

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class AckermannKinematicsNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  AckermannKinematicsNode()
  : LifecycleNode("ackermann_kinematics_node"),
    x_(0.0), y_(0.0), theta_(0.0)
  {}

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override
  {
    // 1. declare and read parameters
    declare_parameter("wheelbase", 2.5);
    declare_parameter("track_width_front", 1.5);
    double wb = get_parameter("wheelbase").as_double();
    double tw = get_parameter("track_width_front").as_double();
    wheelbase_ = wb;

    // 2. create kinematics object
    kinematics_ = std::make_unique<AckermannKinematics>(wb, tw);

    // 3. create publishers
    wheel_pub_ = create_publisher<std_msgs::msg::Float64MultiArray>("/wheel_commands", 10);
    odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("/odom", 10);
    path_pub_ = create_publisher<nav_msgs::msg::Path>("/path", 10);

    // 4. create subscriber
    cmd_sub_ = create_subscription<ackermann_msgs::msg::AckermannDrive>(
      "/cmd_ackermann", 10,
      std::bind(&AckermannKinematicsNode::cmd_callback, this, std::placeholders::_1));

    // 5. create TF broadcaster
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

    last_time_ = now();
    RCLCPP_INFO(get_logger(), "on_configure() done");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override
  {
    wheel_pub_->on_activate();
    odom_pub_->on_activate();
    path_pub_->on_activate();
    RCLCPP_INFO(get_logger(), "on_activate() done");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override
  {
    wheel_pub_->on_deactivate();
    odom_pub_->on_deactivate();
    path_pub_->on_deactivate();
    RCLCPP_INFO(get_logger(), "on_deactivate() done");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override
  {
    wheel_pub_.reset();
    odom_pub_.reset();
    path_pub_.reset();
    cmd_sub_.reset();
    tf_broadcaster_.reset();
    kinematics_.reset();
    path_.poses.clear();
    RCLCPP_INFO(get_logger(), "on_cleanup() done");
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override
  {
    // YOU WRITE THIS — same pattern as on_cleanup
    wheel_pub_.reset();
    odom_pub_.reset();
    path_pub_.reset();
    cmd_sub_.reset();
    tf_broadcaster_.reset();
    kinematics_.reset();
    path_.poses.clear();
    RCLCPP_INFO(get_logger(), "on_shutdown() done");
    return CallbackReturn::SUCCESS;
  }

private:
  void cmd_callback(const ackermann_msgs::msg::AckermannDrive::SharedPtr msg)
  {
    // 1. compute dt
    auto now_time = now();
    double dt = (now_time - last_time_).seconds();
    last_time_ = now_time;

    double phi = msg->steering_angle;
    double v = msg->speed;

    // 2. compute wheel angles using kinematics_ and publish to wheel_pub_
    // YOU WRITE THIS
    auto wheel_cmd = std_msgs::msg::Float64MultiArray();
    wheel_cmd.data = {
      kinematics_->computeLeftSteeringAngle(phi),
      kinematics_->computeRightSteeringAngle(phi)
    };
    wheel_pub_->publish(wheel_cmd);

    // 3. compute yaw rate: omega = v/wheelbase * tan(phi)
    // YOU WRITE THIS — use kinematics_ member or compute directly
    double omega = (v / wheelbase_) * std::tan(phi);
    x_ += v * std::cos(theta_) * dt;
    y_ += v * std::sin(theta_) * dt;
    theta_ += omega * dt;

    // 4. integrate odometry
    // YOU WRITE THIS — use the formulas: x+=v*cos(theta)*dt, y+=v*sin(theta)*dt, theta+=omega*dt

    // 5. publish odom and TF
    publish_odom(now_time);
    publish_tf(now_time);

    // 6. append to path and publish
    geometry_msgs::msg::PoseStamped pose;
    pose.header.stamp = now_time;
    pose.header.frame_id = "odom";
    pose.pose.position.x = x_;
    pose.pose.position.y = y_;
    path_.header.frame_id = "odom";
    path_.poses.push_back(pose);
    path_pub_->publish(path_);
  }

  void publish_odom(const rclcpp::Time & t)
  {
    nav_msgs::msg::Odometry odom;
    odom.header.stamp = t;
    odom.header.frame_id = "odom";
    odom.child_frame_id = "base_link";
    odom.pose.pose.position.x = x_;
    odom.pose.pose.position.y = y_;
    // YOU WRITE THIS — set the quaternion orientation from theta_
    // hint: for 2D, only z and w matter:
    // z = sin(theta/2), w = cos(theta/2)
    odom.pose.pose.orientation.z = std::sin(theta_ / 2.0);
    odom.pose.pose.orientation.w = std::cos(theta_ / 2.0);
    odom_pub_->publish(odom);
  }

  void publish_tf(const rclcpp::Time & t)
  {
    geometry_msgs::msg::TransformStamped tf;
    tf.header.stamp = t;
    tf.header.frame_id = "odom";
    tf.child_frame_id = "base_link";
    tf.transform.translation.x = x_;
    tf.transform.translation.y = y_;
    // YOU WRITE THIS — same quaternion as odom
    tf.transform.rotation.z = std::sin(theta_ / 2.0);
    tf.transform.rotation.w = std::cos(theta_ / 2.0);
    tf_broadcaster_->sendTransform(tf);
  }

  // member variables
  std::unique_ptr<AckermannKinematics> kinematics_;
  rclcpp::Subscription<ackermann_msgs::msg::AckermannDrive>::SharedPtr cmd_sub_;
  rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_pub_;
  rclcpp_lifecycle::LifecyclePublisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp_lifecycle::LifecyclePublisher<nav_msgs::msg::Path>::SharedPtr path_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  double x_, y_, theta_;
  double wheelbase_;
  rclcpp::Time last_time_;
  nav_msgs::msg::Path path_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<AckermannKinematicsNode>();
  rclcpp::executors::SingleThreadedExecutor executor;
  executor.add_node(node->get_node_base_interface());
  executor.spin();
  rclcpp::shutdown();
  return 0;
}
