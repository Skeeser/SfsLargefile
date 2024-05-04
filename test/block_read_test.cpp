#include "gtest/gtest.h"

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

static int32_t block_id = 1;

TEST(sfs, block_read)
{
    string mainblock_path = "";
    string index_path = "";
    int32_t ret = TFS_SUCCESS;

    block_id = 8;

    // 1.加载索引文件
    IndexHandle *index_handle = new IndexHandle(".", block_id);
    if (DEBUG)
    {
        fprintf(stdout, "load index file...\n");
    }
    ret = index_handle->load(block_id, bucket_size, mmap_option);
    ASSERT_FALSE(ret != TFS_SUCCESS) << "load index " << block_id << " failed. err:" << ret << " \n";

    // 2.读取文件的metaInfo
    uint64_t file_id = 1;

    MetaInfo meta;
    ret = index_handle->readSegmentMeta(file_id, meta);
    ASSERT_FALSE(ret != TFS_SUCCESS);

    // 3.根据metaInfo读取文件
    // 2.将文件写入到主块文件
    std::stringstream tmp_streams;
    tmp_streams << "." << MAINBLOCK_DIR_PREFIX << block_id;
    tmp_streams >> mainblock_path;
    // 创建主块操作对象
    FileOperation *mainblock = new FileOperation(mainblock_path, O_RDWR | O_LARGEFILE);
    char *buffer = new char(meta.getSize() + 1);
    ret = mainblock->preadFile(buffer, meta.getSize(), meta.getOffset());
    ASSERT_FALSE(ret != TFS_SUCCESS);

    // 如果成功了就把内容进行打印
    buffer[meta.getSize()] = '\0';
    fprintf(stdout, "read size %d\n,%s\n", meta.getSize(), buffer);

    mainblock->closeFile();
    delete mainblock;
    delete index_handle;
}