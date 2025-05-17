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
        cap.open(0);
    } else {
        cap.open(videoSource);
    }
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open video source: " << videoSource << std::endl;
        return 1;
    }

    // Load object descriptors (to be implemented)
    std::vector<ObjectDescriptor> descriptors = loadDescriptors("descriptors.json");

    // Start annotation loop
    runAnnotationLoop(cap, descriptors);

    return 0;
}
