#include "index_handle.h"

namespace sfs
{
    IndexHandle::IndexHandle(const std::string &base_path, const uint32_t main_block)
    {

        std::stringstream tmp_stream;
        // 合成最终路径
        tmp_stream << base_path << INDEX_DIR_PREFIX << main_block;

        std::string index_path;
        tmp_stream >> index_path;

        file_op_ptr_ = std::make_unique<MMapFileOperation>(index_path, O_CREAT | O_RDWR | O_LARGEFILE);
    }
    IndexHandle::~IndexHandle()
    {
    }

    /*------索引操作------*/
    // 创建索引文件
    int IndexHandle::create(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option)
    {
        int ret = TFS_SUCCESS;
        if (DEBUG)
        {
            printf("index create block: %u index. bucket size: %d, max mmap size: %d, first mmap size: %d, per mmap size: %d\n",
                   logic_block_id, bucket_size, map_option.max_mmap_size_, map_option.first_mmap_size_,
                   map_option.per_mmap_size_);
        }

        if (is_load_)
        {
            return EXIT_INDEX_ALREADY_LOADED_ERROR;
        }

        // 获取索引文件的大小
        int64_t file_size = file_op_ptr_->getFileSize();

        if (file_size < 0)
        {
            if (DEBUG)
                printf("index file size is less than 0, error\n");
            return TFS_ERROR;
        }
        // 空文件，说明还没有创建具体的信息
        else if (file_size == 0)
        {
            // 索引文件头部信息

            IndexHeader i_header;
            i_header.block_info_.block_id_ = logic_block_id;
            i_header.block_info_.seq_no_ = 1;
            i_header.bucket_size_ = bucket_size;
            i_header.index_file_size_ = sizeof(i_header) + bucket_size * sizeof(int32_t);

            // index header + total buckets
            // 进行分配空间并初始化
            char *init_data = new char[i_header.index_file_size_];
            memcpy(init_data, &i_header, sizeof(IndexHeader));
            memset(init_data + sizeof(IndexHeader), 0, i_header.index_file_size_ - sizeof(IndexHeader));

            // std::cout << init_data << " \nsize: " << i_header.index_file_size_ << std::endl;

            // 将索引文件头部和哈希桶的数据写入到index文件中
            ret = file_op_ptr_->pwriteFile(init_data, i_header.index_file_size_, 0);
            delete[] init_data;
            if (ret != TFS_SUCCESS)
            {
                if (DEBUG)
                    printf("pwrite file, error\n");
                return ret;
            }

            // 同步到磁盘
            ret = file_op_ptr_->flushFile();
            if (ret != TFS_SUCCESS)
            {
                if (DEBUG)
                    printf("flush file, error\n");
                return ret;
            }
        }
        else
        {
            // 索引文件已经存在
            return EXIT_META_UNEXPECT_FOUND_ERROR;
        }

        ret = file_op_ptr_->mmapFile(map_option);
        if (ret != TFS_SUCCESS)
        {
            if (DEBUG)
                printf("init block mmap file, error\n");
            return ret;
        }

        is_load_ = true;

        if (DEBUG)
        {
            printf("init blockid: %u index successful. data file size: %d, index file size: %d, bucket size: %d, free head offset: %d, seqno: %d, size: %d, filecount: %d, del_size: %d, del_file_count: %d version: %d\n",
                   logic_block_id, indexHeader()->data_file_offset_, indexHeader()->index_file_size_,
                   indexHeader()->bucket_size_, indexHeader()->free_head_offset_, blockInfo()->seq_no_, blockInfo()->size_t_,
                   blockInfo()->file_count_, blockInfo()->del_size_, blockInfo()->del_file_count_, blockInfo()->version_);
        }
        return TFS_SUCCESS;
    }

    int IndexHandle::load(const uint32_t logic_block_id, const int32_t bucket_size, const MMapOption map_option)
    {
        int ret = TFS_SUCCESS;
        // 如果已经加载了
        if (is_load_)
        {
            return EXIT_INDEX_ALREADY_LOADED_ERROR;
        }

        // 获取索引文件的大小，顺便看有没有
        int64_t file_size = file_op_ptr_->getFileSize();
        if (file_size < 0)
        {
            return file_size;
        }
        else if (file_size == 0) // 空文件
        {
            return EXIT_INDEX_CORRUPT_ERROR;
        }

        MMapOption tmp_map_option = map_option;
        if (file_size > tmp_map_option.first_mmap_size_ && file_size <= tmp_map_option.max_mmap_size_)
        {
            // 将第一次映射文件的大小改为文件大小
            tmp_map_option.first_mmap_size_ = file_size;
        }
        // 映射
        ret = file_op_ptr_->mmapFile(tmp_map_option);

        if (ret != TFS_SUCCESS)
        {
            return ret;
        }

        if (this->bucketSize() == 0 || this->blockInfo()->block_id_ == 0)
        {
            fprintf(stderr, "Index corrupt error. blockid: %u, bucket size: %d\n", blockInfo()->block_id_,
                    this->bucketSize());
            return EXIT_INDEX_CORRUPT_ERROR;
        }

        // 检查文件大小
        int32_t index_file_size = sizeof(IndexHeader) + this->bucketSize() * sizeof(int32_t);

        if (file_size < index_file_size)
        {
            fprintf(stderr, "Index corrupt error. blockid: %u, bucket size: %d, file size: %ld, index file size: %d\n",
                    blockInfo()->block_id_, this->bucketSize(), file_size, index_file_size);
            return EXIT_INDEX_CORRUPT_ERROR;
        }

        // 比较块id
        if (logic_block_id != blockInfo()->block_id_)
        {
            fprintf(stderr, "block id conflict. blockid: %u, index blockid: %u\n", logic_block_id, blockInfo()->block_id_);
            return EXIT_BLOCKID_CONFLICT_ERROR;
        }

        if (bucket_size != this->bucketSize())
        {
            fprintf(stderr, "Index configure error. old bucket size: %d, new bucket size: %d\n", this->bucketSize(),
                    bucket_size);
            return EXIT_BUCKET_CONFIGURE_ERROR;
        }
        is_load_ = true;
        if (DEBUG)
            printf("load blockid: %u index successful. data file offset: %d, index file size: %d, bucket size: %d, free head offset: %d, seqno: %d, size: %d, filecount: %d, del size: %d, del file count: %d version: %d\n",
                   logic_block_id, indexHeader()->data_file_offset_, indexHeader()->index_file_size_, this->bucketSize(),
                   indexHeader()->free_head_offset_, blockInfo()->seq_no_, blockInfo()->size_t_, blockInfo()->file_count_,
                   blockInfo()->del_size_, blockInfo()->del_file_count_, blockInfo()->version_);
        return TFS_SUCCESS;
    }
    // 删除index文件：ummap以及unlink index file
    int IndexHandle::remove(const uint32_t logic_block_id)
    {
        // 看是否已经加载映射了
        if (is_load_)
        {
            if (logic_block_id != blockInfo()->block_id_)
            {
                fprintf(stderr, "block id conflict.block id:%d index block id:%d\n", logic_block_id, blockInfo()->block_id_);
                return EXIT_BLOCKID_CONFLICT_ERROR;
            }
        }

        int ret = file_op_ptr_->munmapFile();
        if (ret != TFS_SUCCESS)
        {
            return ret;
        }
        // 删除文件
        ret = file_op_ptr_->unlinkFile();
        return ret;
    }

    // 进行一些同步操作
    int IndexHandle::flush()
    {
        int ret = file_op_ptr_->flushFile();
        if (ret != TFS_SUCCESS)
        {
            fprintf(stderr, "index flush failed!,reason:%s", strerror(errno));
        }
        return ret;
    }

    // 获取索引头部
    IndexHeader *IndexHandle::indexHeader()
    {
        // 返回映射内存的首地址
        return reinterpret_cast<IndexHeader *>(file_op_ptr_->getMapDataAddr());
    }

    /*------块操作------*/
    // 获取块信息
    BlockInfo *IndexHandle::blockInfo()
    {
        // 返回映射内存的首地址
        return reinterpret_cast<BlockInfo *>(file_op_ptr_->getMapDataAddr());
    }

    // 更新块信息
    int IndexHandle::updateBlockInfo(const OperType oper_type, const uint32_t modify_size)
    {
        // 块ID不能为0
        if (blockInfo()->block_id_ == 0)
        {
            return EXIT_BLOCKID_ZERO_ERROR;
        }

        if (oper_type == C_OPER_INSERT)
        {
            ++blockInfo()->version_;
            ++blockInfo()->file_count_;
            ++blockInfo()->seq_no_;
            blockInfo()->size_t_ += modify_size;
        }
        else if (oper_type == C_OPER_DELETE)
        {
            ++blockInfo()->version_;
            --blockInfo()->file_count_;
            blockInfo()->size_t_ -= modify_size;
            ++blockInfo()->del_file_count_;
            blockInfo()->del_size_ += modify_size;
        }
        if (DEBUG)
            printf("update block info. blockid: %u, version: %u, file count: %u, size: %u, del file count: %u, del size: %u, seq no: %u, oper type: %d\n",
                   blockInfo()->block_id_, blockInfo()->version_, blockInfo()->file_count_, blockInfo()->size_t_,
                   blockInfo()->del_file_count_, blockInfo()->del_size_, blockInfo()->seq_no_, oper_type);
        return TFS_SUCCESS;
    }

    int32_t IndexHandle::getBlockDataOffset()
    {
        return indexHeader()->data_file_offset_;
    }
    // 改变偏移
    void IndexHandle::commitBlockDataOffset(const int file_size)
    {
        indexHeader()->data_file_offset_ += file_size;
    }

    /*------bucket相关------*/
    // 获取桶大小
    int32_t IndexHandle::bucketSize()
    {
        return indexHeader()->bucket_size_;
    }
    // 返回桶数组的首节点
    int32_t *IndexHandle::bucketSlot()
    {
        return reinterpret_cast<int32_t *>(reinterpret_cast<char *>(file_op_ptr_->getMapDataAddr()) + sizeof(IndexHeader));
    }

    /*------hash相关------*/
    int IndexHandle::hashFind(const uint64_t key, int32_t &current_offset, int32_t &previous_offset)
    {
        int ret = TFS_SUCCESS;
        MetaInfo meta_info;
        current_offset = 0;
        previous_offset = 0;

        // 确定key存放的桶slot的位置
        int32_t slot = static_cast<int32_t>(key) % bucketSize();
        // 读取桶首节点存储的第一个节点的偏移量（如果偏移量为0，即该节点还没开始存文件）
        int32_t pos = bucketSlot()[slot];
        // 根据偏移量读取存储的metaInfo
        while (pos)
        {
            ret = file_op_ptr_->preadFile(reinterpret_cast<char *>(&meta_info), sizeof(MetaInfo), pos);
            if (ret != TFS_SUCCESS)
            {
                return ret;
            }

            if (hashCompare_(key, meta_info.getKey()))
            {
                current_offset = pos;
                return TFS_SUCCESS;
            }
            // 如果不是对应的key, 顺着往下找
            previous_offset = pos;
            pos = meta_info.getNextMetaOffset();
        }

        // 那么就是没找到，说明该key还不存在
        return EXIT_META_NOT_FOUND_ERROR;
    }
    int32_t IndexHandle::hashInsert(const uint32_t key, int32_t &previous_offset, MetaInfo &meta)
    {
        int ret = TFS_SUCCESS;
        int32_t current_offset;
        MetaInfo tmp_meta_info;

        // 确定key存放的桶slot的位置
        int32_t slot = static_cast<int32_t>(key) % bucketSize();

        // 将meta下一个节点的偏移量置零
        meta.setNextMetaOffset(0);

        // 确定metaInfo存储在文件中的偏移量
        // 考虑有可重用节点的情况
        int32_t free_meta_offset = this->getFreeMetaOffset();
        if (free_meta_offset != 0)
        {
            // 将可重用链表的第一个节点进行读取，保存至tmp中
            ret = file_op_ptr_->preadFile(reinterpret_cast<char *>(&tmp_meta_info), sizeof(MetaInfo), free_meta_offset);
            if (ret != TFS_SUCCESS)
            {
                return ret;
            }

            current_offset = free_meta_offset;

            // 将metaInfo写入索引文件中
            ret = file_op_ptr_->pwriteFile(reinterpret_cast<const char *>(&meta), sizeof(MetaInfo), current_offset);
            if (ret == TFS_SUCCESS)
            {
                // 调整可重用节点的偏移
                indexHeader()->free_head_offset_ = tmp_meta_info.getNextMetaOffset();
            }
            else
                return ret;

            if (DEBUG)
            {
                fprintf(stdout, "reuse metainfo,current_offset:%d\n", current_offset);
            }
        }
        else
        {
            // 如果没有空闲节点，那么就在尾部重新创建一个meta
            current_offset = indexHeader()->index_file_size_;
            // 将metaInfo写入索引文件中
            ret = file_op_ptr_->pwriteFile(reinterpret_cast<const char *>(&meta), sizeof(MetaInfo), current_offset);
            if (ret == TFS_SUCCESS)
                indexHeader()->index_file_size_ += sizeof(MetaInfo);
            else
                return ret;
        }

        // 将meta节点位置插入到哈希链表中
        // 前个节点已经存在
        if (previous_offset != 0)
        {
            // 读出前个节点
            ret = file_op_ptr_->preadFile(reinterpret_cast<char *>(&tmp_meta_info), sizeof(MetaInfo), previous_offset);
            // todo: 下面可能有问题, 如果是可重用的节点会造成不可重用了
            if (ret != TFS_SUCCESS)
            {
                indexHeader()->index_file_size_ -= sizeof(MetaInfo);
                return ret;
            }
            tmp_meta_info.setNextMetaOffset(current_offset);
            // 更新previous节点，将数据写回去
            ret = file_op_ptr_->pwriteFile(reinterpret_cast<char *>(&tmp_meta_info), sizeof(MetaInfo), previous_offset);
            if (ret != TFS_SUCCESS)
            {
                indexHeader()->index_file_size_ -= sizeof(MetaInfo);
                return ret;
            }
        }
        // 如果是该hash桶的首个节点
        else
        {
            bucketSlot()[slot] = current_offset;
        }
        return TFS_SUCCESS;
    }

    bool IndexHandle::hashCompare_(const uint64_t left_key, const uint64_t right_key)
    {
        return left_key == right_key;
    }

    /*------metaInfo相关------*/
    // 写metaInfo，传入key值以及meta
    int32_t IndexHandle::writeSegmentMeta(const uint64_t key, MetaInfo &meta)
    {
        int32_t current_offset = 0;
        int32_t previous_offset = 0;

        // 检测key是否已经存在, 并获取cur和pre的offset
        int ret = hashFind(key, current_offset, previous_offset);
        // 说明找到了
        if (ret == TFS_SUCCESS)
        {
            return EXIT_META_UNEXPECT_FOUND_ERROR;
        }
        // 如果不是没找到那就就是报错
        else if (ret != EXIT_META_NOT_FOUND_ERROR)
        {
            return ret;
        }
        // 如果不存在就将metaInfo写入到哈希表中hash_insert(meta,slot,previous_offset)
        ret = hashInsert(key, previous_offset, meta);

        // 更新块的信息
        updateBlockInfo(C_OPER_INSERT, meta.getSize());
        return ret;
    }

    // 读metaInfo
    int32_t IndexHandle::readSegmentMeta(const uint64_t key, MetaInfo &meta)
    {
        int32_t current_offset = 0;
        int32_t previous_offset = 0;

        int32_t ret = hashFind(key, current_offset, previous_offset);
        // 说明存在key
        if (ret == TFS_SUCCESS)
        {
            ret = file_op_ptr_->preadFile(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), current_offset);
            return ret;
        }
        return ret;
    }

    // 删除metaInfo
    int32_t IndexHandle::deleteSegmentMeta(const uint64_t key)
    {
        // 初始化要删除节点的偏移值以及上一个节点的偏移值
        int32_t current_offset = 0;
        int32_t previous_offset = 0;
        // 找到要删除节点的当前offset以及上个节点的offset
        int32_t ret = hashFind(key, current_offset, previous_offset);

        if (ret != TFS_SUCCESS)
        {
            return ret;
        }
        MetaInfo meta;
        MetaInfo previous_meta;
        // 读取要删除节点的信息
        ret = file_op_ptr_->preadFile(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), current_offset);
        if (ret != TFS_SUCCESS)
        {
            return ret;
        }

        int32_t next_pos = meta.getNextMetaOffset();
        // 如果要删除的节点位于桶的“开头”
        if (previous_offset == 0)
        {
            int32_t slot = static_cast<uint32_t>(key) % bucketSize();
            bucketSlot()[slot] = next_pos;
        }
        else
        {
            // 读取上一个meta的信息
            ret = file_op_ptr_->preadFile(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), previous_offset);
            if (ret != TFS_SUCCESS)
            {
                return ret;
            }
            // 将previous_meta节点重新写回去
            previous_meta.setNextMetaOffset(next_pos);
            ret = file_op_ptr_->pwriteFile(reinterpret_cast<char *>(&meta), sizeof(MetaInfo), previous_offset);
            if (ret != TFS_SUCCESS)
            {
                return ret;
            }

            // 将索引的可重用节点的偏移进行更新
            indexHeader()->free_head_offset_ = current_offset;
            if (DEBUG)
            {
                fprintf(stdout, "delete_segment_meta delete metainfo,current_offset:%d\n", current_offset);
            }
            // 更新块的信息
            updateBlockInfo(C_OPER_DELETE, meta.getSize());
            return TFS_SUCCESS;
        }
        return ret;
    }

    int32_t IndexHandle::getFreeMetaOffset()
    {
        return indexHeader()->free_head_offset_;
    }

}