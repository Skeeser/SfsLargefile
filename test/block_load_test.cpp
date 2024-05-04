#include "gtest/gtest.h"

// 测试块加载状态
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

TEST(sfs, block_load)
{
    string mainblock_path = "";
    string index_path = "";
    int32_t ret = TFS_SUCCESS;

    int32_t block_id = 1;

    block_id = 8;
    cout << "block_id: " << block_id << endl;

    // 加载索引文件
    std::unique_ptr<IndexHandle> index_handle(new IndexHandle(".", block_id));
    fprintf(stdout, "load index file...\n");

    ret = index_handle->load(block_id, bucket_size, mmap_option);
    ASSERT_FALSE(ret != TFS_SUCCESS) << "load index " << block_id << " failed. err:" << ret << " \n";
}