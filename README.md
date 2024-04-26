<div align="center">

<img alt="LOGO" src="assets/logo1.png" height="200" />

# SfsLargefile

<br>

<div>
    <img alt="c++" src="https://img.shields.io/badge/c++-11-%2300599C">
</div>
<div>
    <img alt="platform" src="https://img.shields.io/badge/platform-Linux%20-blueviolet">
</div>
<div>
    <img alt="license" src="https://img.shields.io/github/license/Skeeser/SfsLargefile">
    <img alt="commit" src="https://img.shields.io/github/commit-activity/m/Skeeser/SfsLargefile?color=%23ff69b4">
    <img alt="stars" src="https://img.shields.io/github/stars/Skeeser/SfsLargefile?style=social">
</div>
<br>

[简体中文](README_ZH.md)  | [English](README_EN.md) 

分布式文件系统引擎，核心部分大文件结构的实现，利用**大文件块**、**特殊的数据结构**以及**内存映射**等操作对大型数据和海量小型数据进行高效管理（更多针对海量小型数据的存储、插入、查询、读取以及删除） <br>

</div>

<br>

## 功能特性


<br>

## 文件结构


<br>


## 系统架构
Linux  

<br>


## 应用技术
C++、Docker、gRPC、protobuf、Cmake、异步日志

<br>

## 构建
### 先构建docker容器
#### 镜像创建 
```shell
cd docker/scripts
./docker_build.sh
```

#### 创建容器
```shell
./docker_run.sh
```

#### 进入容器
```shell
./docker_into.sh
```

#### 删除容器
```shell
./docker_stop_and_run.sh
```

<br>

### 然后编译
进入容器后
```shell
cd work
mkdir build
cd build
cmake ..
make -j8
```

<br>

## 运行
### 启动grpc服务(主服务器)
在`build`文件夹下  
```shell
./rpc_manager/server/server
```

### 启动监控(需要监控的服务器)
```shell
./monitor_work/src/monitor
```

### 展示数据
- [ ] 前端待做

<br>

## 使用指南

<!-- 描述如何使用该项目 -->
<br>

## 如何贡献
如果你碰巧看见这个项目, 想要参与开发  
可以查看这个文档 [如何参与开源项目](doc/github参与开源项目流程.md)  

如果想了解这个项目, 可以查看[开发文档](doc/dev.md)  

<br>

## 许可证
GPL  