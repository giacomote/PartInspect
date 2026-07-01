#!/bin/sh

xhost +local:docker 1>/dev/null 2>&1  # Enabling graphical permissions for Wayland (on Fedora Linux)

docker compose up -d

echo -e "\nEntering container..."
docker compose exec -it ros2_gazebo bash -c "source /opt/ros/jazzy/setup.bash && exec bash"