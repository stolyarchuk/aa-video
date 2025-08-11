ARG USER_NAME=ubuntu
FROM docker.io/ubuntu:24.04 AS base

ARG OPENCV_VERSION=4.12.0
ARG GRPC_VERSION=1.74.1

ENV DEBIAN_FRONTEND=noninteractive \
    APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=DontWarn

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    apt-get update -q && \
    apt-get install -qy --no-install-recommends --no-install-suggests \
    apt-utils sudo pkg-config build-essential gnupg2 wget binutils curl git python3.12-venv \
    ca-certificates gcc g++ gdb make cmake locales ccache vim ninja-build valgrind autoconf libtool \
    doxygen python3-dev autotools-dev libicu-dev libbz2-dev clang-19 clang-format-19 clang-tidy-19 \
    libgtest-dev libgmock-dev \
    libprotobuf-dev protobuf-compiler libgrpc-dev libgrpc++-dev protobuf-compiler-grpc \
    yasm gettext  libtool autoconf autopoint automake libgstreamer-plugins-base1.0-dev libgstreamer1.0-dev gstreamer1.0-tools \
    libfreetype6-dev libfreetype6 unzip nasm libharfbuzz-bin libharfbuzz-dev libvpx-dev openssl libgcrypt20-dev libva-dev \
    libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libgtk-3-dev \
    libnuma-dev libopenblas-dev liblapacke-dev libdc1394-dev python3-dev libeigen3-dev python3-numpy libtbb-dev && \
    ln -s /usr/include/lapacke.h /usr/include/x86_64-linux-gnu

RUN --mount=type=cache,target=/root/.ccache \
    git clone -b ${OPENCV_VERSION} https://github.com/opencv/opencv.git --depth=1 && \
    git clone -b ${OPENCV_VERSION} https://github.com/opencv/opencv_contrib.git --depth=1 && \
    mkdir opencv_build && cd opencv_build && \
    cmake \
    -GNinja \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DOPENCV_EXTRA_MODULES_PATH=../opencv_contrib/modules \
    -DEIGEN_INCLUDE_PATH=/usr/include/eigen3 \
    -DOPENCV_ENABLE_NONFREE=OFF \
    -DOPENCV_GENERATE_PKGCONFIG=ON \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_DOCS=OFF \
    -DBUILD_OPENCV_LEGACY=OFF \
    -DBUILD_TESTS=OFF \
    -DBUILD_PERF_TESTS=OFF \
    -DBUILD_OPENCV_PYTHON2=OFF \
    -DBUILD_OPENCV_PYTHON3=ON \
    -DENABLE_FAST_MATH=ON \
    -DWITH_CUDA=OFF \
    -DWITH_TBB=ON \
    -DWITH_GTK=ON \
    -DOPENCV_DNN_CUDA=OFF \
    -DWITH_NVCUVID=OFF \
    -DWITH_OPENCL=ON \
    -DWITH_GSTREAMER=ON \
    -DVIDEOIO_PLUGIN_LIST=gstreamer,ffmpeg \
    ../opencv && \
    ninja -j$(nproc) -l4 && ninja install && \
    ldconfig

# Copy and build the codebase
COPY . /app
WORKDIR /app

RUN rm -rf /app/build && cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release

FROM base AS development
ARG USER_NAME

ENV DEBIAN_FRONTEND=noninteractive \
    APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=DontWarn

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    apt-get update -q && \
    apt-get install -qy --no-install-recommends openssh-client && \
    apt-get clean all && \
    rm -rf /var/lib/apt/lists/*

RUN echo "${USER_NAME} ALL=(ALL:ALL) NOPASSWD: ALL" > /etc/sudoers.d/${USER_NAME}_user && \
    sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && \
    sed -i '/ru_RU.UTF-8/s/^# //g' /etc/locale.gen && \
    locale-gen

FROM docker.io/ubuntu:24.04 AS production
ARG USER_NAME

ENV DEBIAN_FRONTEND=noninteractive \
    APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=DontWarn

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt/lists,sharing=locked \
    apt-get update -q && \
    apt-get install -qy --no-install-recommends --no-install-suggests \
    libprotobuf32t64 libgrpc++1.51 libgrpc29 \
    libtbb12 libfreetype6 libharfbuzz0b \
    libgstreamer1.0-0 libgstreamer-plugins-base1.0-0 \
    libavcodec60 libavformat60 libavutil58 \
    libgtk-3-0 libdc1394-25 libopenblas0 libwebpdemux2 \
    ca-certificates && \
    apt-get clean all && \
    rm -rf /var/lib/apt/lists/*

# Copy OpenCV libraries from builder stage
COPY --from=base /usr/local/bin/ /usr/local/bin/
COPY --from=base /usr/local/lib/ /usr/local/lib/
COPY --from=base /usr/local/include/ /usr/local/include/
COPY --from=base /usr/local/share/ /usr/local/share/

# Copy application binaries from builder stage
COPY --from=base /app/build/server/detector_server /usr/local/bin/
COPY --from=base /app/build/client/detector_client /usr/local/bin/
COPY --from=base /app/input /input
COPY --from=base /app/models /models

# Update library cache
RUN ldconfig

CMD [ "detector_server", "-m=/models/yolox_s.onnx" ]
