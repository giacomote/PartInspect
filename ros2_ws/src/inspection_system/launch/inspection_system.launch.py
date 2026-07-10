import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    pkg_share = get_package_share_directory('inspection_system')
    world_file = os.path.join(pkg_share, 'worlds', 'inspection_system_world.sdf')
    bridge_config = os.path.join(pkg_share, 'config', 'bridge_config.yml')

    use_sim_time_arg = DeclareLaunchArgument(
        'use_sim_time',
        default_value='true',
        description='Use simulation (Gazebo) clock if true'
    )

    use_sim_time_config = LaunchConfiguration('use_sim_time')
    
    # Gazebo Launcher
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            os.path.join(get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')
        ]),
        launch_arguments={'gz_args': f'{world_file}'}.items()
    )

    # ROS2-Gazebo Bridge
    gazebo_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        parameters=[
            {'config_file': bridge_config},
            {'use_sim_time': use_sim_time_config}
        ],
        output='screen'
    )

    # ROS2 Nodes
    vision_node = Node(
        package='inspection_system',
        executable='vision_node',
        name='camera_1',
        parameters=[{'use_sim_time': use_sim_time_config}],
        output='screen'
    )

    controller_node = Node(
        package='inspection_system',
        executable='controller_node',
        name='conceyor_belt_controller',
        parameters=[{'use_sim_time': use_sim_time_config}],
        output='screen'
    )

    pusher_node = Node(
        package='inspection_system',
        executable='pusher_node',
        name='pusher_solenoid',
        parameters=[{'use_sim_time': use_sim_time_config}],
        output='screen'
    )

    check_node = Node(
        package='inspection_system',
        executable='check_node',
        name='camera_2',
        parameters=[{'use_sim_time': use_sim_time_config}],
        output='screen'
    )
    
    # Returning All
    return LaunchDescription([
        use_sim_time_arg,
        gazebo,
        gazebo_bridge,
        vision_node,
        controller_node,
        pusher_node,
        check_node
    ])