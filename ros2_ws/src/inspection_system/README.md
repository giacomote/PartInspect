# Inspection System

A simulated ROS2 and Gazebo environment for automated quality inspection in manufacturing.

## 📜 Author & License

This repository and its files are provided by Giacomo Tessari (`@giacomote` on GitHub), and they are released under the
"GPL v3" license.

    Copyright (C) 2026 Giacomo Tessari

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.

For more details about the license and to view the full terms and conditions, please refer to the [LICENSE](LICENSE)
file.

## 📋 Requirements

Before building and running this package, ensure your system meets the correct ROS2 version:
**Humble Hawksbill** or **Jazzy Jalisco**

### ROS2 Package Dependencies
The system relies on the following official ROS2 packages, which must be installed in your environment:
*   `rclcpp` (ROS2 C++ client library)
*   `std_msgs` (Standard ROS2 message types)
*   `sensor_msgs` (Interface for camera and sensor data)
*   `image_transport` (Optimized image publication and subscription)
*   `cv_bridge` (Bridge between ROS2 image messages and OpenCV)

You can automatically resolve and install any missing ROS2 dependencies by running `rosdep` from the root of your
workspace:

```bash
rosdep install --from-paths src --ignore-src -r -y
```

### Third-Party Libraries
- **OpenCV 4.x** (Core computer vision library, used for image binarization and thresholding)

## 🧪 Installing and Testing the Package

To install and test this ROS2 package, you need to:
1. Place the package into your ROS2 workspace

2. Compile it via `colcon`:

```bash
>> cd ~/ros2_ws
>> colcon build --packages-select inspection_system
>> source install/setup.bash
```

3. Launch the entire system using the dedicated launch file:

```bash
>> ros2 launch inspection_system inspection_system.launch.py
```