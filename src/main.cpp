// Main entry point for CanDetector
// - Handles argument parsing (video file or camera)
// - Loads object descriptors
// - Runs the annotation loop
//
// See README.md for usage instructions.

#include <opencv2/opencv.hpp>
#include "annotation.h"
#include "descriptor.h"
#include <iostream>

int main(int argc, char** argv) {
    std::string videoSource;
    if (argc > 1) {
        videoSource = argv[1];
    } else {
        videoSource = "0"; // Default to camera
    }

    cv::VideoCapture cap;
    if (videoSource == "0") {
        cap.open("libcamerasrc ! video/x-raw,format=RGB,width=640,height=480 ! videoconvert ! appsink", cv::CAP_GSTREAMER);
        if (!cap.isOpened()) {
            std::cerr << "Error: Could not open camera using GStreamer pipeline with RGB format." << std::endl;
            return 1;
        }
    } else {
        cap.open(videoSource);
    }
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video source: " << videoSource << std::endl;
        return 1;
    }

    std::cout << "Starting CanDetector with video source: " << videoSource << std::endl;

    // Load object descriptors (to be implemented)
    std::cout << "Loading object descriptors..." << std::endl;
    std::vector<ObjectDescriptor> descriptors = loadDescriptors("descriptors.json");

    std::cout << "Starting annotation loop..." << std::endl;
    // Start annotation loop
    runAnnotationLoop(cap, descriptors);

    std::cout << "Annotation loop finished." << std::endl;

    return 0;
}
