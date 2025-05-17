// annotation.cpp
// Implements annotation UI and logic for CanDetector
#include "annotation.h"
#include <opencv2/opencv.hpp>
#include <iostream>

void runAnnotationLoop(cv::VideoCapture& cap, const std::vector<ObjectDescriptor>& descriptors) {
    cv::Mat frame;
    int frameNumber = 0;
    std::vector<cv::Rect> boundingBoxes;

    // Mouse callback for interactive annotation
    cv::setMouseCallback("CanDetector Annotation", [](int event, int x, int y, int flags, void* userdata) {
        // TODO: Implement mouse interaction for moving, resizing, deleting, or adding bounding boxes
    });

    while (cap.read(frame)) {
        if (frame.empty()) {
            std::cerr << "Error: Empty frame captured." << std::endl;
            break;
        }

        std::cout << "Processing frame " << frameNumber << "..." << std::endl;

        // Object recognition based on descriptors
        boundingBoxes.clear();
        for (const auto& descriptor : descriptors) {
            // TODO: Detect objects based on aspect ratio and color properties
            // Add detected bounding boxes to boundingBoxes vector
        }

        // Draw bounding boxes
        for (const auto& box : boundingBoxes) {
            cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2);
        }

        // Display the frame
        cv::imshow("CanDetector Annotation", frame);
        std::cout << "Frame " << frameNumber << " displayed. Waiting for user input..." << std::endl;

        // Wait for user input
        char key = (char)cv::waitKey(0); // Wait indefinitely for a key press
        if (key == 'q' || key == 27) { // Exit on 'q' or ESC
            std::cout << "Exiting annotation loop." << std::endl;
            break;
        }

        // TODO: Save annotations in YOLO format

        frameNumber++;
    }
    cv::destroyAllWindows();
}
