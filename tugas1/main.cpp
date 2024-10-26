#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    VideoCapture cap(0);  // Open the default camera
    if (!cap.isOpened()) {
        cout << "Error: Could not open the camera." << endl;
        return -1;
    }

    Mat frame;
    while (true) {
        cap >> frame;  // Capture each frame
        if (frame.empty()) {
            cout << "Error: Empty frame." << endl;
            break;
        }

        // Convert to HSV and detect green color
        Mat hsv_frame, mask;
        cvtColor(frame, hsv_frame, COLOR_BGR2HSV);

        // Define HSV range for green color
        Scalar lower_green(40, 40, 40);  // Lower bound for green
        Scalar upper_green(70, 255, 255);  // Upper bound for green
        inRange(hsv_frame, lower_green, upper_green, mask);

        // Apply Gaussian blur to reduce noise
        GaussianBlur(mask, mask, Size(9, 9), 2);

        // Use morphological operations to fill in gaps and reduce noise
        Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
        dilate(mask, mask, kernel);
        erode(mask, mask, kernel);

        // Find contours of the detected green object
        vector<vector<Point>> contours;
        findContours(mask, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

        double max_area = 0;
        int max_contour_idx = -1;
        Rect largest_bounding_rect;

        // Find the largest green object
        for (size_t i = 0; i < contours.size(); i++) {
            double contour_area = contourArea(contours[i]);
            if (contour_area > max_area) {
                max_area = contour_area;
                max_contour_idx = i;
                largest_bounding_rect = boundingRect(contours[i]);
            }
        }

        // If a green object is detected, draw a circle around it and display its width
        if (max_contour_idx != -1) {
            Point2f center;
            float radius;
            minEnclosingCircle(contours[max_contour_idx], center, radius);

            // Draw a circle around the largest green object
            circle(frame, center, (int)radius, Scalar(0, 0, 255), 2);  // Red circle

            // Get the pixel width of the bounding rectangle of the largest object
            float pixel_width = largest_bounding_rect.width;

            // Display the pixel width on the video frame
            stringstream ss;
            ss << "Width: " << pixel_width << " px";
            putText(frame, ss.str(), Point(largest_bounding_rect.x, largest_bounding_rect.y - 10),
                    FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 2);
        }

        // Show the video frame with detected object and pixel width
        imshow("Green Object Detection", frame);

        // Break the loop if the user presses the 'q' key
        if (waitKey(30) == 'q') break;
    }

    // Release the camera
    cap.release();

    // Close all OpenCV windows
    destroyAllWindows();

    return 0;
}
