#include "gtest/gtest.h"

#include "common.h"
#include "mmap_file.h"

using namespace sfs;
using namespace std;

static const mode_t OPEN_MODE = 0644;
const MMapOption mmap_option = {10240000, 4096, 4096};

int open_file(string file_name, int open_flags)
{
    int fd = open(file_name.c_str(), open_flags, OPEN_MODE);
    if (fd < 0)
    {
        // 返回errno，这样是为了避免errno重置
        return -errno;
    }

    return fd;
}
TEST(sfs, mmap_file)
{
    const char *filename = "./mapfile_test.txt";
    // 1.打开/创建一个文件，取得文件的句柄
    int fd = open_file(filename, O_RDWR | O_CREAT | O_LARGEFILE);

    ASSERT_FALSE(fd < 0) << "open file failed.filename:" << filename << "error desc:" << strerror(-fd);

    printf("fd:%d\n", fd);

    // 创建mmap对象
    MMapFile *map_file = new MMapFile(mmap_option, fd);

    bool is_mapped = map_file->mapFile(true);
    // 映射成功
    ASSERT_TRUE(is_mapped) << "map file failed";

    // 追加内存
    map_file->remapFile();
    fprintf(stderr, "data:%p,size:%d\n", map_file->getDataAddr(), map_file->getSize());

    // 写数据
    memset(map_file->getDataAddr(), '8', map_file->getSize());

    // 同步文件
    ASSERT_TRUE(map_file->syncFile());
    ASSERT_TRUE(map_file->munmapFile());

    close(fd);
}