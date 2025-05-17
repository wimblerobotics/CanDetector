# CanDetector

A C++/OpenCV tool for Raspberry Pi (Ubuntu 24.04) to assist in building annotated datasets for YOLO object detection training, focused on cylindrical objects (e.g., cans, bottles) for robotics applications.

## Features
- Capture video from camera or file
- Propose bounding boxes for objects based on color and aspect ratio
- Interactive annotation: accept, reject, adjust, add, or delete bounding boxes per frame
- Save annotations in YOLO format (one .txt file per frame)
- Configurable object descriptors (name, size, main color, etc.)
- Designed for easy review and editing of annotations

## Getting Started

### Prerequisites
- Raspberry Pi 5 (or other Linux system)
- Ubuntu 24.04
- OpenCV (>=4.5 recommended)
- CMake (>=3.10)
- C++17 compiler

### Build Instructions
```sh
sudo apt update
sudo apt install cmake g++ libopencv-dev
cd CanDetector
mkdir build 
cmake -S . -B build
clear;cmake --build build;./build/CanDetector
```

### Usage
- Run the program to capture or process video
- Annotate bounding boxes interactively
- Annotations are saved in YOLO format for training

## License
MIT License. See [LICENSE](LICENSE).

## Contributing
Pull requests welcome! Please open issues for suggestions or bugs.
