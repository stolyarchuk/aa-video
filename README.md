# AA Video Processing System

A modern C++ video processing system with gRPC-based client-server architecture for real-time object detection using YOLO neural networks with polygon-based detection zones.

## Project Structure

```text
aa_video_processing/
├── CMakeLists.txt          # Main CMake configuration
├── client/                 # Client application
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── detector_client.h
│   │   └── rpc_client.h
│   └── src/
│       └── main.cpp
├── server/                 # Server application
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── detector_server.h
│   │   ├── detector_service.h
│   │   └── rpc_server.h
│   └── src/
│       ├── detector_server.cpp
│       └── main.cpp
├── shared/                 # Shared components
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── frame.h
│   │   ├── logging.h
│   │   ├── options.h
│   │   ├── point.h
│   │   ├── polygon.h
│   │   └── signal_set.h
│   ├── proto/
│   │   ├── detector_service.proto
│   │   ├── frame.proto
│   │   ├── point.proto
│   │   └── polygon.proto
│   └── src/
├── tests/                  # Unit tests
│   ├── test_detector_server.cpp
│   ├── test_options.cpp
│   └── test_signal_set.cpp
├── models/                 # Neural network models
│   ├── yolov7x.weights
│   ├── yolov7.cfg
│   ├── yolov3.weights
│   └── yolov3.cfg
├── build/                  # Build directory (generated)
├── Dockerfile              # Container configuration
├── Doxyfile               # Documentation configuration
└── README.md              # This file
```

## Features

- **Modern C++23** standard with Google C++ Style Guide
- **gRPC** for high-performance client-server communication
- **Protocol Buffers** for efficient data serialization
- **OpenCV 4.8.1** with DNN module for computer vision
- **YOLO object detection** with COCO dataset support (80 classes)
- **Polygon-based detection zones** with inclusion/exclusion areas and priority-based adjudication
- **Non-Maximum Suppression** for duplicate detection filtering
- **Letterboxing preprocessing** for maintaining aspect ratios
- **Coordinate scaling** with letterbox-aware transformations
- **CMake 3.20+** modular build system
- **Google Test** comprehensive unit testing
- **Doxygen** API documentation generation
- **Docker containerization** with development environment
- **Signal handling** for graceful shutdown
- **Structured logging** with configurable levels
- **Cross-platform** support (Linux, planned ARM64)
- **Cross-compilation**: Planned for ARM64 Rockchip RK3588 SoC

## Prerequisites

- **CMake 3.20** or higher
- **C++23 compatible compiler** (GCC 11+, Clang 12+, MSVC 2022+)
- **Protocol Buffers** compiler and libraries
- **gRPC C++** libraries and plugins
- **OpenCV 4.8.1+** with DNN support
- **Google Test** (for running tests)
- **Doxygen** (optional, for documentation)

## Quick Start

### Using Docker (Recommended)

The project includes a complete development environment:

```bash
# Build the container
docker build -t aa-video-processing .

# Run with source mounted
docker run -it --rm -v $(pwd):/workspace aa-video-processing
```

### Manual Installation

For Ubuntu/Debian systems:

```bash
# Install dependencies
sudo apt update && sudo apt install -y \
    build-essential cmake clang clang-format \
    libprotobuf-dev protobuf-compiler \
    libgrpc++-dev libgrpc-dev protobuf-compiler-grpc \
    libopencv-dev libopencv-contrib-dev \
    libgtest-dev doxygen
```

## Building the Project

### Configure and Build

```bash
# Configure Debug build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug

# Build all components
cmake --build build

# Or build specific targets
cmake --build build --target detector_server
cmake --build build --target detector_client
```

### Build Options

Available CMake options:

- `BUILD_CLIENT=ON/OFF` - Build client application (default: ON)
- `BUILD_SERVER=ON/OFF` - Build server application (default: ON)
- `BUILD_TESTS=ON/OFF` - Build unit tests (default: ON)
- `BUILD_DOCUMENTATION=ON/OFF` - Generate API docs (default: OFF)

Example with custom options:

```bash
cmake -B build -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_DOCUMENTATION=ON \
    -DBUILD_TESTS=OFF
```

### VS Code Tasks

Use the predefined tasks for development:

- `Configure CMake` - Configure the build system
- `Build Project` - Build all targets (Debug)
- `Build Release` - Configure and build Release
- `Run Tests` - Execute unit tests
- `Run Application` - Run the main executable
- `Generate Documentation` - Build API documentation

## Usage

### Starting the Server

Run the detector server with a neural network model:

```bash
# Using YOLO for object detection (supports both .weights+.cfg and .onnx formats)
./build/server/detector_server \
    --model=./models/yolov7x.weights \
    --cfg=./models/yolov7.cfg \
    --width=640 \
    --height=640 \
    --address=localhost:50051 \
    --verbose=true

# Using ONNX format
./build/server/detector_server \
    --model=./models/yolov7.onnx \
    --width=640 \
    --height=640 \
    --address=localhost:50051 \
    --verbose=true
```

### Running the Client

Connect to the server and process frames:

```bash
# Connect to server and process input image
./build/client/detector_client \
    --address=localhost:50051 \
    --input=input/000000039769.jpg \
    --width=640 \
    --height=640 \
    --verbose=true

# Process with custom confidence threshold
./build/client/detector_client \
    --address=localhost:50051 \
    --input=/path/to/image.jpg \
    --width=640 \
    --height=640 \
    --verbose=true
```

### Command Line Options

#### Server Options

- `--model` - Path to YOLO model file (.weights, .onnx) (REQUIRED)
- `--cfg` - Path to YOLO configuration file (.cfg) (Required for .weights format)
- `--width` - Network input width (default: 640)
- `--height` - Network input height (default: 640)
- `--address` - Server bind address (default: localhost:50051)
- `--verbose` - Enable verbose logging (default: false)

#### Client Options

- `--address` - Server address to connect (default: localhost:50051)
- `--input` - Input image file path (REQUIRED for client)
- `--width` - Frame width for processing (default: 640)
- `--height` - Frame height for processing (default: 640)
- `--verbose` - Enable verbose logging (default: false)

## Running Tests

The project includes comprehensive unit tests:

```bash
# Run all tests
cmake --build build --target test

# Run tests with detailed output
ctest --test-dir build --output-on-failure

# Run specific test suite
./build/tests/test_detector_server
./build/tests/test_options
./build/tests/test_signal_set
./build/tests/test_polygon
./build/tests/test_frame
```

## Polygon-Based Detection System

The server supports sophisticated polygon-based detection zones with the following features:

### Polygon Types

- **Inclusion Zones**: Only detect objects within these polygonal areas
- **Exclusion Zones**: Block all detections within these polygonal areas
- **Priority System**: Higher priority polygons override lower priority ones in overlapping regions

### Polygon Configuration

Each polygon can specify:

- **Type**: `INCLUSION` or `EXCLUSION`
- **Priority**: Integer value (higher = more important)
- **Target Classes**: List of COCO class IDs to detect (empty = all classes)
- **Vertices**: Array of 2D points defining the polygon shape

### Detection Logic

1. **Object Detection**: YOLO processes the entire frame
2. **Center Point Testing**: Each detection's center point is tested against all polygons
3. **Priority Resolution**: If multiple polygons contain a point, highest priority wins
4. **Class Filtering**: Detections are filtered based on polygon's target class list
5. **Final Results**: Only approved detections are included in the response

### Example Workflow

```text
Input Frame → YOLO Detection → Polygon Filtering → Final Results
     │              │                  │               │
     │         Bounding Boxes     Priority-based    Filtered
     │         + Confidence       Class Filtering   Detections
     │         + Class IDs                              │
     └─────────────────────────────────────────────────┘
                   Returned to Client
```

## Documentation

### API Documentation

Generate comprehensive API documentation with Doxygen:

```bash
# Configure with documentation enabled
cmake -B build -S . -DBUILD_DOCUMENTATION=ON

# Build documentation
cmake --build build --target docs
cmake --build build --target docs

# Open documentation in browser
xdg-open build/docs/html/index.html
```

### Protocol Buffer Schema

The system uses the following gRPC services:

- **DetectorService** - Main object detection service
  - `ProcessFrame(ProcessFrameRequest) → ProcessFrameResponse`
  - `CheckHealth(CheckHealthRequest) → CheckHealthResponse`

Key message types:

- `Frame` - Image/video frame data (compatible with OpenCV Mat)
- `Point` - 2D coordinates for polygon vertices
- `Polygon` - Detection zones with inclusion/exclusion rules, priorities, and target class filters
- `ProcessFrameRequest` - Contains frame and polygon definitions
- `ProcessFrameResponse` - Returns processed frame and detection results

### Architecture Overview

```text
┌─────────────────┐    gRPC     ┌─────────────────┐
│  Client App     │◄────────────┤  Server App     │
│                 │             │                 │
│ - Frame capture │             │ - YOLO loading  │
│ - Polygon setup │             │ - Inference     │
│ - Result display│             │ - NMS filtering │
│ - User controls │             │ - Polygon zones │
└─────────────────┘             └─────────────────┘
         │                               │
         │                               │
    ┌────▼────┐                     ┌────▼────┐
    │ Shared  │                     │ Shared  │
    │ Library │                     │ Library │
    │         │                     │         │
    │ - Protobuf messages           │ - OpenCV DNN
    │ - Frame/Polygon classes       │ - YOLO networks
    │ - Common utilities            │ - Signal handling
    │ - Logging system              │ - Detection logic
    └─────────┘                     └─────────┘
```

## VS Code Integration

This project includes VS Code configuration for:

- Building with CMake Tools extension
- Debugging with C/C++ extension
- Running tests with Test Explorer

### Recommended Extensions

- C/C++ Extension Pack
- CMake Tools
- Test Explorer UI

## Development

### Project Architecture

The system is organized as a modular C++ application:

- **`client/`** - Client application for frame capture and processing requests
- **`server/`** - Server application with neural network inference engine
- **`shared/`** - Common components including Protocol Buffer definitions
- **`tests/`** - Comprehensive unit test suite

### Code Style Guidelines

The project follows **Google C++ Style Guide** with modern C++23 features:

- **Namespace**: All code in `aa` namespace with sub-namespaces
- **Naming**: PascalCase for classes/methods, snake_case for variables
- **Headers**: Always use `#pragma once` for header guards
- **Logging**: Use `AA_LOG_*` macros instead of `std::cout/cerr`
- **Constructors**: Use `{}` initialization and "pass by value + move" idiom
- **Line limit**: 120 characters (use `.clang-format` for auto-formatting)

### Adding New Features

#### Adding New Source Files

1. **Headers**: Place in appropriate `include/` directory with `.h` extension
2. **Implementation**: Place in corresponding `src/` directory with `.cpp` extension
3. **CMake**: Update relevant `CMakeLists.txt` to include new sources
4. **Tests**: Create corresponding test files in `tests/` directory

#### Adding New Protocol Buffer Messages

1. **Define**: Add `.proto` files in `shared/proto/`
2. **Generate**: CMake automatically generates C++ code during build
3. **Implement**: Create C++ wrapper classes in `shared/include/` and `shared/src/`
4. **Test**: Add unit tests for message serialization/deserialization

#### Adding New Neural Network Models

1. **Format**: YOLO models (.weights + .cfg) or ONNX format for cross-platform compatibility
2. **Location**: Place models in `models/` directory
3. **Integration**: Update server preprocessing/postprocessing for new model input sizes
4. **Documentation**: Update model-specific parameters and usage
5. **COCO Classes**: Current implementation supports 80 COCO object classes (0-79):
   - Person, Vehicle, Animal, Furniture, Electronics, Sports, Food, and more
   - See [COCO dataset documentation](https://cocodataset.org/#explore) for complete class list

### Development Environment

**Recommended setup**: Use the provided devcontainer with VS Code for consistent development environment.

#### VS Code Extensions

- **C/C++ Extension Pack** - IntelliSense, debugging, formatting
- **CMake Tools** - Build system integration
- **Test Explorer UI** - Test execution and results
- **GitLens** - Git integration and history
- **Doxygen Documentation** - API documentation support

#### Available VS Code Tasks

- **Configure CMake** - Set up build configuration
- **Build Project** - Compile all targets
- **Run Tests** - Execute test suite
- **Generate Documentation** - Build API docs
- **Clean** - Remove build artifacts

## Performance and Optimization

### Model Performance

Supported neural network models:

- **YOLO (v3/v7)** - Object detection with COCO dataset (80 classes)
  - Input size: 640x640 (configurable)
  - Inference time: ~100-200ms on CPU
  - Memory usage: ~150-300MB depending on model size
  - Output: Bounding boxes with confidence scores and class IDs (0-79)
  - Features: Letterboxing preprocessing, NMS post-processing, coordinate scaling

### Detection Features

- **Polygon-based filtering**: Define arbitrary shaped detection zones
- **Inclusion zones**: Detect specified object classes within polygon areas
- **Exclusion zones**: Block all detections within polygon areas
- **Priority-based adjudication**: Higher priority polygons override lower priority ones
- **Class-specific filtering**: Target specific COCO object classes per polygon
- **Non-Maximum Suppression**: Removes duplicate detections with configurable IoU threshold
- **Confidence thresholding**: Filters detections below minimum confidence (0.25)

### System Requirements

**Minimum requirements:**

- 4GB RAM (8GB recommended)
- 2 CPU cores (4+ cores recommended)
- OpenCV 4.8.1+ with DNN support

**Optimal performance:**

- 16GB+ RAM for large models
- 8+ CPU cores or GPU acceleration
- SSD storage for model loading

### Deployment Options

#### Docker Deployment

```bash
# Build production image
docker build -t aa-video-processing:latest .

# Run server container with YOLO model
docker run -d \
    --name detector-server \
    -p 50051:50051 \
    -v ./models:/app/models:ro \
    aa-video-processing:latest \
    ./detector_server \
    --model=/app/models/yolov7x.weights \
    --cfg=/app/models/yolov7.cfg \
    --width=640 --height=640

# Run client container
docker run -it --rm \
    --network host \
    -v ./input:/app/input:ro \
    aa-video-processing:latest \
    ./detector_client \
    --input=/app/input/image.jpg \
    --width=640 --height=640
```

#### Cross-Compilation (Planned)

Future support for ARM64 embedded systems:

- **Target**: Rockchip RK3588 SoC
- **Use case**: Edge AI deployment
- **Performance**: Hardware-accelerated inference

## Troubleshooting

### Common Issues

#### Build Errors

```bash
# Missing protobuf compiler
sudo apt install protobuf-compiler libprotobuf-dev

# Missing gRPC libraries
sudo apt install libgrpc++-dev protobuf-compiler-grpc

# OpenCV DNN module not found
sudo apt install libopencv-contrib-dev
```

#### Runtime Issues

```bash
# Model file not found
./detector_server --model=./models/yolov7x.weights --cfg=./models/yolov7.cfg

# Port already in use
./detector_server --model=./models/yolov7x.weights --address=localhost:50052

# Verbose logging for debugging
./detector_server --model=./models/yolov7x.weights --cfg=./models/yolov7.cfg --verbose=true

# Client connection issues
./detector_client --input=input/000000039769.jpg --address=localhost:50051 --verbose=true
```

#### Performance Issues

- **High memory usage**: Reduce model complexity or batch size
- **Slow inference**: Consider GPU acceleration or model optimization
- **Network latency**: Use local deployment or optimize serialization

### Logging and Debugging

Enable verbose logging for detailed diagnostic information:

```bash
# Server debugging
./detector_server --model=./models/yolov7x.weights --cfg=./models/yolov7.cfg --verbose=true

# Client debugging
./detector_client --input=input/000000039769.jpg --address=localhost:50051 --verbose=true
```

Log levels available:

- `AA_LOG_ERROR` - Error conditions and failures
- `AA_LOG_WARNING` - Warnings and potential issues
- `AA_LOG_INFO` - General information and status
- `AA_LOG_DEBUG` - Detailed debugging information

## Contributing

### Development Workflow

1. **Fork** the repository
2. **Create** feature branch: `git checkout -b feature/amazing-feature`
3. **Follow** coding standards and run tests
4. **Commit** using [Conventional Commits](https://www.conventionalcommits.org/)
5. **Push** to branch: `git push origin feature/amazing-feature`
6. **Create** Pull Request

### Code Quality

Before submitting pull requests:

```bash
# Format code
clang-format -i src/**/*.cpp include/**/*.h

# Run all tests
cmake --build build --target test

# Check for memory leaks (optional)
valgrind --tool=memcheck ./build/tests/test_detector_server
```

### Commit Convention

Use [Conventional Commits](https://www.conventionalcommits.org/) format:

```text
feat: add support for YOLOv8 model format
fix: resolve memory leak in frame processing
docs: update API documentation for new endpoints
test: add unit tests for polygon detection zones
```

## License

This project is provided as an educational and development template. See individual components for specific licensing terms.

## Acknowledgments

- **OpenCV** community for computer vision libraries
- **gRPC** team for high-performance RPC framework
- **Protocol Buffers** for efficient serialization
- **Google Test** for comprehensive testing framework
