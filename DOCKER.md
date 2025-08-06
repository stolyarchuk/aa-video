# Docker Usage Guide

This document explains how to use the Docker images built for the AA Video Processing System.

## Available Images

The CI/CD pipeline builds two image variants:

### Production Image (`latest`, version tags)

- **Registry**: `ghcr.io/stolyarchuk/aa-test:latest`
- **Purpose**: Minimal runtime image with only the built binaries and runtime dependencies
- **Size**: ~500MB (optimized)
- **Contents**:
  - Detector client and server binaries
  - Runtime libraries (OpenCV, gRPC, etc.)
  - Models and input directories
- **User**: Non-root `appuser`

### Development Image (`dev`, version tags with `-dev` suffix)

- **Registry**: `ghcr.io/stolyarchuk/aa-test:dev`
- **Purpose**: Full development environment with build tools
- **Size**: ~3GB (includes all dev dependencies)
- **Contents**:
  - Complete build toolchain (CMake, GCC, Clang)
  - Development tools (GDB, Valgrind, clang-format)
  - Source code and build artifacts
  - ccache for faster rebuilds
- **User**: `ubuntu` with sudo access

## Usage Examples

### Running the Production Image

```bash
# Pull the latest production image
docker pull ghcr.io/stolyarchuk/aa-test:latest

# Run the detector server
docker run --rm -p 50051:50051 ghcr.io/stolyarchuk/aa-test:latest detector_server

# Run the detector client with custom input
docker run --rm -v $(pwd)/my_image.jpg:/app/input/my_image.jpg \
  ghcr.io/stolyarchuk/aa-test:latest detector_client --input=/app/input/my_image.jpg
```

### Using the Development Image

```bash
# Pull the development image
docker pull ghcr.io/stolyarchuk/aa-test:dev

# Start an interactive development session
docker run -it --rm -v $(pwd):/workspace ghcr.io/stolyarchuk/aa-test:dev bash

# Inside the container, you can:
# - Build the project: cd /workspace && cmake -B build && make -C build
# - Run tests: cd /workspace/build && ctest
# - Use development tools: gdb, valgrind, clang-format, etc.
```

### Docker Compose Example

```yaml
version: '3.8'
services:
  detector-server:
    image: ghcr.io/stolyarchuk/aa-test:latest
    command: detector_server --address=0.0.0.0:50051 --model=/app/models/yolov7.weights
    ports:
      - "50051:50051"
    volumes:
      - ./models:/app/models:ro

  detector-client:
    image: ghcr.io/stolyarchuk/aa-test:latest
    command: detector_client --input=/app/input/test.jpg --address=detector-server:50051
    volumes:
      - ./input:/app/input:ro
    depends_on:
      - detector-server
```

## Image Tags

### Production Images

- `latest` - Latest stable build from main branch
- `dev` - Latest build from dev branch
- `v1.0.0` - Semantic version releases
- `v1.0` - Major.minor version
- `v1` - Major version

### Development Images

- `dev` - Latest development build from main branch
- `dev-dev` - Latest build from dev branch
- `v1.0.0-dev` - Development version of releases

## Authentication

Images are published to GitHub Container Registry (ghcr.io). For public repositories, no authentication is required for pulling. For private repositories:

```bash
# Login with GitHub Personal Access Token
echo $GITHUB_TOKEN | docker login ghcr.io -u USERNAME --password-stdin

# Or use GitHub CLI
gh auth token | docker login ghcr.io -u USERNAME --password-stdin
```

## Build Cache Optimization

The build process uses multiple caching layers:

1. **Docker Layer Cache**: Shared across builds via GitHub Actions cache
2. **ccache**: C++ compilation cache for faster rebuilds
3. **BuildKit Cache Mounts**: Persistent cache during build process

This results in:

- Initial build: ~15-20 minutes
- Subsequent builds: ~3-5 minutes (with cache hits)
- Development builds: ~60-80% faster compilation

## Local Development

For local development, you can build the images directly:

```bash
# Build production image
docker build --target production -t aa-test:local .

# Build development image
docker build --target development -t aa-test:dev-local .

# Use BuildKit for better caching
DOCKER_BUILDKIT=1 docker build --target production -t aa-test:local .
```

## Health Checks and Monitoring

The production image includes basic health check capabilities:

```bash
# Check if binaries are working
docker run --rm ghcr.io/stolyarchuk/aa-test:latest detector_server --help
docker run --rm ghcr.io/stolyarchuk/aa-test:latest detector_client --help

# Test with sample input
docker run --rm ghcr.io/stolyarchuk/aa-test:latest \
  detector_client --input=/app/input/000000039769.jpg --verbose
```

## Troubleshooting

### Common Issues

1. **Permission Denied**: Production image runs as non-root user

   ```bash
   # Mount volumes with proper permissions
   docker run --rm -v $(pwd)/input:/app/input:ro ghcr.io/stolyarchuk/aa-test:latest
   ```

2. **Missing Models**: Ensure model files are available

   ```bash
   # Check what models are included
   docker run --rm ghcr.io/stolyarchuk/aa-test:latest ls -la /app/models/
   ```

3. **Network Issues**: Use proper port mapping for gRPC

   ```bash
   # Server needs port 50051 exposed
   docker run --rm -p 50051:50051 ghcr.io/stolyarchuk/aa-test:latest detector_server
   ```

### Getting Help

- View container logs: `docker logs <container_id>`
- Interactive debugging: `docker run -it --rm ghcr.io/stolyarchuk/aa-test:dev bash`
- Check image contents: `docker run --rm ghcr.io/stolyarchuk/aa-test:latest find /app`

## Security Considerations

- Production images run as non-root user `appuser`
- Minimal attack surface with only runtime dependencies
- Regular base image updates via Dependabot
- Vulnerability scanning in CI/CD pipeline
- No secrets or credentials stored in images
