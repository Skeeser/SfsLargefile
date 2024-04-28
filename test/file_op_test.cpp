#include "gtest/gtest.h"

#include "file_op.h"
#include "common.h"
#define DATASIZE 64
using namespace sfs;
using namespace std;

TEST(sfs, file_op)
{
    const char *filename = "./file_op.txt";
    FileOperation *fileOp = new FileOperation(filename, O_RDWR | O_CREAT | O_LARGEFILE);

    int fd = fileOp->openFile();
    ASSERT_FALSE(fd < 0) << "open file " << filename << " faild，reason:\n"
                         << strerror(-fd);

    char buffer[DATASIZE + 1];
    memset(buffer, '8', DATASIZE);
    // 将数据写入到文件中，偏移为1024,数据大小为DATASIZE
    int ret = fileOp->pwriteFile(buffer, DATASIZE, 2 * DATASIZE);

    ASSERT_FALSE(ret < 0) << "pwriteFile";
    // {
    //     if (ret == FEXIT_DISK_OPER_INCOMPLETE)
    //     {
    //         fprintf(stderr, "pwrite file write length is less than required!\n");
    //     }
    //     else
    //     {
    //         fprintf(stderr, "pwrite file %s faild. reason:%s\n", filename, strerror(-ret));
    //     }
    //     return -1;
    // }

    fprintf(stdout, "pwrite file %s success!\n", filename);

    memset(buffer, 0, DATASIZE);
    ret = fileOp->preadFile(buffer, DATASIZE, 2 * DATASIZE);
    ASSERT_FALSE(ret < 0) << "preadFile";
    // {
    //     if (ret == FEXIT_DISK_OPER_INCOMPLETE)
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
    buffer[DATASIZE] = '\0';
    fprintf(stdout, "pread file %s success!\n", filename);
    fprintf(stdout, "data is %s\n", buffer);

    memset(buffer, '9', DATASIZE);
    ret = fileOp->writeFile(buffer, DATASIZE);
    ASSERT_FALSE(ret < 0) << "writeFile";
    // {
    //     if (ret == FEXIT_DISK_OPER_INCOMPLETE)
    //     {
    //         fprintf(stderr, "write file write length is less than required!\n");
    //     }
    //     else
    //     {
    //         fprintf(stderr, "write file %s faild. reason:%s\n", filename, strerror(-ret));
    //     }
    //     return -1;
    // }

    // 关闭文件
    fileOp->closeFile();
    // 删除文件
    fileOp->unlinkFile();
}