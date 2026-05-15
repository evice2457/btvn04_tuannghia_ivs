# ackermann_lifecycle_pkg

ROS2 package minh họa **Lifecycle Node** và mô hình động học **Ackermann (bicycle model)**.
Bao gồm node lifecycle tối giản, node tính toán kinematics + odometry, một system manager để điều khiển vòng đời từ xa qua service, và các script Python so sánh Ackermann với differential drive cùng mô phỏng nhiễu cảm biến.

---

## 1. Cấu trúc thư mục

```text
btvn04_tuannghia_ivs/
├── CMakeLists.txt
├── package.xml
├── include/
│   └── ackermann_lifecycle_pkg/
│       └── ackermann_kinematics.hpp        # Class AckermannKinematics (header-only)
├── src/
│   ├── minimal_lifecycle_node.cpp          # Node lifecycle tối giản (Bài 1)
│   ├── ackermann_kinematics_node.cpp       # Node kinematics + odometry + TF
│   └── ackermann_system_manager.cpp        # Service /system/start, /system/stop
├── scripts/
│   ├── compare_kinematics.py               # So sánh Ackermann vs Diff Drive
│   ├── odometry_noise.py                   # Mô phỏng nhiễu Gaussian trên odometry
│   ├── kinematics_comparison.csv / .png    # Kết quả đã chạy sẵn
│   └── odometry_noise.png
└── test/
    └── test_ackermann_kinematics.cpp       # GTest cho AckermannKinematics
```

---

## 2. Dependencies

- ROS2 (Humble hoặc mới hơn)
- `rclcpp`, `rclcpp_lifecycle`, `lifecycle_msgs`
- `ackermann_msgs`, `nav_msgs`, `geometry_msgs`, `tf2_ros`
- `std_msgs`, `std_srvs`
- Python 3 + `matplotlib` (cho script mô phỏng)

---

## 3. Build

Đặt package vào thư mục `src/` của workspace ROS2, sau đó:

```bash
cd ~/ros2_ws
colcon build --packages-select ackermann_lifecycle_pkg
source install/setup.bash
```

---

## 4. Các node và cách chạy

### 4.1. `minimal_lifecycle_node`

Node lifecycle cơ bản, publish chuỗi `"Hello, Lifecycle!"` lên topic `/status` với chu kỳ 1s (chỉ khi ở trạng thái `active`).

```bash
ros2 run ackermann_lifecycle_pkg minimal_lifecycle_node
```

Điều khiển vòng đời thủ công:

```bash
ros2 lifecycle set /minimal_lifecycle_node configure
ros2 lifecycle set /minimal_lifecycle_node activate
ros2 lifecycle set /minimal_lifecycle_node deactivate
ros2 lifecycle set /minimal_lifecycle_node cleanup
```

### 4.2. `ackermann_kinematics_node`

Lifecycle node nhận lệnh `AckermannDrive`, tính:

- Góc lái trái/phải theo công thức Ackermann (header [ackermann_kinematics.hpp](include/ackermann_lifecycle_pkg/ackermann_kinematics.hpp))
- Tích phân odometry (`x`, `y`, `theta`)
- Phát TF `odom → base_link`

**Parameters:**

| Tên                 | Mặc định | Mô tả                        |
| ------------------- | -------- | ---------------------------- |
| `wheelbase`         | `2.5` m  | Khoảng cách 2 trục           |
| `track_width_front` | `1.5` m  | Khoảng cách 2 bánh trước     |

**Topics:**

| Hướng | Topic              | Loại message                                            |
| ----- | ------------------ | ------------------------------------------------------- |
| Sub   | `/cmd_ackermann`   | `ackermann_msgs/AckermannDrive`                         |
| Pub   | `/wheel_commands`  | `std_msgs/Float64MultiArray` `[phi_left, phi_right]`    |
| Pub   | `/odom`            | `nav_msgs/Odometry`                                     |
| Pub   | `/path`            | `nav_msgs/Path`                                         |
| TF    | `odom → base_link` | —                                                       |

```bash
ros2 run ackermann_lifecycle_pkg ackermann_kinematics_node
```

### 4.3. `ackermann_system_manager`

Node thường (không lifecycle), bọc các transition của `ackermann_kinematics_node` thành 2 service `Trigger`:

| Service          | Tác dụng                                                                |
| ---------------- | ----------------------------------------------------------------------- |
| `/system/start`  | `configure` → `activate`, publish `"active"` lên `/system/status`       |
| `/system/stop`   | `deactivate` → `cleanup`, publish `"inactive"`                          |

```bash
ros2 run ackermann_lifecycle_pkg ackermann_system_manager
ros2 service call /system/start std_srvs/srv/Trigger
ros2 service call /system/stop  std_srvs/srv/Trigger
```

---

## 5. Ví dụ điều khiển

Mở 3 terminal:

```bash
# T1
ros2 run ackermann_lifecycle_pkg ackermann_kinematics_node

# T2
ros2 run ackermann_lifecycle_pkg ackermann_system_manager
ros2 service call /system/start std_srvs/srv/Trigger

# T3 — gửi lệnh chạy thẳng v=1 m/s, lái 0.2 rad
ros2 topic pub /cmd_ackermann ackermann_msgs/msg/AckermannDrive \
  "{speed: 1.0, steering_angle: 0.2}" -r 10
```

Quan sát:

```bash
ros2 topic echo /odom
ros2 topic echo /wheel_commands
ros2 run rviz2 rviz2     # Add Path + TF
```

---

## 6. Script Python (offline)

```bash
cd scripts/
python3 compare_kinematics.py    # → kinematics_comparison.csv + .png
python3 odometry_noise.py        # → odometry_noise.png
```

- [compare_kinematics.py](scripts/compare_kinematics.py) — Mô phỏng 10s profile S-curve, so sánh quỹ đạo Ackermann vs differential drive.
- [odometry_noise.py](scripts/odometry_noise.py) — Thêm nhiễu Gaussian (σ = 0.001, 0.01, 0.05) lên encoder vận tốc và góc lái, chạy 5 trials/mức để thấy độ trôi.

---

## 7. Unit tests

```bash
colcon test --packages-select ackermann_lifecycle_pkg
colcon test-result --verbose
```

Các test trong [test_ackermann_kinematics.cpp](test/test_ackermann_kinematics.cpp):

- `ZeroSteeringAngle` — đi thẳng → góc lái 2 bánh = 0
- `LeftTurn` — rẽ trái → góc bánh trái > bánh phải
- `Symmetry` — đối xứng trái/phải qua dấu của `phi`

---

## 8. Công thức Ackermann sử dụng

Với bán kính quay `R = L / tan(φ)` (L = wheelbase, φ = góc lái trung tâm):

```text
φ_left  = atan( L / (R − t/2) )
φ_right = atan( L / (R + t/2) )
ω       = v / L · tan(φ)
```

Tích phân odometry:

```text
x     += v · cos(θ) · dt
y     += v · sin(θ) · dt
θ     += ω · dt
```

---

## Maintainer

evice2457 — `trantuannghia2457@gmail.com`
