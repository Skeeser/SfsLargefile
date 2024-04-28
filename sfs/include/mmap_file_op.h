#pragma once

#include "common.h"
#include "file_op.h"
#include "mmap_file.h"

#include <memory>

namespace sfs
{
    class MMapFileOperation : public FileOperation
    {
    public:
        explicit MMapFileOperation(const std::string &file_name, const int open_flags = O_CREAT | O_RDWR | O_LARGEFILE) : FileOperation(file_name, open_flags), map_file_ptr_(nullptr), is_mapped_(false)
        {
        }

        ~MMapFileOperation()
        {
        }

        // 文件映射
        int mmapFile(const MMapOption &mmap_option);
        // 文件解除映射
        int munmapFile();

        int preadFile(char *buf, const int32_t size, const int64_t offset) override;
        int pwriteFile(const char *buf, const int32_t size, const int64_t offset) override;

        // 获取映射的数据
        void *getMapDataAddr() const;
        int flushFile();

    private:
        // 映射文件
        std::unique_ptr<MMapFile> map_file_ptr_;
        // 是否映射
        bool is_mapped_;
    };
}