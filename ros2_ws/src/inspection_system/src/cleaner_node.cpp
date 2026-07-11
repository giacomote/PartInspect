#include <map>
#include <string>
#include <vector>
#include <chrono>
#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <ros_gz_interfaces/srv/delete_entity.hpp>
#include <ros_gz_interfaces/msg/entity.hpp>

using namespace std::chrono_literals;


class CleanerNode : public rclcpp::Node {
public:
    CleanerNode() : Node("object_cleaner_node") {
        // Using simulation time (to avoid synchronization problems caused by low simulation performances)
        this->set_parameter(rclcpp::Parameter("use_sim_time", true));

        // Config parameters
        world_name_ = "inspection_system"; 
        z_threshold_ = 0.15;                  // Height below which the object is considered on the ground
        delay_duration_ = 15.0;               // Delay before the object deletion

        // Subscribing to the object position (odometry) topic
        odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
            "/gazebo/spawned_objects/odometry", 10, 
            std::bind(&CleanerNode::odom_callback, this, std::placeholders::_1)
        );

        // Creating a client to use the Gazebo object deletion service
        std::string service_name = "/world/" + world_name_ + "/remove";
        delete_client_ = this->create_client<ros_gz_interfaces::srv::DeleteEntity>(service_name);

        // Creating a periodic timer (each 100ms) to check the objects
        timer_ = this->create_wall_timer(100ms, std::bind(&CleanerNode::check_timers, this));

        RCLCPP_INFO(this->get_logger(), "Cleaner Node ready!");
    }

private:
    void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg) {
        if (msg->header.frame_id == "world" || msg->header.frame_id == world_name_) {
            std::string full_child_id = msg->child_frame_id;

            // Cleaning object name (deleting an eventual link name from it)
            size_t slash_pos = full_child_id.find('/');
            std::string model_name = (slash_pos != std::string::npos) ? full_child_id.substr(0, slash_pos) : full_child_id;

            // Ignoring last messages from already deleted objects
            if (ignored_objects_.find(model_name) != ignored_objects_.end()) return;

            // Ignoring static models in the world
            if (model_name == "ground_plane" || model_name == "conveyor_belt" || 
                model_name == "parts_bin" || model_name == "robot" || 
                model_name == "inspection_camera" || model_name == "check_camera" || 
                model_name == "solenoid_pusher") {
                return;
            }

            // Computing the Z coordinate from the position of the object
            double z_pos = msg->pose.pose.position.z;
            rclcpp::Time current_time = this->get_clock()->now();

            if (z_pos < z_threshold_) {
                // Inserting the object into a fallen objects list
                if (fallen_objects_.find(model_name) == fallen_objects_.end()) {
                    fallen_objects_[model_name] = current_time;
                }
            } else {
                // Reset if the object somehow returns up
                if (fallen_objects_.find(model_name) != fallen_objects_.end()) {
                    fallen_objects_.erase(model_name);
                }
            }
        }
    }

    void check_timers() {
        rclcpp::Time current_time = this->get_clock()->now();
        std::vector<std::string> objects_to_delete;

        // Removing objects from the blacklist (names can repeat)
        for (auto it = ignored_objects_.begin(); it != ignored_objects_.end(); ) {
            if ((current_time - it->second).seconds() >= 1.0) {
                it = ignored_objects_.erase(it);
            } else {
                ++it;
            }
        }

        // Computing objects to delete (fallen some time before)
        for (const auto& [name, fall_time] : fallen_objects_) {
            double elapsed_time = (current_time - fall_time).seconds();
            if (elapsed_time >= delay_duration_) {
                objects_to_delete.push_back(name);
            }
        }

        // Deleting objects
        for (const auto& name : objects_to_delete) {
            if (!delete_client_->service_is_ready()) {
                RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 1000,
                    "Gazebo deletion service NOT ready. Object deletion delayed for: %s", name.c_str()
                );
                continue; 
            }

            ignored_objects_[name] = current_time;  // Ignoring future messages from the object (for a period of time)
            fallen_objects_.erase(name);  // Deleting the object from the pending objects to delete

            delete_entity(name);
        }
    }

    void delete_entity(const std::string& name) {
        auto request = std::make_shared<ros_gz_interfaces::srv::DeleteEntity::Request>();
        request->entity.name = name;
        request->entity.type = ros_gz_interfaces::msg::Entity::MODEL;

        // RCLCPP_INFO(this->get_logger(), "Timeout expired. Invoking deletion for: '%s'", name.c_str());

        auto result_future = delete_client_->async_send_request(request,
            [this, name](rclcpp::Client<ros_gz_interfaces::srv::DeleteEntity>::SharedFuture future) {
                try {
                    auto response = future.get();
                    if (!response->success) {
                        RCLCPP_ERROR(this->get_logger(), "Gazebo refused deleting '%s'", name.c_str());
                    }
                } catch (const std::exception& e) {
                    RCLCPP_ERROR(this->get_logger(), "Deletion error (object '%s'): %s", name.c_str(), e.what());
                }
            });
    }

    std::string world_name_;
    double z_threshold_;
    double delay_duration_;

    std::map<std::string, rclcpp::Time> fallen_objects_;
    std::map<std::string, rclcpp::Time> ignored_objects_;

    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
    rclcpp::Client<ros_gz_interfaces::srv::DeleteEntity>::SharedPtr delete_client_;
    rclcpp::TimerBase::SharedPtr timer_;
};


int main(int argc, char ** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<CleanerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}