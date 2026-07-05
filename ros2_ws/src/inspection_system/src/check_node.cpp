#include <memory>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/int32.hpp"
#include "cv_bridge/cv_bridge.hpp"
#include <opencv2/opencv.hpp>


class CheckNode : public rclcpp::Node {
public:
    CheckNode() : Node("check_node") {
        auto sensor_qos = rclcpp::QoS(rclcpp::KeepLast(5)).best_effort().durability_volatile();

        // Subscription to the video topic (of a real camera or in Gazebo)
        camera_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera2/image_raw",
            sensor_qos,
            std::bind(&CheckNode::image_callback, this, std::placeholders::_1)
        );

        // Generating a publisher which notifies the detected manufacturing part type
        // 0 = Square / Cube (OK), 1 = Triangle / Cone (NOT OK)
        result_pub_ = this->create_publisher<std_msgs::msg::Int32>("/check_node/detection_result", 10);

        RCLCPP_INFO(this->get_logger(), "Check Node ready!");
    }

private:
    void image_callback(const sensor_msgs::msg::Image::SharedPtr msg) {
        cv_bridge::CvImagePtr cv_ptr;

        // Trying to convert ROS2 image type (sensor_msgs::Image) into an OpenCV matrix (cv::Mat)
        try { 
            cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        }
        catch (cv_bridge::Exception& e) {
            RCLCPP_ERROR(this->get_logger(), "cv_bridge error: %s", e.what());
            return;
        }

        // Gray-scale conversion, binarization and thresholding
        cv::Mat gray, thresh;
        cv::cvtColor(cv_ptr->image, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, thresh, 120, 255, cv::THRESH_BINARY);

        // Finding contours and computing shape
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(thresh, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        for (const auto& contour : contours) {
            if (cv::contourArea(contour) < 500) continue;

            // Approximating to a mathematical polygon
            std::vector<cv::Point> approx;
            double epsilon = 0.04 * cv::arcLength(contour, true);
            cv::approxPolyDP(contour, approx, epsilon, true);

            RCLCPP_INFO(
                this->get_logger(),
                "Detected object having AREA %f and %zu VERTICES",
                cv::contourArea(contour), approx.size()
            );
            
            auto result_msg = std_msgs::msg::Int32();

            // Classifying object, using number of vertices
            if (approx.size() == 3) {
                RCLCPP_WARN(this->get_logger(), "Found TRIANGLE / CONE (bad part!)");

                result_msg.data = 1;
                result_pub_->publish(result_msg);
            } 
            else if (approx.size() == 4) {
                RCLCPP_INFO(this->get_logger(), "Found SQUARE / CUBE");

                result_msg.data = 0;
                result_pub_->publish(result_msg);
            }
        }
    }

    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr camera_sub_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr result_pub_;
};


int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CheckNode>());
    rclcpp::shutdown();
    return 0;
}