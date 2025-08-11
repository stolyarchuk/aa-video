# Docker Usage Guide

This document explains how to use the Docker images built for the AA Video Processing System.

## Usage Examples

### Running the Production Image

```bash
# Pull the latest production image
docker pull ghcr.io/stolyarchuk/aa-video:latest

# Run the detector server
docker run --rm -it --net host ghcr.io/stolyarchuk/aa-video:latest

# Run the detector client with custom input
docker run --rm -it --net host -v $(pwd)/data:/data ghcr.io/stolyarchuk/aa-video:latest \
  detector_client --input=/data/input_image.jpg --output=/data/output_image.jpg
```

## X11 Prerequisites for GUI Applications

When running applications that require X11 display (GUI applications), you need to set up X11 authentication and mount the necessary directories.

### X11 Authentication Setup

The `.devcontainer/initialize.sh` script prepares X11 authentication for Docker containers by creating a Docker-specific Xauth file. This script:

- Creates `/tmp/.docker.xauth` if it doesn't exist
- Extracts the current X11 authorization from `$DISPLAY`
- Converts the authorization to a Docker-compatible format (replacing hostname with `ffff`)
- Merges the authorization into the Docker Xauth file

Run this script on the host before starting containers that need X11 access:

```bash
bash .devcontainer/initialize.sh
```

### Required Docker Mounts

For X11 applications to work properly in Docker, you need these volume mounts:

```bash
--mount source=/tmp/.X11-unix,target=/tmp/.X11-unix,type=bind,consistency=cached \
--mount source=/tmp/.docker.xauth,target=/tmp/.docker.xauth,type=bind,consistency=cached
```

### Additional Device and User Mappings

For hardware acceleration and proper user permissions:

```bash
--device=/dev/dri \
--userns=keep-id
```
