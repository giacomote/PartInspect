# PartInspect

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

## 📂 Project Structure

    PartInspect/
    ├── ros2_ws/                # Shared ROS2 container workspace
    ├── .gitignore
    ├── docker-compose.yml      # Docker containers settings
    ├── LICENSE
    ├── README.md
    └── start.sh                # Container startup script

## 🧪 Testing the Project

### Important Note

This project is designed to run on a PC that:

- Has a dedicated **NVIDIA GPU**
- Is running **Fedora Linux 44**

If you want to run it on a different system, you will need to modify the `docker-compose.yml` file accordingly.

Alternatively, you can run the project directly on your host operating system, after installing `ROS2 Jazzy Jalisco`
and `Gazebo Harmonic`. In that case, please refer to the [ROS package README file](./ros2_ws/src/inspection_system/README.md).

### Testing Procedure

To install and test this ROS2 package, you need to:

1. Change its permissions and run the `start.sh` script to enter the container:

```bash
>> chmod u+x ./start.sh
>> ./start.sh
```

2. Build the ROS package using `colcon`:

```bash
>> cd /home/ros_user/ros2_ws
>> colcon build --packages-select inspection_system
>> source install/setup.bash
```

3. Launch the complete system:

```bash
ros2 launch inspection_system inspection_system.launch.py
```

4. Start the simulation (by pressing the `Play` button on the bottom-left corner of the Gazebo window)

5. Use the `Resource Spawner` to place `Small Box` and `Small Cone` objects onto the conveyor belt. These models can be
   found under the `Local Resources` menu.

## ⚙️ Simulation Overview

The simulation is based on two main principles:

- **Small Boxes** represent valid parts and are allowed to continue along the conveyor belt
- **Small Cones** represent defective parts. They are automatically detected and removed from the conveyor by a
  pneumatic pusher

As a safety mechanism, a second camera is installed further along the conveyor.  
If a defective part (a cone) is not successfully removed and reaches this checkpoint, the system detects the failure
and automatically stops the conveyor belt.