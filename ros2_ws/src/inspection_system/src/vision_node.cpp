#include <memory>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "std_msgs/msg/int32.hpp"
#include "cv_bridge/cv_bridge.hpp"
#include <opencv2/opencv.hpp>


class VisionNode : public rclcpp::Node {
public:
    VisionNode() : Node("vision_node"), has_classified_current_object_(false) {
        auto sensor_qos = rclcpp::QoS(rclcpp::KeepLast(5)).best_effort().durability_volatile();

        // Subscription to the video topic (of a real camera or in Gazebo)
        camera_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
            "/camera1/image_raw",
            sensor_qos,
            std::bind(&VisionNode::image_callback, this, std::placeholders::_1)
        );

        // Generating a publisher which notifies the detected manufacturing part type
        // 0 = Square / Cube (OK), 1 = Triangle / Cone (NOT OK)
        result_pub_ = this->create_publisher<std_msgs::msg::Int32>("/vision/detection_result", 10);

        RCLCPP_INFO(this->get_logger(), "Vision Node ready!");
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

        cv::Mat original_frame = cv_ptr->image;

        // Computing a Region Of Interest (ROI)
        int crop_x = original_frame.cols * 0.10; 
        int crop_y = original_frame.rows * 0.10; 
        int crop_w = original_frame.cols * 0.80;
        int crop_h = original_frame.rows * 0.50;
        
        cv::Rect roi(crop_x, crop_y, crop_w, crop_h);
        cv::Mat cropped_frame = original_frame(roi);

        // Gray-scale conversion and a little blur (to de-noise the image)
        cv::Mat gray, blurred, binary;
        cv::cvtColor(cropped_frame, gray, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 0);

        // Inverted thresholding
        // All dark pixels (< 85) are set to white (255)
        // All the other pixels (> 85) are set to black (0)
        int soglia_valore = 85;
        cv::threshold(blurred, binary, soglia_valore, 255, cv::THRESH_BINARY_INV);

        // Image Cleaning (Opening)
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
        cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

        // --- VISUAL DEBUG ---
        // To visualize the result of all the previous image processing operations, uncomment the following 2 lines
        cv::imshow("Thresholding Debug", binary);
        cv::waitKey(1);

        // Finding contours
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        bool found_any_valid_object = false;

        for (const auto& contour : contours) {
            if (cv::contourArea(contour) < 800) continue;

            found_any_valid_object = true;

            // Computing the rectangle which contains the part and checking if it touches the border of the ROI
            cv::Rect br = cv::boundingRect(contour);

            bool is_fully_inside = (br.x > 3 && br.y > 3 && 
                                    (br.x + br.width) < cropped_frame.cols - 3 && 
                                    (br.y + br.height) < cropped_frame.rows - 3);

            // Classifying the object (but only if it is completely inside the ROI)
            if (is_fully_inside && !has_classified_current_object_) {
                cv::imshow("Classification Frame", binary);  // Showing the classification image

                std::vector<cv::Point> approx;
                double epsilon = 0.02 * cv::arcLength(contour, true);
                cv::approxPolyDP(contour, approx, epsilon, true);

                RCLCPP_INFO(
                    this->get_logger(),
                    "Detected object having AREA %f and %zu VERTICES",
                    cv::contourArea(contour), approx.size()
                );
                
                auto result_msg = std_msgs::msg::Int32();

                // Classifying the shape
                if (approx.size() == 3) {
                    RCLCPP_WARN(this->get_logger(), "Found TRIANGLE / CONE (bad part!)");
                    result_msg.data = 1;
                    result_pub_->publish(result_msg);

                    has_classified_current_object_ = true;
                    break; 
                } 
                else if (approx.size() >= 4 && approx.size() <= 6) {
                    RCLCPP_INFO(this->get_logger(), "Found SQUARE / CUBE");
                    result_msg.data = 0;
                    result_pub_->publish(result_msg);

                    has_classified_current_object_ = true;
                    break; 
                }
            }
        }

        if (!found_any_valid_object) {
            has_classified_current_object_ = false;
        }
        
        cv::waitKey(1);  // Keeping alive the debug window
    }

    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr camera_sub_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr result_pub_;
    bool has_classified_current_object_;
};


int main(int argc, char *argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<VisionNode>());
    rclcpp::shutdown();
    return 0;
}