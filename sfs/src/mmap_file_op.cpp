#include "mmap_file_op.h"

namespace sfs
{
    // 文件映射
    int MMapFileOperation::mmapFile(const MMapOption &mmap_option)
    {
        // 检验mmap操作合法性
        if (mmap_option.max_mmap_size_ < mmap_option.first_mmap_size_ || mmap_option.max_mmap_size_ == 0)
        {
            return TFS_ERROR;
        }
        int fd = checkFile_();
        if (fd < 0)
        {
            fprintf(stderr, "MMapFileOperation::mmap_file -check file failed.\n");
            return TFS_ERROR;
        }

        // 如果没有mmap
        if (!is_mapped_)
        {
            // 构建文件映射对象并映射
            map_file_ptr_.reset(new MMapFile(mmap_option, fd));
            is_mapped_ = map_file_ptr_->mapFile(true);
        }

        if (is_mapped_)
        {
            return TFS_SUCCESS;
        }
        else
        {
            return TFS_ERROR;
        }
    }

    // 文件解除映射
    int MMapFileOperation::munmapFile()
    {
        if (is_mapped_ && map_file_ptr_ != nullptr)
        {
            map_file_ptr_ = nullptr; // 原本智能指针管理的资源自动释放
            is_mapped_ = false;
        }
        return TFS_SUCCESS;
    }

    int MMapFileOperation::checkFile()
    {
        return checkFile_();
    }

    int
    MMapFileOperation::preadFile(char *buf, const int32_t size, const int64_t offset)
    {
        // 内存已经完成映射 (读虚拟内存)
        if (is_mapped_)
        {
            // 确认大小, 如果偏移+大小超过映射大小
            while ((offset + size) > map_file_ptr_->getSize())
            {
                if (DEBUG)
                {
                    // 32位系统下__PRI64_PREFIX为‘l'，64位系统下为‘ll’
                    fprintf(stderr, "mmap MMapFileOperation::pread_file file pread, size: %d, offset: %" __PRI64_PREFIX "d, map file size: %d. need remap\n",
                            size, offset, map_file_ptr_->getSize());
                }
                // 尝试内存追加
                if (!map_file_ptr_->remapFile())
                {
                    break;
                }
            }
            if ((offset + size) <= map_file_ptr_->getSize())
            {
                memcpy(buf, (char *)map_file_ptr_->getDataAddr() + offset, size);
                return TFS_SUCCESS;
            }
        }

        // 内存没完成映射或者映射空间不全(文件io读取数据)
        return FileOperation::preadFile(buf, size, offset);
    }

    int MMapFileOperation::pwriteFile(const char *buf, const int32_t size, const int64_t offset)
    { // 内存已经完成映射 (读虚拟内存)
        if (is_mapped_)
        {
            // 确认大小, 如果偏移+大小超过映射大小
            while ((offset + size) > map_file_ptr_->getSize())
            {
                if (DEBUG)
                {
                    // 32位系统下__PRI64_PREFIX为‘l'，64位系统下为‘ll’
                    fprintf(stderr, "mmap MMapFileOperation::pwrite_file file pwrite, size: %d, offset: %" __PRI64_PREFIX "d, map file size: %d. need remap\n",
                            size, offset, map_file_ptr_->getSize());
                }
                // 尝试内存追加
                if (!map_file_ptr_->remapFile())
                {
                    break;
                }
            }
            if ((offset + size) <= map_file_ptr_->getSize())
            {
                memcpy((char *)map_file_ptr_->getDataAddr() + offset, buf, size);
                return TFS_SUCCESS;
            }
        }

        // if (DEBUG)
        // {

        //     fprintf(stderr, "is_map:%d, mmap write failed\n", is_mapped_);
        // }
        // 内存没完成映射或者映射空间不全(文件io读取数据)
        return FileOperation::pwriteFile(buf, size, offset);
    }

    // 获取映射的数据
    void *MMapFileOperation::getMapDataAddr() const
    {
        if (is_mapped_)
        {
            return map_file_ptr_->getDataAddr();
        }
        return nullptr;
    }
    int MMapFileOperation::flushFile()
    {
        if (is_mapped_)
        {
            if (map_file_ptr_->syncFile())
            {
                return TFS_SUCCESS;
            }
            return TFS_ERROR;
        }

        // 如果没有内存映射
        return FileOperation::flushFile();
    }
}