// annotation.cpp
// Implements annotation UI and logic for CanDetector
#include "annotation.h"
#include <opencv2/opencv.hpp>
#include <iostream>

void runAnnotationLoop(cv::VideoCapture& cap, const std::vector<ObjectDescriptor>& descriptors) {
    cv::Mat frame;
    int frameNumber = 0;
    while (cap.read(frame)) {
        // TODO: Propose bounding boxes using color/shape heuristics
        // TODO: Draw boxes, handle mouse input for editing
        // TODO: Save annotation in YOLO format
        cv::imshow("CanDetector Annotation", frame);
        char key = (char)cv::waitKey(0); // Wait for user input
        if (key == 'q' || key == 27) break; // Quit on 'q' or ESC
        frameNumber++;
    }
    cv::destroyAllWindows();
}
