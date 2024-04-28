#include "gtest/gtest.h"
#include "common.h"
#include "mmap_file_op.h"
#include <iostream>
using namespace std;
using namespace sfs;

const MMapOption mmap_option = {10240000, 4096, 4096};
TEST(sfs, mmap_file_op)
{
    int ret = 0;
    const char *filename = "mmap_file_op.txt";
    MMapFileOperation *mmfo = new MMapFileOperation(filename);

    int fd = mmfo->openFile();
    ASSERT_FALSE(fd < 0) << "open file " << filename << " faild，reason:\n"
                         << strerror(-fd);

    ret = mmfo->mmapFile(mmap_option);
    ASSERT_FALSE(ret == TFS_ERROR) << "mmap_file failed，reason：" << strerror(errno);

    char buffer[128 + 1];
    memset(buffer, '6', 128);
    buffer[127] = '\n';
    // 测试写
    ret = mmfo->pwriteFile(buffer, 128, 8);
    ASSERT_FALSE(ret < 0) << "pwrite_mmap";
    // {
    //     if (ret == EXIT_DISK_OPER_INCOMPLETE)
    //     {
    //         fprintf(stderr, "write file write length is less than required!\n");
    //     }
    //     else
    //     {
    //         fprintf(stderr, "write file %s faild. reason:%s\n", filename, strerror(-ret));
    //     }
    // }

    // 测试读
    memset(buffer, 0, 128);
    ret = mmfo->preadFile(buffer, 128, 8);
    ASSERT_FALSE(ret < 0) << "pread";
    // {
    //     if (ret == EXIT_DISK_OPER_INCOMPLETE)
    //     {
    //         fprintf(stderr, "pread file write length is less than required!\n");
    //     }
    //     else
    //     {
    //         fprintf(stderr, "pread file %s faild. reason:%s\n", filename, strerror(-ret));
    //     }
    //     return -1;
    // }

    // 设置字符串结束符
    buffer[128] = '\0';
    fprintf(stdout, "pread file %s success!\n", filename);
    fprintf(stdout, "data is %s\n", buffer);

    // 同步文件
    ret = mmfo->flushFile();
    ASSERT_FALSE(ret == TFS_ERROR) << "flush file" << filename << "faild. reason:\n"
                                   << strerror(errno);

    memset(buffer, '8', 128);
    buffer[127] = '\n';
    // 调用文件io写入
    ret = mmfo->pwriteFile(buffer, 128, 4000);
    ASSERT_FALSE(ret < 0) << "pwrite_io";

    ret = mmfo->munmapFile();
    mmfo->closeFile();
}
