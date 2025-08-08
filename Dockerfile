ARG USER_NAME=ubuntu
FROM nvidia/cuda:12.9.1-cudnn-devel-ubuntu24.04 AS base

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
    -DWITH_CUDA=ON \
    -DENABLE_FAST_MATH=ON \
    -DCUDA_FAST_MATH=ON \
    -DWITH_CUBLAS=ON \
    -DWITH_TBB=ON \
    -DWITH_GTK=ON \
    -DOPENCV_DNN_CUDA=ON \
    -DWITH_NVCUVID=OFF \
    -DWITH_OPENCL=ON \
    -DWITH_GSTREAMER=ON \
    -DVIDEOIO_PLUGIN_LIST=gstreamer,ffmpeg \
    ../opencv && \
    ninja -j$(nproc) -l4 && ninja install && \
    ldconfig

FROM base AS development

RUN echo "${USER_NAME} ALL=(ALL:ALL) NOPASSWD: ALL" > /etc/sudoers.d/${USER_NAME}_user && \
    sed -i '/en_US.UTF-8/s/^# //g' /etc/locale.gen && \
    sed -i '/ru_RU.UTF-8/s/^# //g' /etc/locale.gen && \
    locale-gen

FROM nvidia/cuda:12.9.1-cudnn-runtime-ubuntu24.04 AS production

CMD [ "/bin/bash" ]
