#include <memory>
#include <chrono>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/int32.hpp"

using namespace std::chrono_literals;


class PusherNode : public rclcpp::Node {
public:
    PusherNode() : Node("pusher_node"), current_belt_speed_(0.0), distance_to_pusher_(1.5) {
        // Subscription to the belt speed topic
        speed_sub_ = this->create_subscription<std_msgs::msg::Float64>(
            "/config/belt_speed", 10,
            std::bind(&PusherNode::speed_callback, this, std::placeholders::_1)
        );

        // Subscription to the vision_node result
        vision_sub_ = this->create_subscription<std_msgs::msg::Int32>(
            "/vision/detection_result", 10,
            std::bind(&PusherNode::vision_callback, this, std::placeholders::_1)
        );

        // Creating a publisher to command the Gazebo simulation solenoid
        gazebo_pusher_pub_ = this->create_publisher<std_msgs::msg::Float64>("/gazebo/pusher/cmd_pos", 10);

        // Tracking timer (20Hz / 50ms), to track bad part position
        tracking_timer_ = this->create_wall_timer(
            50ms,
            std::bind(&PusherNode::track_defective_parts_callback, this)
        );

        last_update_time_ = this->now();

        RCLCPP_INFO(this->get_logger(), "Pusher Node ready!");
    }

private:
    void speed_callback(const std_msgs::msg::Float64::SharedPtr msg) {
        current_belt_speed_ = msg->data;
    }

    void vision_callback(const std_msgs::msg::Int32::SharedPtr msg) {
        if (msg->data == 1) {
            RCLCPP_INFO(this->get_logger(), "Vision alert! Defective part detected at the start of the line.");
            
            // Adding a new bad part to be tracked (which starts from 0.0 meters and gradually reaches the solenoid)
            tracked_parts_.push_back(0.0);
        }
    }

    void track_defective_parts_callback() {
        rclcpp::Time now = this->now();
        double dt = (now - last_update_time_).seconds();
        last_update_time_ = now;

        if (tracked_parts_.empty()) {
            return;
        }

        double delta_space = current_belt_speed_ * dt;  // Distance covered by the bad part on the conveyor belt

        // Tracking and pushing bad parts
        auto it = tracked_parts_.begin();
        while (it != tracked_parts_.end()) {
            *it += delta_space;

            if (*it >= distance_to_pusher_) {
                RCLCPP_WARN(this->get_logger(), "Part reached pusher target (%.2fm). Actuating piston!", *it);
                actuate_pusher();
                
                it = tracked_parts_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void actuate_pusher() {
        auto push_msg = std_msgs::msg::Float64();
        
        push_msg.data = 1.0; 
        gazebo_pusher_pub_->publish(push_msg);
        
        RCLCPP_INFO(this->get_logger(), "Piston command sent!");

        // Creating a single-shot timer to reset the solenoid piston
        rclcpp::SensorDataQoS qos;
        rclcpp::create_timer(
            this,
            this->get_clock(),
            std::chrono::milliseconds(500),
            [this]() {
                auto retract_msg = std_msgs::msg::Float64();
                retract_msg.data = 0.0;
                gazebo_pusher_pub_->publish(retract_msg);
                RCLCPP_INFO(this->get_logger(), "Piston retracted.");
            }
        );
    }

    rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr speed_sub_;
    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr vision_sub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr gazebo_pusher_pub_;
    rclcpp::TimerBase::SharedPtr tracking_timer_;

    double current_belt_speed_;
    const double distance_to_pusher_;
    std::vector<double> tracked_parts_;
    rclcpp::Time last_update_time_;
};


int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PusherNode>());
    rclcpp::shutdown();
    return 0;
}