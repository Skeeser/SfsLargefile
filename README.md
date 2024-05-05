<div align="center">

<img alt="LOGO" src="assets/logo1.png" height="200" />

# SfsLargefile

<br>

<div>
    <img alt="c++" src="https://img.shields.io/badge/c++-17-%2300599C">
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
- 以block文件的形式存放数据文件(一般64M一个block)，每个块都有唯一的一个整数编号，块在使用之前所用到的存储空间都会预先分配和初始化。

- 每一个块由一个索引文件、一个主块文件和若干个扩展块组成，“小文件”主要存放在主块中，扩展块主要用来存放溢出的数据。

- 每个索引文件存放对应的块信息和“小文件”索引信息，索引文件会在服务启动是映射（mmap）到内存，以便极大的提高文件检索速度。“小文件”索引信息采用在索引文件中的数据结构哈希链表来实现。

- 每个文件有对应的文件编号，文件编号从1开始编号，依次递增，同时作为哈希查找算法的Key 来定位“小文件”在主块和扩展块中的偏移量。文件编号+块编号按某种算法可得到“小文件”对应的文件名。



<br>

## 文件结构
./
├── assets  
├── doc  文档  
├── docker  
├── sfs  sfs相关代码  
└── test  功能的单元测试  
  
<br>


## 系统架构
Linux  

<br>


## 应用技术
C++、Docker、Cmake、gtest、内存映射、文件操作

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
### 进行单元测试
```shell
./test/test  
```

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