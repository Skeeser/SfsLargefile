#include "mmap_file.h"

namespace sfs
{
    MMapFile::MMapFile() : size_(0), data_(NULL), fd_(-1)
    {
    }

    MMapFile::MMapFile(const int fd) : size_(0), data_(NULL), fd_(fd)
    {
    }
    MMapFile::MMapFile(const MMapOption &mmap_option, const int fd)
        : size_(0), data_(NULL), fd_(fd)
    {
        // 初始化映射选项
        mmap_file_option_.max_mmap_size_ = mmap_option.max_mmap_size_;
        mmap_file_option_.first_mmap_size_ = mmap_option.first_mmap_size_;
        mmap_file_option_.per_mmap_size_ = mmap_option.per_mmap_size_;
    }

    MMapFile::~MMapFile()
    {
        if (data_)
        {
            if (DEBUG)
                printf("mmap file destruct, fd: %d maped size: %d, data: %p\n", fd_, size_, data_);

            // 同步, 将内存写入磁盘文件
            msync(data_, size_, MS_SYNC);
            // 解除映射
            munmap(data_, size_);

            size_ = 0;
            data_ = NULL;
            mmap_file_option_.max_mmap_size_ = 0;
            mmap_file_option_.first_mmap_size_ = 0;
            mmap_file_option_.per_mmap_size_ = 0;
        }
    }

    // 同步文件
    bool MMapFile::syncFile()
    {
        if (data_ != NULL && size_ > 0)
        {
            return msync(data_, size_, MS_ASYNC) == 0;
        }

        return true;
    }

    // 将文件进行映射，同时设置访问权限
    bool MMapFile::mapFile(const bool write)
    {
        // 访问权限
        int flags = PROT_READ;
        if (write)
        {
            flags |= PROT_WRITE;
        }
        if (fd_ < 0)
        {
            return false;
        }

        if (mmap_file_option_.max_mmap_size_ == 0)
        {
            return false;
        }

        // 映射的大小最多只能是最大的内存映射大小
        if (size_ < mmap_file_option_.max_mmap_size_)
        {
            size_ = mmap_file_option_.first_mmap_size_;
        }
        else
        {
            size_ = mmap_file_option_.max_mmap_size_;
        }

        // 调整映射文件的大小
        if (!ensureFileSize_(size_))
        {
            fprintf(stderr, "ensure file size failed in map_file,size:%d\n", size_);
            return false;
        }

        // void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset);
        data_ = mmap(0, size_, flags, MAP_SHARED, fd_, 0);

        if (MAP_FAILED == data_)
        {
            fprintf(stderr, "map file failed:%s", strerror(errno));
            size_ = 0;
            fd_ = -1;
            data_ = NULL;
            return false;
        }

        if (DEBUG)
            printf("mmap file successed, fd: %d maped size: %d, data: %p\n", fd_, size_, data_);

        return true;
    }

    // 取映射到内存数据的首地址
    void *MMapFile::getDataAddr() const
    {
        return data_;
    }
    // 取映射到内存数据的大小
    int32_t MMapFile::getSize() const
    {
        return size_;
    }

    // 解除映射
    bool MMapFile::munmapFile()
    {
        if (munmap(data_, size_) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool MMapFile::remapFile()
    {
        if (fd_ < 0 || data_ == NULL)
        {
            fprintf(stderr, "mremap not mapped yet");
            return false;
        }

        if (size_ == mmap_file_option_.max_mmap_size_)
        {
            fprintf(stderr, "already mapped max size, now size: %d, max size: %d\n", size_, mmap_file_option_.max_mmap_size_);
            return false;
        }

        // 增长映射尺寸大小
        int32_t new_size = size_ + mmap_file_option_.per_mmap_size_;
        if (new_size > mmap_file_option_.max_mmap_size_)
        {
            new_size = mmap_file_option_.max_mmap_size_;
            return false;
        }

        // 重新设置文件大小
        if (!ensureFileSize_(new_size))
        {
            fprintf(stderr, "ensure file size failed in mremap. new size: %d\n", new_size);
            return false;
        }

        if (DEBUG)
            printf("mremap start. fd: %d, now size: %d, new size: %d, old data: %p\n", fd_, size_, new_size, data_);

        /*
         扩大（或缩小）现有的内存映射
         函数原型
         void * mremap(void *old_address, size_t old_size , size_t new_size, int flags);
         addr：     上一次已映射到进程空间的地址；
         old_size： 旧空间的大小；
         new_size： 重新映射指定的新空间大小；
         flags:     取值可以是0或者MREMAP_MAYMOVE，0代表不允许内核移动映射区域，
                 MREMAP_MAYMOVE则表示内核可以根据实际情况移动映射区域以找到一个符合
                 new_size大小要求的内存区域
         */
        void *new_map_data = mremap(data_, size_, new_size, MREMAP_MAYMOVE);
        if (MAP_FAILED == new_map_data)
        {
            fprintf(stderr, "ensure file size failed in mremap. new size: %d, error desc: %s\n", new_size, strerror(errno));
            return false;
        }
        else
        {
            if (DEBUG)
                printf("mremap success. fd: %d, now size: %d, new size: %d, old data: %p, new data: %p\n", fd_, size_, new_size,
                       data_, new_map_data);
        }
        data_ = new_map_data;
        size_ = new_size;
        return true;
    }

    // 调整映射文件的大小
    bool MMapFile::ensureFileSize_(const int32_t size)
    {
        // 存放文件状态
        struct stat s;
        if (fstat(fd_, &s) < 0)
        {
            fprintf(stderr, "fstat error describe:%s\n", strerror(errno));
            return false;
        }

        // 如果磁盘上文件的大小小于要内存映射的大小，那么就对磁盘上的文件大小进行扩容
        if (s.st_size < size)
        {
            // 大小扩容为size_
            if (ftruncate(fd_, size) < 0)
            {
                fprintf(stderr, "ftruncate error describe:%s\n", strerror(errno));
                return false;
            }
        }
        return true;
    }
}