#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    // Open the video file
    VideoCapture cap("/home/rezaafzaal/Downloads/Video asli.avi");

    if (!cap.isOpened()) {
        return -1;
    }

    Mat frame, hsvFrame, mask;

    // Robot's initial position
    double x_robot = 0.0, y_robot = 0.0;
    double last_object_x_cm = 0.0, last_object_y_cm = 0.0;  // Last ball position in centimeters
    const float pixelsToCm = 1.0;  // Conversion factor: 1 pixel = 10 cm

    bool isFirstFrame = true;  // Flag for the first frame

    // Parameters for ball detection
    double minBallArea = 200.0;  // Minimum area of the ball (to filter out small objects)
    double maxBallArea = 5000.0;  // Maximum area of the ball (to filter out large objects)

    // Define the region to exclude (center of the field)
    Point2f centerField(cap.get(CAP_PROP_FRAME_WIDTH) / 2, cap.get(CAP_PROP_FRAME_HEIGHT) / 2);
    int exclusionRadius = 100;  // Exclude objects within this radius

    while (true) {
        cap >> frame;  // Read frame from the video
        if (frame.empty()) {
            break;  // End of video
        }

        // Convert the frame to HSV color space
        cvtColor(frame, hsvFrame, COLOR_BGR2HSV);

        // Define the range of the orange color in HSV (adjust if needed)
        Scalar lower_orange(10, 80, 80); 
        Scalar upper_orange(25, 255, 255);

        // Create a mask for the orange color
        inRange(hsvFrame, lower_orange, upper_orange, mask);

        // Exclude center area (avoid detecting objects like in the image)
        circle(mask, centerField, exclusionRadius, Scalar(0), -1);

        // Find contours of the orange object (the ball)
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // If contours are found, calculate the center of the ball using minAreaRect
        if (!contours.empty()) {
            // Assume the largest contour that fits within the ball size limits is the ball
            double maxArea = 0;
            int largestContourIndex = -1;

            for (int i = 0; i < contours.size(); i++) {
                double area = contourArea(contours[i]);
                if (area > maxArea && area >= minBallArea && area <= maxBallArea) {
                    maxArea = area;
                    largestContourIndex = i;
                }
            }

            if (largestContourIndex != -1) {
                // Fit a rotated rectangle around the ball
                RotatedRect rect = minAreaRect(contours[largestContourIndex]);

                //Get the center of the rectangle
                Point2f rect_points[4];
                rect.points(rect_points);
                Point2f center = rect.center;

                // Convert ball position to centimeters
                double object_x_cm = center.x * pixelsToCm;
                double object_y_cm = center.y * pixelsToCm;

                // If it's not the first frame, update the robot's position
                if (!isFirstFrame) {
                    x_robot -= object_x_cm - last_object_x_cm;
                    y_robot += object_y_cm - last_object_y_cm;
                }

                // Update last known ball position
                last_object_x_cm = object_x_cm;
                last_object_y_cm = object_y_cm;

                // Set the first frame flag to false after processing the first frame
                isFirstFrame = false;

                // Draw the detected ball's rectangle on the frame
                for (int j = 0; j < 4; j++) {
                    line(frame, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 0), 2);
                }

                // Draw the 4 blue dots to lock on the ball (as seen in the video)
                circle(frame, Point(center.x - 10, center.y - 10), 5, Scalar(255, 0, 0), -1);
                circle(frame, Point(center.x + 10, center.y - 10), 5, Scalar(255, 0, 0), -1);
                circle(frame, Point(center.x - 10, center.y + 10), 5, Scalar(255, 0, 0), -1);
                circle(frame, Point(center.x + 10, center.y + 10), 5, Scalar(255, 0, 0), -1);

                // Display the robot's position on the frame
                std::string positionText = "Posisi robot (x,y): (" + std::to_string(x_robot) + ", " + std::to_string(y_robot) + ")";
                putText(frame, positionText, Point(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);
            }
        }

        // Show the frame with the robot's position and ball lock
        imshow("Video Robot", frame);

        // Break the loop if 'q' is pressed
        if (waitKey(30) == 'q') {
            break;
        }
    }

    return 0;
}