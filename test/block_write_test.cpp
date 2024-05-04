#include "gtest/gtest.h"

// 往块里写文件
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

static int32_t block_id = 1;

TEST(sfs, block_write)
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

    // 2.将文件写入到主块文件
    std::stringstream tmp_streams;
    tmp_streams << "." << MAINBLOCK_DIR_PREFIX << block_id;
    tmp_streams >> mainblock_path;

    // 创建主块操作对象
    FileOperation *mainblock = new FileOperation(mainblock_path, O_RDWR | O_CREAT | O_LARGEFILE);

    char buffer[4096];
    memset(buffer, '6', sizeof(buffer));
    // 拿到主块偏移
    int32_t data_offset = index_handle->getBlockDataOffset();
    // 拿到可分配的文件编号
    uint32_t file_no = index_handle->blockInfo()->seq_no_;

    ret = mainblock->pwriteFile(buffer, sizeof(buffer), data_offset);
    ASSERT_FALSE(ret != TFS_SUCCESS);
    // {
    //     fprintf(stderr, "write to main block failed.ret:%d,reason:%s\n", ret, strerror(errno));
    //     mainblock->close_file();
    //     delete mainblock;
    //     delete index_handle;
    //     return -3;
    // }

    // 文件已经写入成功主块
    // 3.索引文件中写入metaInfo
    MetaInfo meta;
    meta.setFileId(file_no);
    meta.setOffset(data_offset);
    meta.setSize(sizeof(buffer));
    ret = index_handle->writeSegmentMeta(meta.getKey(), meta);

    ASSERT_TRUE(ret == TFS_SUCCESS);

    // 1.更新索引头部信息
    index_handle->commitBlockDataOffset(sizeof(buffer));
    // 2.更新块信息
    index_handle->updateBlockInfo(C_OPER_INSERT, sizeof(buffer));

    ret = index_handle->flush();
    // if (ret != TFS_SUCCESS)
    // {
    //     fprintf(stderr, "flush mainblock %d failed %d failed.file_no:%u\n", block_id, file_no);
    // }

    // else
    // {
    //     fprintf(stderr, "write_segment_meta failed mainblock %d failed %d failed.file_no:%u\n", block_id, file_no);
    // }

    // if (ret == TFS_SUCCESS)
    // {
    //     if (debug)
    //         fprintf(stdout, "write succesfully.file_no:%u,block_id:%d\n", file_no, block_id);
    // }

    // 4.其他操作
    mainblock->closeFile();
    index_handle->flush();

    delete mainblock;
    delete index_handle;
}