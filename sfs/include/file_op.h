#pragma once
#include "common.h"

// 文件操作相关
namespace sfs
{
    // 文件操作类
    class FileOperation
    {
    public:
        FileOperation(const std::string &file_name, const int open_flags = O_RDWR | O_LARGEFILE);
        ~FileOperation();

        int openFile();
        void closeFile();

        // 将数据写入到磁盘
        int flushFile();

        // 删除文件
        int unlinkFile();

        // 读文件，nbytes为读的大小，offset为偏移量
        virtual int preadFile(char *buf, const int32_t nbytes, const int64_t offset);
        // 写文件
        virtual int pwriteFile(const char *buf, const int32_t nbytes, const int64_t offset);
        // 与上面区别是带有偏移
        int writeFile(const char *buf, const int32_t nbytes);

        int64_t getFileSize();

        // 将文件大小改成length长度
        int ftruncateFile(const int64_t length);
        // 偏移文件指针，移动文件流的读写位置
        int seekFile(const int64_t offset);

        int getFd() const
        {
            return fd_;
        }

    protected:
        int fd_;
        int open_flags_;
        char *file_name_;

        int checkFile_();

    protected:
        // 0代表8进制，6代表文件拥有者拥有读写权限，后面两个4表示同组成员和其他成员拥有读的权限
        static const mode_t OPEN_MODE = 0644;

        // 如果读取磁盘文件失败最多尝试次数
        static const int MAX_DISK_TIMES = 5;
    };
}