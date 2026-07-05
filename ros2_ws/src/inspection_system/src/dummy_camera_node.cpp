#include <memory>
#include <chrono>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.hpp"
#include <opencv2/opencv.hpp>

using namespace std::chrono_literals;


class DummyCameraNode : public rclcpp::Node {
public:
    DummyCameraNode() : Node("dummy_camera_node"), alternate_(true) {
        auto sensor_qos = rclcpp::QoS(rclcpp::KeepLast(5)).best_effort().durability_volatile();
        
        // Creating a publisher on the topic /camera/image_raw
        camera_pub_ = this->create_publisher<sensor_msgs::msg::Image>("/camera1/image_raw", sensor_qos);
        
        // Publishing a fake image every 2 seconds
        timer_ = this->create_wall_timer(2s, std::bind(&DummyCameraNode::publish_fake_image, this));
        
        RCLCPP_INFO(this->get_logger(), "Dummy camera ready! Starting image generation...");
    }

private:
    void publish_fake_image() {
        cv::Mat frame = cv::Mat::zeros(400, 400, CV_8UC3);
        
        // Drawing a square and a triangle (alternatively)
        if (alternate_) {
            cv::rectangle(frame, cv::Point(100, 100), cv::Point(300, 300), cv::Scalar(255, 255, 255), -1);
            RCLCPP_INFO(this->get_logger(), "Publishing a SQUARE...");
        } else {
            std::vector<cv::Point> triangle_points = { cv::Point(200, 100), cv::Point(100, 300), cv::Point(300, 300) };
            std::vector<std::vector<cv::Point>> pts = { triangle_points };
            cv::fillPoly(frame, pts, cv::Scalar(255, 255, 255));
            RCLCPP_INFO(this->get_logger(), "Publishing a TRIANGLE...");
        }

        alternate_ = !alternate_;

        // Converting OpenCV matrix into a ROS2 image
        auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toImageMsg();
        camera_pub_->publish(*msg);
    }

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr camera_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    bool alternate_;
};


int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DummyCameraNode>());
    rclcpp::shutdown();
    return 0;
}