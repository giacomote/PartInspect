#include <memory>
#include <chrono>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "cv_bridge/cv_bridge.hpp"
#include <opencv2/opencv.hpp>

using namespace std::chrono_literals;


class DummyCameraNode : public rclcpp::Node {
public:
    DummyCameraNode() : Node("dummy_camera_node"), state_(0) {
        auto sensor_qos = rclcpp::QoS(rclcpp::KeepLast(5)).best_effort().durability_volatile();
        
        // Creating a publisher on the topic /camera/image_raw
        camera_pub_ = this->create_publisher<sensor_msgs::msg::Image>("/camera1/image_raw", sensor_qos);
        
        // Publishing a fake image every 2 seconds
        timer_ = this->create_wall_timer(2s, std::bind(&DummyCameraNode::publish_fake_image, this));
        
        RCLCPP_INFO(this->get_logger(), "Dummy camera ready! Starting image generation...");
    }

private:
    void publish_fake_image() {
        cv::Mat frame(400, 400, CV_8UC3, cv::Scalar(200, 200, 200));
        
        switch (state_) {
            case 0:  // SQUARE
                cv::rectangle(frame, cv::Point(100, 100), cv::Point(300, 300), cv::Scalar(0, 0, 0), -1);
                RCLCPP_INFO(this->get_logger(), "[STATE 0] Publishing a SQUARE...");
                state_ = 1;
                break;

            case 1:  // Blank image after the square
                RCLCPP_INFO(this->get_logger(), "[STATE 1] Publishing a BLANK image...");
                state_ = 2;
                break;

            case 2:  // TRIANGLE
                {
                    std::vector<cv::Point> triangle_points = { cv::Point(200, 100), cv::Point(100, 300), cv::Point(300, 300) };
                    std::vector<std::vector<cv::Point>> pts = { triangle_points };
                    cv::fillPoly(frame, pts, cv::Scalar(0, 0, 0));
                }
                RCLCPP_INFO(this->get_logger(), "[STATE 2] Publishing a TRIANGLE...");
                state_ = 3;
                break;

            case 3:  // Blank image after the triangle
                RCLCPP_INFO(this->get_logger(), "[STATE 3] Publishing a BLANK image");
                state_ = 0;
                break;
            
            default:
                state_ = 0;
                break;
        }

        // Converting OpenCV matrix into a ROS2 image
        auto msg = cv_bridge::CvImage(std_msgs::msg::Header(), "bgr8", frame).toImageMsg();
        camera_pub_->publish(*msg);
    }

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr camera_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    int state_;
};


int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<DummyCameraNode>());
    rclcpp::shutdown();
    return 0;
}