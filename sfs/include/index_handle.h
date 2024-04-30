#pragma once

#include "common.h"
#include "mmap_file_op.h"

#include <memory>
#include <sys/stat.h>

namespace sfs
{
    // 索引处理
    // 索引头部类
    struct IndexHeader
    {
    public:
        IndexHeader()
        {
            memset(this, 0, sizeof(IndexHeader));
        }
        BlockInfo block_info_;     // 块信息
        int32_t bucket_size_;      // hash桶的大小
        int32_t data_file_offset_; // 下个写的数据在主块中的偏移
        int32_t index_file_size_;  // 索引文件当前偏移
        int32_t free_head_offset_; // 可重用节点链表的头部
    };

    class IndexHandle
    {
    public:
        IndexHandle(const std::string &base_path, const uint32_t main_block);
        ~IndexHandle();

        /*------索引操作------*/
        // 创建索引文件
        int create(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option);
        int load(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option);
        // 删除index文件：ummap以及unlink index file
        int remove(const uint32_t logic_block_id);
        // 进行一些同步操作
        int flush();
        // 获取索引头部
        IndexHeader *indexHeader();

        /*------块操作------*/
        // 获取块信息
        BlockInfo *blockInfo();
        // 更新块信息
        int updateBlockInfo(const OperType oper_type, const uint32_t modify_size);

        int32_t getBlockDataOffset();
        // 改变偏移
        void commitBlockDataOffset(const int file_size);

        /*------bucket相关------*/
        // 获取桶大小
        int32_t bucketSize();
        // 返回桶数组的首节点
        int32_t *bucketSlot();

        /*------hash相关------*/
        int hashFind(const uint64_t key, int32_t &current_offset, int32_t &previous_offset);
        int32_t hashInsert(const uint32_t key, int32_t &previous_offset, MetaInfo &meta);

        /*------metaInfo相关------*/
        // 写metaInfo，传入key值以及meta
        int32_t writeSegmentMeta(const uint64_t key, MetaInfo &meta);
        // 读metaInfo
        int32_t readSegmentMeta(const uint64_t key, MetaInfo &meta);
        // 删除metaInfo
        int32_t deleteSegmentMeta(const uint64_t key);
        int32_t getFreeMetaOffset();

    private:
        bool hashCompare_(const uint64_t left_key, const uint64_t right_key);

    private:
        std::unique_ptr<MMapFileOperation> file_op_ptr_;
        bool is_load_; // 索引文件是否被加载
    };

}