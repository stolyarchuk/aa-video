# Docker Usage Guide

This document explains how to use the Docker images built for the AA Video Processing System.

## Usage Examples

### Running the Production Image

```bash
# Pull the latest production image
docker pull docker.io/stolyarchuk/aa-video:latest

# Run the detector server
docker run --rm -it --net host -v $(pwd)/models:/models docker.io/stolyarchuk/aa-video:latest \
  detector_server --model=/models/yolov7.weights --cfg=/models/yolov7.cfg

# Run the detector client with custom input
docker run --rm -it --net host -v $(pwd)/data:/data docker.io/stolyarchuk/aa-video:latest \
  detector_client --input=/data/input_image.jpg --output=/data/output_image.jpg
```
