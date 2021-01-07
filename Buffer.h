#include <string>
#include <fstream>

#include "BasicConfig.h"
#include "ReplacementPolicy.h"
class BufferManager;
using byte = char;

template <typename _RepPolicy>
class __Block
{
#ifdef MOD
#undef MOD
#endif
#define MOD "BLOCK"
    friend class BufferManager;
    using AccessType = typename _RepPolicy::AccessType;

public:
    static const size_t block_size = BLOCK_SIZE;
    bool _isPinned;
    // 区分高层对Buffer的访问是读还是写过于困难，直接使用read(),write()方法提供缓冲区的裸指针
    // 高层不能保存这个指针，每次对缓冲区访问都需要调用下面这两个接口
    const byte *read()
    {
        _policy.access(AccessType::READ);
        return _rawData;
    }
    byte *write()
    {
        _policy.access(AccessType::WRITE);
        return _rawData;
    }
    ~__Block()
    {
        if (_isPinned)
            LOG(MOD, "Try to replace pinned page");
        this->__write();
    }
    bool operator<(const __Block &r) const
    {
        // l pin: l > r
        // l not pin:
        //    r pin: l < r
        //    r not pin: l._policy < r._policy
        return !this->_isPinned && (r._isPinned || this->_policy < r._policy);
    }

    friend std::ostream &operator<<(std::ostream &os, const __Block &c) {
        std::cout << "remove " << c._filePath << " " << c._offset / BLOCK_SIZE << std::endl;
        return os;
    }

private:
    // 基本属性
    byte *_rawData;
    std::string _filePath;
    size_t _offset;

    _RepPolicy _policy;
    // 从指定文件读取一个block, 这个block可能不存在
    // 但文件必须存在
    __Block(const std::string &filePath, size_t offset) : _isPinned(false)
    {
        this->_offset = offset;
        this->_filePath = filePath;
        _rawData = new byte[block_size]{0};

        std::ifstream is(filePath, std::ios::in | std::ios::binary);
        if (!is)
        {
            //create empty file
            std::ofstream os(this->_filePath);
            os.close();
            is.open(filePath, std::ios::in | std::ios::binary);
        }

        size_t begin = is.tellg();
        is.seekg(0, std::ios::end);
        size_t fileLen = is.tellg() - begin;

        if (fileLen > offset)
        {
            is.seekg(offset);
            is.readsome(_rawData, block_size);
        }
        _policy._isDirty = fileLen <= offset;
        is.close();
    }
    // 根据_filePath, _offset将数据块写入指定文件
    void __write()
    {
        if (!_policy._isDirty)
            return;
        std::ofstream os(this->_filePath, std::ios::in | std::ios::binary);
        if (!os)
            LOG(MOD, "Unknown Failure");
        os.seekp(_offset);
        os.write(_rawData, block_size);
        os.close();
        _policy._isDirty = false;
    }

    // 禁用移动构造函数与拷贝构造函数
    __Block(const __Block &) = delete;
    __Block(__Block &&) = delete;
};

using Block = __Block<LRUPolicy>;

#include <array>

class BufferManager
{
#ifdef MOD
#undef MOD
#define MOD "BufferManager"
#endif
private:
    std::array<Block *, N_BLOCKS> buffer;

    // 当前Buffer中的块数
    int size;
    void dequeue_noexcept(int index) noexcept
    {
        if (index < 0 || index >= size)
            return;
#ifdef _DEBUG_
        LOG(MOD, *buffer[index]);
#endif
        // remove block
        delete buffer[index];
        buffer[index] = buffer[size - 1];
        buffer[--size] = nullptr;
    }

    // 必须保证size < N_BLOCKS
    void enqueue(Block *block_ptr)
    {
        // 缓冲区满, 分配失败
        if (size >= N_BLOCKS)
            throw std::bad_alloc();
        buffer[size++] = block_ptr;
    }

    // 返回最易被替换的block的iterator
    auto select()
    {
        Block *min_ptr = nullptr;
        int index = -1;
        for (int i = 0; i < size; i++)
        {
            if (!min_ptr)
            {
                min_ptr = buffer[i];
                index = i;
                continue;
            }
            if (!buffer[i]->_isPinned && *buffer[i] < *min_ptr)
            {
                min_ptr = buffer[i];
                index = i;
            }
        }
        return min_ptr ? index : -1;
    }

public:
    BufferManager()
    {
        size = 0;
    }
    ~BufferManager()
    {
        for (int i = 0; i < size; i++)
            delete buffer[i];
    }
    //上层利用filePath和offset获取Block
    Block &get_block(const std::string &filePath, size_t offset)
    {
        // if exists, return directly
        for (int i = 0; i < size; i++)
        {
            if (buffer[i]->_filePath == filePath && buffer[i]->_offset == offset)
                return *buffer[i];
        }
        // if not exists, read from disk
        if (size == N_BLOCKS)
            dequeue_noexcept(select());
        Block *ptr = new Block(filePath, offset);
        enqueue(ptr);
        return *ptr;
    }
};
