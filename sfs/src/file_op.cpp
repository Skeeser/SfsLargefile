#include "file_op.h"
#include "common.h"

namespace sfs
{
    FileOperation::FileOperation(const std::string &file_name, const int open_flags) : fd_(-1), open_flags_(open_flags)
    {
        // 重新分配一份内存，将file_name赋值给file_name_
        file_name_ = strdup(file_name.c_str());
    }
    FileOperation::~FileOperation()
    {
        if (fd_ > 0)
        {
            // 调用库文件的close，而不是咱们作用域的close
            ::close(fd_);
        }

        if (file_name_ != NULL)
        {
            free(file_name_);
            file_name_ = NULL;
        }
    }

    int FileOperation::openFile()
    {
        if (fd_ > 0)
        {
            ::close(fd_);
            fd_ = -1;
        }

        // 创建文件夹
        std::string tmp_str = file_name_;
        int pos = tmp_str.find_last_of('/');
        if (pos != std::string::npos)
        {
            tmp_str.erase(pos, tmp_str.length() - pos);
            if (tmp_str != ".")
            {
                std::filesystem::create_directories(tmp_str);
            }
        }

        fd_ = ::open(file_name_, open_flags_, OPEN_MODE);
        if (fd_ < 0)
        {
            return -errno;
        }
        return fd_;
    }
    void FileOperation::closeFile()
    {
        if (fd_ < 0)
        {
            return;
        }

        ::close(fd_);
        // 重新置为-1
        fd_ = -1;
    }

    // 将数据写入到磁盘
    int FileOperation::flushFile()
    {
        // 如果我们在设置open_flag的时候设置了O_SYNC那么说明本来就会做同步
        if (open_flags_ & O_SYNC)
        {
            return 0;
        }
        // 确保文件打开
        int fd = checkFile_();
        if (fd < 0)
        {
            return fd;
        }

        return fsync(fd);
    }

    // 删除文件
    int FileOperation::unlinkFile()
    {
        int fd = checkFile_();
        if (fd < 0)
        {
            return fd;
        }
        return ::unlink(file_name_);
    }

    /*
     * （1）库文件中pread和pwrite读写文件以后不会移动文件指针
     * （2）库文件中write和read读写文件以后会移动文件指针
     * （3）如果遇到errno为EINTR(系统繁忙) EAGAIN(再次尝试)可以继续尝试读磁盘
     */
    // 读文件，nbytes为读的大小，offset为偏移量
    int FileOperation::preadFile(char *buf, const int32_t nbytes, const int64_t offset)
    {
        // 记录要读的剩余字节数
        int32_t left = nbytes;
        // 读文件的偏移
        int64_t read_offset = offset;
        char *p_tmp = buf;
        int64_t read_len = 0;
        // 最多只能读MAX_DISK_TIMES次磁盘（如果读文件失败的话）
        int i = 0;
        while (left > 0)
        {
            i++;
            if (i >= MAX_DISK_TIMES)
            {
                break;
            }

            int fd = checkFile_();
            if (fd < 0)
            {
                return fd;
            }

            read_len = ::pread64(fd_, p_tmp, left, read_offset);
            if (read_len < 0)
            {
                read_len = -errno;

                // 临时中断或者叫我们继续尝试
                if (-read_len == EINTR || -read_len == EAGAIN)
                {
                    continue;
                }
                else if (EBADF == -read_len)
                {
                    fd_ = -1;
                    continue;
                }
                else
                {
                    return read_len;
                }
            }
            else if (read_len == 0)
            {
                break;
            }

            // 读出来的字节数>0
            left -= read_len;
            p_tmp += read_len;
            read_offset += read_len;
        }

        // 如果还有没读完的数据
        if (left != 0)
        {
            // 磁盘未读取完成
            return EXIT_DISK_OPER_INCOMPLETE;
        }

        return TFS_SUCCESS;
    }
    // 写文件
    int FileOperation::pwriteFile(const char *buf, const int32_t nbytes, const int64_t offset)
    {
        // 记录要写的剩余字节数
        int32_t left = nbytes;
        // 读文件的偏移
        int64_t written_offset = offset;
        const char *p_tmp = buf;
        int64_t written_len = 0;

        // 最多只能读MAX_DISK_TIMES次磁盘（如果读文件失败的话）
        int i = 0;
        while (left > 0)
        {
            i++;
            if (i >= MAX_DISK_TIMES)
            {
                break;
            }

            int fd = checkFile_();
            if (fd < 0)
            {
                return fd;
            }

            written_len = ::pwrite64(fd_, p_tmp, left, written_offset);
            // std::cout << " pwrite ret : " << fd_ << std::endl;
            if (written_len < 0)
            {
                written_len = -errno;

                // 临时中断或者叫我们继续尝试
                if (-written_len == EINTR || -written_len == EAGAIN)
                {
                    continue;
                }
                else if (EBADF == -written_len)
                {
                    fd_ = -1;
                    continue;
                }
                else
                {
                    return written_len;
                }
            }

            // 读出来的字节数>0
            left -= written_len;
            p_tmp += written_len;
            written_offset += written_len;
        }

        // 如果还有没读完的数据
        if (left != 0)
        {
            // 磁盘未读取完成
            return EXIT_DISK_OPER_INCOMPLETE;
        }

        return TFS_SUCCESS;
    }

    int FileOperation::writeFile(const char *buf, const int32_t nbytes)
    {
        // 记录要写的剩余字节数
        int32_t left = nbytes;

        const char *p_tmp = buf;
        int64_t written_len = 0;

        // 最多只能读MAX_DISK_TIMES次磁盘（如果读文件失败的话）
        int i = 0;
        while (left > 0)
        {
            i++;
            if (i >= MAX_DISK_TIMES)
            {
                break;
            }

            int fd = checkFile_();
            if (fd < 0)
            {
                return fd;
            }

            written_len = ::write(fd_, p_tmp, left);

            if (written_len < 0)
            {
                written_len = -errno;

                // 临时中断或者叫我们继续尝试
                if (-written_len == EINTR || -written_len == EAGAIN)
                {
                    continue;
                }
                else if (EBADF == -written_len)
                {
                    fd_ = -1;
                    continue;
                }
                else
                {
                    return written_len;
                }
            }

            // 读出来的字节数>0
            left -= written_len;
            p_tmp += written_len;
        }

        // 如果还有没读完的数据
        if (left != 0)
        {
            // 磁盘未读取完成
            return EXIT_DISK_OPER_INCOMPLETE;
        }

        return TFS_SUCCESS;
    }

    int64_t FileOperation::getFileSize()
    {
        int fd = checkFile_();
        if (fd < 0)
        {
            return -1;
        }
        // 获取文件大小, 利用fstat函数
        struct stat s;
        // 获取文件状态
        if (fstat(fd, &s) != 0)
        {
            return -errno;
        }

        return s.st_size;
    }

    // 将文件大小改成length长度
    int FileOperation::ftruncateFile(const int64_t length)
    {
        int fd = checkFile_();
        if (fd < 0)
        {
            return fd;
        }

        return ftruncate(fd, length);
    }

    int FileOperation::seekFile(const int64_t offset)
    {
        int fd = checkFile_();

        if (fd < 0)
        {
            return fd;
        }

        // SEEK_SET是相对于文件头部的位置
        return lseek(fd, offset, SEEK_SET);
    }

    int FileOperation::checkFile_()
    {
        if (fd_ < 0)
        {
            fd_ = openFile();
        }

        return fd_;
    }
}