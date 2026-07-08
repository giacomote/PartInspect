#include <memory>
#include <chrono>
#include <cmath>
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/int32.hpp"

using namespace std::chrono_literals;


class ControllerNode : public rclcpp::Node {
public:
    ControllerNode() : Node("controller_node"), belt_speed_(-0.5) {
        // Subscription to the control sensor topic
        check_sub_ = this->create_subscription<std_msgs::msg::Int32>(
            "/check/detection_result", 10,
            std::bind(&ControllerNode::check_callback, this, std::placeholders::_1)
        );

        // Initializing a timer (20Hz / 50ms) and a publisher to communicate the conveyor belt speed
        control_timer_ = this->create_wall_timer(
            50ms, 
            std::bind(&ControllerNode::speed_update_callback, this)
        );

        speed_pub_ = this->create_publisher<std_msgs::msg::Float64>("/config/belt_speed", 10);
        gazebo_belt_pub_ = this->create_publisher<std_msgs::msg::Float64>("/gazebo/conveyor_belt/cmd_vel", 10);

        RCLCPP_INFO(this->get_logger(), "Controller Node started!");
        RCLCPP_INFO(this->get_logger(), "Initial belt speed: %.2f m/s", belt_speed_);
    }

private:
    void check_callback(const std_msgs::msg::Int32::SharedPtr msg) {
        RCLCPP_INFO(this->get_logger(), "Received data from check_node: %d", msg->data);
        
        if (msg->data == 1) {
            RCLCPP_WARN(this->get_logger(), "Bad part detected at check point! Stopping...");
            
            if (std::abs(belt_speed_) > 0.0) {
                belt_speed_ = 0.0;
                RCLCPP_ERROR(this->get_logger(), "EMERGENCY STOP ENGAGED: Conveyor stopped via software!");
            }
        }
    }

    void speed_update_callback() {
        auto speed_msg = std_msgs::msg::Float64();
        speed_msg.data = belt_speed_;

        speed_pub_->publish(speed_msg);
        gazebo_belt_pub_->publish(speed_msg);
    }

    rclcpp::Subscription<std_msgs::msg::Int32>::SharedPtr check_sub_;
    rclcpp::TimerBase::SharedPtr control_timer_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr speed_pub_;
    rclcpp::Publisher<std_msgs::msg::Float64>::SharedPtr gazebo_belt_pub_;

    double belt_speed_;
};


int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ControllerNode>());
    rclcpp::shutdown();
    return 0;
}