// descriptor.cpp
// Loads object descriptors from a JSON file (stub)
#include "descriptor.h"
#include <vector>

std::vector<ObjectDescriptor> loadDescriptors(const std::string& filename) {
    // TODO: Implement JSON loading
    // For now, return a default descriptor for a coke can
    return { {"coke_can", 12.0f, 6.6f, {220, 30, 30}, {0, 0, 0}, 0.484f} };
}
