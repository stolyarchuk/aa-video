# YoloX OpenCV Processing System

![YoloX OpenCV Processing System](output/output.png)

This is a small C++ video processing project. It uses gRPC and OpenCV to run YOLO models for object detection.

Docs: [https://stolyarchuk.github.io/aa-video/](https://stolyarchuk.github.io/aa-video/)

## What it does

- Runs YOLO-style models (YOLOv7/10/11, YOLOX) via OpenCV DNN.
- Serves detection over gRPC to clients.
- Filters detections with polygon zones (include/exclude, priorities).
- Uses RAII and bounds checks for safer code.

## Quick start

Requirements: CMake 3.20+, a C++23 compiler, OpenCV with DNN, protobuf & gRPC libraries.

Build (debug):

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Run the server with a model:

```bash
./build/server/detector_server --model=./models/yolox_s.onnx --verbose=true
```

Run the client (example):

```bash
./build/client/detector_client --input=input/000000039769.jpg --verbose=true
```

Run tests:

```bash
cmake --build build --target test
ctest --test-dir build --output-on-failure
```

## Polygons and filtering

- Inclusion zones: detect only listed classes inside them.
- Exclusion zones: block detections inside them.
- Priority: higher-priority polygons win in overlaps.
- Per-polygon class lists and coordinate scaling are supported.

See the docs for full examples and config format.

## Notes

- Expected CPU inference time: ~100–200 ms per frame (depends on model and CPU).
- Minimum: 4 GB RAM, 2 cores. More is better.

## Development

- Follow Google C++ style and C++23 idioms.
- Tests use Google Test.
- Use the provided devcontainer for a consistent environment.

## License

MIT — see the `LICENSE` file.

## Links

- Docs: [Docs](https://stolyarchuk.github.io/aa-video/)
- Issues: [Issues](https://github.com/stolyarchuk/aa-video/issues)
