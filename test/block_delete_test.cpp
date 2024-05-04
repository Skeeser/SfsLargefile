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

TEST(sfs, block_delete)
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

    // 2.删除指定文件的metaInfo
    uint64_t file_id = 1;

    ret = index_handle->deleteSegmentMeta(file_id);
    ASSERT_FALSE(ret != TFS_SUCCESS);

    ret = index_handle->flush();
    ASSERT_FALSE(ret != TFS_SUCCESS);

    fprintf(stdout, "delete successfully!\n");
    delete index_handle;
}