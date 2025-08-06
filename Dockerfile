FROM ubuntu:24.04 AS builder

# Install basic packages and development tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    wget \
    curl \
    unzip \
    ca-certificates \
    gnupg \
    lsb-release \
    sudo \
    software-properties-common \
    && rm -rf /var/lib/apt/lists/*

# Install C++ development tools
RUN apt-get update && apt-get install -y \
    clang \
    clang-format \
    clang-tidy \
    gdb \
    valgrind \
    ninja-build \
    ccache \
    && rm -rf /var/lib/apt/lists/*

# Configure ccache
ENV CCACHE_DIR=/root/.ccache
ENV CC="ccache gcc"
ENV CXX="ccache g++"
RUN ccache --set-config=max_size=2G
RUN ccache --set-config=cache_dir=/root/.ccache
RUN ccache --set-config=compression=true

# Install dependencies for gRPC and OpenCV
RUN apt-get update && apt-get install -y \
    autoconf \
    libtool \
    libssl-dev \
    zlib1g-dev \
    libc-ares-dev \
    libabsl-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libgrpc++-dev \
    libgrpc-dev \
    protobuf-compiler-grpc \
    && rm -rf /var/lib/apt/lists/*

# Install OpenCV dependencies
RUN apt-get update && apt-get install -y \
    python3-opencv \
    libeigen3-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libprotobuf-dev \
    protobuf-compiler \
    libtbb-dev \
    libatlas-base-dev \
    liblapack-dev \
    libblas-dev \
    libhdf5-dev \
    && rm -rf /var/lib/apt/lists/*

# Install additional OpenCV DNN dependencies
RUN apt-get update && apt-get install -y \
    libgtk-3-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libv4l-dev \
    libxvidcore-dev \
    libx264-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
    gfortran \
    libopenexr-dev \
    libatlas-base-dev \
    python3-dev \
    python3-numpy \
    libtbbmalloc2 \
    libtbb-dev \
    libdc1394-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Google Test
RUN apt-get update && apt-get install -y \
    libgtest-dev \
    libgmock-dev \
    && rm -rf /var/lib/apt/lists/*

# Build and install a more recent version of OpenCV with DNN support from source
# This ensures we have the latest DNN capabilities
WORKDIR /tmp
RUN --mount=type=cache,target=/root/.ccache \
    --mount=type=cache,target=/var/cache/apt \
    --mount=type=cache,target=/var/lib/apt/lists \
    git clone --depth 1 --branch 4.12.0 https://github.com/opencv/opencv.git && \
    git clone --depth 1 --branch 4.12.0 https://github.com/opencv/opencv_contrib.git && \
    cd opencv && \
    mkdir build && cd build && \
    CC="ccache gcc" CXX="ccache g++" cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D INSTALL_PYTHON_EXAMPLES=OFF \
    -D INSTALL_C_EXAMPLES=OFF \
    -D OPENCV_ENABLE_NONFREE=ON \
    -D OPENCV_EXTRA_MODULES_PATH=/tmp/opencv_contrib/modules \
    -D PYTHON_EXECUTABLE=/usr/bin/python3 \
    -D BUILD_EXAMPLES=OFF \
    -D WITH_TBB=ON \
    -D WITH_V4L=ON \
    -D WITH_QT=OFF \
    -D WITH_GTK=ON \
    -D WITH_OPENGL=ON \
    -D OPENCV_DNN_CUDA=OFF \
    -D BUILD_opencv_dnn=ON \
    -D BUILD_opencv_python3=ON \
    -D CMAKE_C_COMPILER_LAUNCHER=ccache \
    -D CMAKE_CXX_COMPILER_LAUNCHER=ccache \
    .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    cd / && rm -rf /tmp/opencv /tmp/opencv_contrib

# Install Doxygen for documentation
RUN apt-get update && apt-get install -y \
    doxygen \
    graphviz \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /app
COPY . .

# Build the application
RUN --mount=type=cache,target=/root/.ccache \
    mkdir -p build && \
    cd build && \
    CC="ccache gcc" CXX="ccache g++" cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    .. && \
    make -j$(nproc)

# Development stage (for devcontainer)
FROM builder AS development

RUN echo 'ubuntu ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER ubuntu

CMD ["/bin/bash"]

# Production stage
FROM ubuntu:24.04 AS production

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libopencv-dev \
    libgrpc++-dev \
    libprotobuf-dev \
    libtbb-dev \
    libgtk-3-0 \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN groupadd -r appuser && useradd -r -g appuser appuser

# Copy built binaries
COPY --from=builder /app/build/client/detector_client /usr/local/bin/
COPY --from=builder /app/build/server/detector_server /usr/local/bin/

# Copy models and input directories
COPY --from=builder /app/models /app/models
COPY --from=builder /app/input /app/input

# Set working directory and user
WORKDIR /app
USER appuser

# Default command
CMD ["detector_server"]
