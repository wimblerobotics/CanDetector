// annotation.cpp
// Implements annotation UI and logic for CanDetector
#include "annotation.h"

#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>

void runAnnotationLoop(cv::VideoCapture &cap,
                       const std::vector<ObjectDescriptor> &descriptors) {
    cv::Mat frame;
    int frameNumber = 0;
    std::vector<cv::Rect> boundingBoxes;

    while (cap.read(frame)) {
        if (frame.empty()) {
            std::cerr << "Error: Empty frame captured." << std::endl;
            break;
        }

        std::cout << "Processing frame " << frameNumber << "..." << std::endl;

        // Enhanced object recognition to merge regions based on primary and
        // secondary colors
        boundingBoxes.clear();
        for (const auto &descriptor : descriptors) {
            // Convert primary and secondary colors to HSV
            cv::Mat primaryColor(1, 1, CV_8UC3,
                                 cv::Scalar(descriptor.main_color_rgb[2],
                                            descriptor.main_color_rgb[1],
                                            descriptor.main_color_rgb[0]));
            cv::Mat secondaryColor(
                1, 1, CV_8UC3,
                cv::Scalar(descriptor.secondary_color_rgb[2],
                           descriptor.secondary_color_rgb[1],
                           descriptor.secondary_color_rgb[0]));
            cv::Mat primaryColorHSV, secondaryColorHSV;
            cv::cvtColor(primaryColor, primaryColorHSV, cv::COLOR_BGR2HSV);
            cv::cvtColor(secondaryColor, secondaryColorHSV, cv::COLOR_BGR2HSV);

            // Define HSV ranges for primary and secondary colors
            cv::Scalar primaryLower(primaryColorHSV.at<cv::Vec3b>(0, 0)[0] - 10,
                                    100, 100);
            cv::Scalar primaryUpper(primaryColorHSV.at<cv::Vec3b>(0, 0)[0] + 10,
                                    255, 255);
            cv::Scalar secondaryLower(
                secondaryColorHSV.at<cv::Vec3b>(0, 0)[0] - 10, 50, 50);
            cv::Scalar secondaryUpper(
                secondaryColorHSV.at<cv::Vec3b>(0, 0)[0] + 10, 255, 255);

            cv::Mat hsvFrame;
            cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);

            // Threshold based on primary and secondary colors
            cv::Mat primaryMask, secondaryMask;
            cv::inRange(hsvFrame, primaryLower, primaryUpper, primaryMask);
            cv::inRange(hsvFrame, secondaryLower, secondaryUpper,
                        secondaryMask);

            // Find contours for primary color
            std::vector<std::vector<cv::Point>> primaryContours;
            cv::findContours(primaryMask, primaryContours, cv::RETR_EXTERNAL,
                             cv::CHAIN_APPROX_SIMPLE);

            // Merge neighboring regions of primary color
            std::vector<cv::Rect> mergedRegions;
            for (const auto &contour : primaryContours) {
                cv::Rect boundingBox = cv::boundingRect(contour);
                bool merged = false;
                for (auto &region : mergedRegions) {
                    if ((cv::norm(region.tl() - boundingBox.tl()) < 50) ||
                        (cv::norm(region.br() - boundingBox.br()) < 50)) {
                        region |= boundingBox;  // Merge regions
                        merged = true;
                        break;
                    }
                }
                if (!merged) {
                    mergedRegions.push_back(boundingBox);
                }
            }

            // Debugging: Print the number of merged regions before validation
            std::cout << "Number of merged regions: " << mergedRegions.size()
                      << std::endl;

            // Validate merged regions
            for (const auto &region : mergedRegions) {
                cv::Mat testRegion = hsvFrame(region);
                cv::Mat primaryRegion, secondaryRegion;
                cv::inRange(testRegion, primaryLower, primaryUpper,
                            primaryRegion);
                cv::inRange(testRegion, secondaryLower, secondaryUpper,
                            secondaryRegion);

                double primaryArea = cv::countNonZero(primaryRegion);
                double secondaryArea = cv::countNonZero(secondaryRegion);
                double totalArea = region.area();
                float aspectRatio =
                    static_cast<float>(region.width) / region.height;

                // Debugging: Print validation details including centroid
                cv::Point centroid(region.x + region.width / 2,
                                   region.y + region.height / 2);
                std::cout << "Region: Centroid = (" << centroid.x << ", "
                          << centroid.y << ")"
                          << ", Primary Area = " << primaryArea
                          << ", Secondary Area = " << secondaryArea
                          << ", Total Area = " << totalArea
                          << ", Aspect Ratio = " << aspectRatio << std::endl;

                // Check color ratio and aspect ratio
                if (primaryArea / totalArea >= descriptor.color_ratio - 0.1 &&
                    primaryArea / totalArea <= descriptor.color_ratio + 0.1 &&
                    secondaryArea / totalArea >=
                        (1 - descriptor.color_ratio) - 0.1 &&
                    secondaryArea / totalArea <=
                        (1 - descriptor.color_ratio) + 0.1) {
                    // Calculate expected aspect ratio and apply fuzzy test
                    float expectedAspectRatio =
                        descriptor.typical_width / descriptor.typical_height;
                    float lowerBound =
                        expectedAspectRatio * (1.0f - ASPECT_RATIO_FUZZ);
                    float upperBound =
                        expectedAspectRatio * (1.0f + ASPECT_RATIO_FUZZ);
                    std::cout
                        << "Expected Aspect Ratio: " << expectedAspectRatio
                        << ", Lower Bound: " << lowerBound
                        << ", Upper Bound: " << upperBound
                        << ", Aspect Ratio: " << aspectRatio << std::endl;
                    // Check if the aspect ratio is within the fuzzy bounds
                    if (aspectRatio >= lowerBound &&
                        aspectRatio <=
                            upperBound) {  // Fuzzy aspect ratio check
                        boundingBoxes.push_back(region);
                    }
                }
            }

            // Debugging: Print the number of bounding boxes after validation
            std::cout << "Number of bounding boxes after validation: "
                      << boundingBoxes.size() << std::endl;

            // Debugging: Visualize primary and secondary masks
            cv::imshow("Primary Mask", primaryMask);
            cv::imshow("Secondary Mask", secondaryMask);

            // Debugging: Draw merged regions on the frame
            for (const auto &region : mergedRegions) {
                cv::rectangle(frame, region, cv::Scalar(255, 0, 0),
                              2);  // Blue for merged regions
            }

            // Display the frame with merged regions
            cv::imshow("Merged Regions", frame);
        }

        // Draw bounding boxes
        for (const auto &box : boundingBoxes) {
            cv::rectangle(frame, box, cv::Scalar(0, 255, 0), 2);
        }

        // Display bounding boxes in the console (temporary side panel)
        std::cout << "Bounding Boxes:" << std::endl;
        for (size_t i = 0; i < boundingBoxes.size(); ++i) {
            const auto &box = boundingBoxes[i];
            std::cout << i << ": [" << box.x << ", " << box.y << "] to ["
                      << (box.x + box.width) << ", " << (box.y + box.height)
                      << "]" << std::endl;
        }

        // Highlight selected bounding box
        static int selectedBoxIndex = -1;
        if (selectedBoxIndex >= 0 && selectedBoxIndex < boundingBoxes.size()) {
            cv::rectangle(frame, boundingBoxes[selectedBoxIndex],
                          cv::Scalar(0, 165, 255),
                          2);  // Orange for selected box
        }

        // Display the frame
        cv::imshow("CanDetector Annotation", frame);
        std::cout << "Frame " << frameNumber
                  << " displayed. Waiting for user input..." << std::endl;

        // Wait for user input
        char key = (char)cv::waitKey(0);  // Wait indefinitely for a key press
        if (key == 'q' || key == 27) {    // Exit on 'q' or ESC
            std::cout << "Exiting annotation loop." << std::endl;
            break;
        } else if (key == ' ') {          // Fetch a new frame on space bar
            std::cout << "Fetching next frame..." << std::endl;
            frameNumber++;
            continue;
        }

        // Mouse callback for interactive annotation
        cv::setMouseCallback(
            "CanDetector Annotation",
            [](int event, int x, int y, int flags, void *userdata) {
                auto *data =
                    static_cast<std::pair<std::vector<cv::Rect> *, int *> *>(
                        userdata);
                auto &boxes = *data->first;
                auto &selectedIndex = *data->second;

                if (event == cv::EVENT_LBUTTONDOWN) {
                    for (size_t i = 0; i < boxes.size(); ++i) {
                        if (boxes[i].contains(cv::Point(x, y))) {
                            selectedIndex = static_cast<int>(i);
                            break;
                        }
                    }
                }
            },
            new std::pair<std::vector<cv::Rect> *, int *>(&boundingBoxes,
                                                          &selectedBoxIndex));

        // Save annotations in YOLO format
        std::ofstream yoloFile("annotations.txt", std::ios::app);
        for (const auto &box : boundingBoxes) {
            float xCenter = (box.x + box.width / 2.0f) / frame.cols;
            float yCenter = (box.y + box.height / 2.0f) / frame.rows;
            float width = static_cast<float>(box.width) / frame.cols;
            float height = static_cast<float>(box.height) / frame.rows;

            yoloFile << "0 " << xCenter << " " << yCenter << " " << width << " "
                     << height << "\n";
        }
        yoloFile.close();

        // Save the annotated frame
        std::string outputFilename =
            "frame_" + std::to_string(frameNumber) + ".jpg";
        cv::imwrite(outputFilename, frame);

        frameNumber++;
    }
    cv::destroyAllWindows();
}
