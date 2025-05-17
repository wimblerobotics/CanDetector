// annotation.cpp
// Implements annotation UI and logic for CanDetector
#include "annotation.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

void runAnnotationLoop(cv::VideoCapture& cap, const std::vector<ObjectDescriptor>& descriptors) {
    cv::Mat frame;
    int frameNumber = 0;
    std::vector<cv::Rect> boundingBoxes;

    while (cap.read(frame)) {
        if (frame.empty()) {
            std::cerr << "Error: Empty frame captured." << std::endl;
            break;
        }

        std::cout << "Processing frame " << frameNumber << "..." << std::endl;

        // Object recognition based on descriptors
        boundingBoxes.clear();
        for (const auto& descriptor : descriptors) {
            // Convert main color to HSV
            cv::Mat mainColor(1, 1, CV_8UC3, cv::Scalar(descriptor.main_color_rgb[2], descriptor.main_color_rgb[1], descriptor.main_color_rgb[0]));
            cv::Mat mainColorHSV;
            cv::cvtColor(mainColor, mainColorHSV, cv::COLOR_BGR2HSV);

            // Define HSV range for main color
            cv::Scalar lowerBound(mainColorHSV.at<cv::Vec3b>(0, 0)[0] - 10, 100, 100);
            cv::Scalar upperBound(mainColorHSV.at<cv::Vec3b>(0, 0)[0] + 10, 255, 255);

            cv::Mat hsvFrame;
            cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);

            // Threshold based on main color
            cv::Mat mask;
            cv::inRange(hsvFrame, lowerBound, upperBound, mask);

            // Find contours in the mask
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            for (const auto& contour : contours) {
                cv::Rect boundingBox = cv::boundingRect(contour);
                boundingBoxes.push_back(boundingBox);
            }
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

        // Mouse callback for interactive annotation
        cv::setMouseCallback("CanDetector Annotation", [](int event, int x, int y, int flags, void* userdata) {
            static cv::Point startPoint;
            static bool drawing = false;
            auto* boundingBoxes = static_cast<std::vector<cv::Rect>*>(userdata);

            switch (event) {
                case cv::EVENT_LBUTTONDOWN:
                    startPoint = cv::Point(x, y);
                    drawing = true;
                    break;
                case cv::EVENT_MOUSEMOVE:
                    if (drawing) {
                        cv::Rect newBox(startPoint, cv::Point(x, y));
                        if (!boundingBoxes->empty()) {
                            boundingBoxes->back() = newBox;
                        } else {
                            boundingBoxes->push_back(newBox);
                        }
                    }
                    break;
                case cv::EVENT_LBUTTONUP:
                    drawing = false;
                    break;
                case cv::EVENT_RBUTTONDOWN:
                    // Delete bounding box if right-clicked inside
                    boundingBoxes->erase(std::remove_if(boundingBoxes->begin(), boundingBoxes->end(), [x, y](const cv::Rect& box) {
                        return box.contains(cv::Point(x, y));
                    }), boundingBoxes->end());
                    break;
            }
        }, &boundingBoxes);

        // Save annotations in YOLO format
        std::ofstream yoloFile("annotations.txt", std::ios::app);
        for (const auto& box : boundingBoxes) {
            float xCenter = (box.x + box.width / 2.0f) / frame.cols;
            float yCenter = (box.y + box.height / 2.0f) / frame.rows;
            float width = static_cast<float>(box.width) / frame.cols;
            float height = static_cast<float>(box.height) / frame.rows;

            yoloFile << "0 " << xCenter << " " << yCenter << " " << width << " " << height << "\n";
        }
        yoloFile.close();

        // Save the annotated frame
        std::string outputFilename = "frame_" + std::to_string(frameNumber) + ".jpg";
        cv::imwrite(outputFilename, frame);

        frameNumber++;
    }
    cv::destroyAllWindows();
}
