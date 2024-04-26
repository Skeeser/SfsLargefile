#pragma once

#include "common.h"

namespace sfs
{
    // 文件映射
    class MMapFile
    {
    public:
        MMapFile();
        explicit MMapFile(const int fd);
        MMapFile(const MMapOption &mmap_option, const int fd);
        ~MMapFile();

        // 将文件进行映射，同时设置访问权限
        bool mapFile(const bool write = false);

        // 取映射到内存数据的首地址
        void *getDataAddr() const;
        // 取映射到内存数据的大小
        int32_t getSize() const;

        // 调用相关系统调用
        // 同步文件
        bool syncFile();
        // 解除映射
        bool munmapFile();
        // 重新映射, 增长映射内存的尺寸, 调用一次增长一次
        bool remapFile();

    private:
        // 调整映射文件的大小
        bool ensureFileSize_(const int32_t size);

    private:
        int32_t size_;
        int fd_;
        void *data_;
        struct MMapOption mmap_file_option_;
    };
}