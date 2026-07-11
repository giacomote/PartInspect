# Inspection System

A simulated ROS2 and Gazebo environment for automated quality inspection in manufacturing.

This project simulates an industrial quality inspection cell composed of:

- A **conveyor belt** transporting parts
- A **vision system** for part identification
- A **solenoid actuator** for rejecting defective items
- **ROS2 nodes** coordinating perception, decision making, and actuation

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

Before building and running this package, make sure your system is using one of the supported ROS 2 distributions:
- **Humble Hawksbill**
- **Jazzy Jalisco**

### ROS2 Package Dependencies
The system relies on the following official ROS2 packages, which must be installed in your environment:
*   `rclcpp` -- ROS2 C++ client library
*   `std_msgs` -- Standard ROS2 message types
*   `sensor_msgs` -- Interface for camera and sensor data
*   `image_transport` -- Optimized image publication and subscription
*   `cv_bridge` -- Bridge between ROS2 image messages and OpenCV

You can automatically resolve and install any missing ROS2 dependencies by running `rosdep` from the root of your
workspace:

```bash
rosdep install --from-paths src --ignore-src -r -y
```

### Third-Party Libraries
- **OpenCV 4.x** -- Core computer vision library

## 🧪 Installing and Testing the Package

To install and test this ROS2 package, you need to:

1. Place the package into your ROS2 workspace

2. Build the package using `colcon`:

```bash
>> cd <ros2_workspace>
>> source /opt/ros/jazzy/setup.bash
>> colcon build --packages-select inspection_system
>> source install/setup.bash
```

3. Configure Gazebo to locate the project models by setting the following environment variable:

```bash
export GZ_SIM_RESOURCE_PATH=<ros2_workspace>/ros2_ws/src/inspection_system/models
```

4. Launch the complete system:

```bash
ros2 launch inspection_system inspection_system.launch.py
```

5. Start the simulation (by pressing the `Play` button on top of the right sidebar menu)

6. Use the `Resource Spawner` to place `Small Box` and `Small Cone` objects onto the conveyor belt. These models can be
   found under the `Local Resources` menu.

## ⚙️ Simulation Overview

The simulation is based on two main principles:

- **Small Boxes** represent valid parts and are allowed to continue along the conveyor belt
- **Small Cones** represent defective parts. They are automatically detected and removed from the conveyor by a
  pneumatic pusher

As a safety mechanism, a second camera is installed further along the conveyor.  
If a defective part (a cone) is not successfully removed and reaches this checkpoint, the system detects the failure
and automatically stops the conveyor belt.

Objects that fall off the conveyor automatically disappear from the simulation after a few seconds.