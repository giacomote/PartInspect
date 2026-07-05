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

Note that this project is specifically made to work in a PC which is:
- Equipped with an **NVIDIA dedicated GPU**
- Running **Fedora Linux 44**

It is up to you to manage the `docker-compose.yml` file to run the container on a different system.

Alternatively, you can always run the project directly on your host operating system, after you installed `ROS2 Jazzy`
and `Gazebo Harmonic`.

---

However, to test the project, you need to execute the following steps:
1. Change the permissions and run the `start.sh` script:

```bash
>> chmod u+x ./start.sh
>> ./start.sh
```

1. Compile and execute ROS2 package ...