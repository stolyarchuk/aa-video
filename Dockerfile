FROM debian:12-slim

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
    libopencv-dev \
    libopencv-contrib-dev \
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
    openexr \
    libatlas-base-dev \
    python3-dev \
    python3-numpy \
    libtbbmalloc2 \
    libtbb-dev \
    libjpeg-dev \
    libpng-dev \
    libtiff-dev \
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
RUN git clone --depth 1 --branch 4.8.1 https://github.com/opencv/opencv.git && \
    git clone --depth 1 --branch 4.8.1 https://github.com/opencv/opencv_contrib.git && \
    cd opencv && \
    mkdir build && cd build && \
    cmake -D CMAKE_BUILD_TYPE=RELEASE \
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
    -D WITH_GTK=OFF \
    -D WITH_OPENGL=ON \
    -D OPENCV_DNN_CUDA=OFF \
    -D BUILD_opencv_dnn=ON \
    -D BUILD_opencv_python3=ON \
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

# Create vscode user
RUN groupadd --gid 1000 vscode \
    && useradd --uid 1000 --gid vscode --shell /bin/bash --create-home vscode \
    && echo 'vscode ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Set working directory
WORKDIR /workspace

# Change ownership to vscode user
RUN chown -R vscode:vscode /workspace

USER vscode

# Verify installations
RUN pkg-config --modversion opencv4 && \
    pkg-config --exists grpc++ && \
    echo "gRPC and OpenCV with DNN support installed successfully"

CMD ["/bin/bash"]
