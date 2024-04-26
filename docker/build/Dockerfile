FROM ubuntu:20.04

# 禁止交互式提示
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=Asia/Shanghai

# 设置默认的shell位置
SHELL ["/bin/bash", "-c"]

# 配置apt, 删除软件包和配置软件源
RUN apt-get clean && \
    apt-get autoclean

COPY apt/sources.list /etc/apt/


RUN apt-get update  && apt-get upgrade -y  && \
    apt-get install -y \
    htop \
    apt-utils \
    curl \
    cmake \
    gdb \
    git \
    openssh-server \
    build-essential \
    libboost-all-dev \
    libgoogle-glog-dev \
    net-tools \
    vim 

RUN apt-get install -y libc-ares-dev  libssl-dev gcc g++ make 

# 复制运行依赖安装脚本
COPY install/cmake /tmp/install/cmake
RUN /tmp/install/cmake/install_cmake.sh

COPY install/gtest /tmp/install/gtest
RUN /tmp/install/gtest/install_gtest.sh

