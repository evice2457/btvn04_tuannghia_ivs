#!/bin/bash
# Lab 4 — S-curve demo: trái 90° → phải 180° → trái 90°
# Kết quả: hình chữ S lớn, robot kết thúc cùng hướng ban đầu

source ~/ros2_ws/install/setup.bash

echo "[1/5] Đi thẳng 3s..."
ros2 topic pub --times 30 /cmd_ackermann ackermann_msgs/msg/AckermannDrive \
  "{speed: 1.0, steering_angle: 0.0}" -r 10

echo "[2/5] Rẽ TRÁI 90° (5.7s)..."
ros2 topic pub --times 57 /cmd_ackermann ackermann_msgs/msg/AckermannDrive \
  "{speed: 1.0, steering_angle: 0.6}" -r 10

echo "[3/5] Rẽ PHẢI 180° (11.5s) — đây là phần giữa chữ S..."
ros2 topic pub --times 115 /cmd_ackermann ackermann_msgs/msg/AckermannDrive \
  "{speed: 1.0, steering_angle: -0.6}" -r 10

echo "[4/5] Rẽ TRÁI 90° (5.7s)..."
ros2 topic pub --times 57 /cmd_ackermann ackermann_msgs/msg/AckermannDrive \
  "{speed: 1.0, steering_angle: 0.6}" -r 10

echo "[5/5] Đi thẳng 3s..."
ros2 topic pub --times 30 /cmd_ackermann ackermann_msgs/msg/AckermannDrive \
  "{speed: 1.0, steering_angle: 0.0}" -r 10

echo "Hoàn thành! Kiểm tra đường path trong RViz2."
