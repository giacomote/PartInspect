import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='inspection_system',
            executable='vision_node',
            name='camera_1',
            output='screen'
        ),
        Node(
            package='inspection_system',
            executable='controller_node',
            name='conceyor_belt_controller',
            output='screen'
        ),
        Node(
            package='inspection_system',
            executable='pusher_node',
            name='pusher_solenoid',
            output='screen'
        ),
        Node(
            package='inspection_system',
            executable='check_node',
            name='camera_2',
            output='screen'
        )
    ])