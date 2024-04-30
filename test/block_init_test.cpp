#include "gtest/gtest.h"

// 测试块的初始化
#include "index_handle.h"
#include "file_op.h"
#include "common.h"
#include <sstream>

using namespace sfs;
using namespace std;

// 内存映射参数
const static MMapOption mmap_option = {1024000, 4096, 4096};
// 主块文件大小
const static uint32_t main_blocksize = 1024 * 1024 * 64;
// 哈希桶的大小
const static uint32_t bucket_size = 1000;

TEST(sfs, block_init)
{
    string mainblock_path = "";
    string index_path = "";
    int32_t ret = TFS_SUCCESS;
    int32_t block_id = 1;
    block_id = 8;

    std::stringstream tmp_streams;
    tmp_streams << "." << MAINBLOCK_DIR_PREFIX << block_id;
    tmp_streams >> mainblock_path;

    cout << "mainblock_path:" << mainblock_path << endl;

    // 创建主块操作对象
    FileOperation *mainblock = new FileOperation(mainblock_path, O_RDWR | O_CREAT | O_LARGEFILE);
    // int fd = mainblock->openFile();
    // ASSERT_TRUE(fd > 0);

    // 生成索引文件
    IndexHandle *index_handle = new IndexHandle(".", block_id);

    fprintf(stdout, "init index file...\n");

    ret = index_handle->create(block_id, bucket_size, mmap_option);
    ASSERT_FALSE(ret != TFS_SUCCESS) << "ret:" << ret << " create index " << block_id << " failed. \n";

    // 生成主块文件
    // 创建主块文件（调整为设置的指定大小）
    ret = mainblock->ftruncateFile(main_blocksize);

    ASSERT_FALSE(ret != TFS_SUCCESS) << "create main block " << block_id << " failed. \n";

    mainblock->closeFile();
    index_handle->flush();

    delete mainblock;
    delete index_handle;
}