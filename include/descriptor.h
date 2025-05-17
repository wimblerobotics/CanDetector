// descriptor.h
// Object descriptor for guiding detection and annotation
#pragma once
#include <string>
#include <vector>

struct ObjectDescriptor {
    std::string name; // e.g. "coke_can"
    float typical_height; // in cm
    float typical_width;  // in cm
    std::vector<int> main_color_rgb; // e.g. {220, 30, 30}
    std::vector<int> secondary_color_rgb; // optional
    float color_ratio; // ratio of main to secondary color (0.0-1.0)
};

// Loads descriptors from a JSON file (to be implemented)
std::vector<ObjectDescriptor> loadDescriptors(const std::string& filename);
