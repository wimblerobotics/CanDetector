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

        // Enhanced object recognition to merge regions based on primary and secondary colors
        boundingBoxes.clear();
        for (const auto& descriptor : descriptors) {
            // Convert primary and secondary colors to HSV
            cv::Mat primaryColor(1, 1, CV_8UC3, cv::Scalar(descriptor.main_color_rgb[2], descriptor.main_color_rgb[1], descriptor.main_color_rgb[0]));
            cv::Mat secondaryColor(1, 1, CV_8UC3, cv::Scalar(descriptor.secondary_color_rgb[2], descriptor.secondary_color_rgb[1], descriptor.secondary_color_rgb[0]));
            cv::Mat primaryColorHSV, secondaryColorHSV;
            cv::cvtColor(primaryColor, primaryColorHSV, cv::COLOR_BGR2HSV);
            cv::cvtColor(secondaryColor, secondaryColorHSV, cv::COLOR_BGR2HSV);

            // Define HSV ranges for primary and secondary colors
            cv::Scalar primaryLower(primaryColorHSV.at<cv::Vec3b>(0, 0)[0] - 10, 100, 100);
            cv::Scalar primaryUpper(primaryColorHSV.at<cv::Vec3b>(0, 0)[0] + 10, 255, 255);
            cv::Scalar secondaryLower(secondaryColorHSV.at<cv::Vec3b>(0, 0)[0] - 10, 50, 50);
            cv::Scalar secondaryUpper(secondaryColorHSV.at<cv::Vec3b>(0, 0)[0] + 10, 255, 255);

            cv::Mat hsvFrame;
            cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);

            // Threshold based on primary and secondary colors
            cv::Mat primaryMask, secondaryMask;
            cv::inRange(hsvFrame, primaryLower, primaryUpper, primaryMask);
            cv::inRange(hsvFrame, secondaryLower, secondaryUpper, secondaryMask);

            // Find contours for primary color
            std::vector<std::vector<cv::Point>> primaryContours;
            cv::findContours(primaryMask, primaryContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            // Merge neighboring regions of primary color
            std::vector<cv::Rect> mergedRegions;
            for (const auto& contour : primaryContours) {
                cv::Rect boundingBox = cv::boundingRect(contour);
                bool merged = false;
                for (auto& region : mergedRegions) {
                    if ((cv::norm(region.tl() - boundingBox.tl()) < 50) || (cv::norm(region.br() - boundingBox.br()) < 50)) {
                        region |= boundingBox; // Merge regions
                        merged = true;
                        break;
                    }
                }
                if (!merged) {
                    mergedRegions.push_back(boundingBox);
                }
            }

            // Validate merged regions
            for (const auto& region : mergedRegions) {
                cv::Mat testRegion = hsvFrame(region);
                cv::Mat primaryRegion, secondaryRegion;
                cv::inRange(testRegion, primaryLower, primaryUpper, primaryRegion);
                cv::inRange(testRegion, secondaryLower, secondaryUpper, secondaryRegion);

                double primaryArea = cv::countNonZero(primaryRegion);
                double secondaryArea = cv::countNonZero(secondaryRegion);
                double totalArea = region.area();

                // Check color ratio and aspect ratio
                if (primaryArea / totalArea >= descriptor.color_ratio - 0.1 &&
                    primaryArea / totalArea <= descriptor.color_ratio + 0.1 &&
                    secondaryArea / totalArea >= (1 - descriptor.color_ratio) - 0.1 &&
                    secondaryArea / totalArea <= (1 - descriptor.color_ratio) + 0.1) {

                    float aspectRatio = static_cast<float>(region.width) / region.height;
                    if (aspectRatio >= 0.5 && aspectRatio <= 2.0) { // Example aspect ratio range
                        boundingBoxes.push_back(region);
                    }
                }
            }

            // Debugging: Visualize primary and secondary masks
            cv::imshow("Primary Mask", primaryMask);
            cv::imshow("Secondary Mask", secondaryMask);

            // Debugging: Draw merged regions on the frame
            for (const auto& region : mergedRegions) {
                cv::rectangle(frame, region, cv::Scalar(255, 0, 0), 2); // Blue for merged regions
            }

            // Display the frame with merged regions
            cv::imshow("Merged Regions", frame);
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
