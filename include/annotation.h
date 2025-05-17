// annotation.h
// Annotation UI and logic for CanDetector
#pragma once
#include <opencv2/opencv.hpp>
#include "descriptor.h"
#include <vector>

// Runs the main annotation loop: shows frames, draws bounding boxes, handles user input
void runAnnotationLoop(cv::VideoCapture& cap, const std::vector<ObjectDescriptor>& descriptors);
