//这个文件包含BufferManager的替换策略，实现时必须继承_BasePolicy
//注意: 另外必须实现 bool operator <(const &);
using ull = unsigned long long;
class _BasePolicy{
    template <typename T>
    friend class __Block;
    // 在基类的原因: 通过一个global counter，Policy就能利用虚函数access确定访问序列
    static ull global_counter;

protected:
    static ull increase_access_times() {
        return global_counter++;
    }
    bool _isDirty;
public:
    enum AccessType
    {
        READ,
        WRITE
    };
    virtual void access(AccessType accessType) = 0;

};

ull _BasePolicy::global_counter = 0;

class LRUPolicy : public _BasePolicy
{
private:
    ull LRU_cnt;
public:
    LRUPolicy() {
        LRU_cnt = _BasePolicy::increase_access_times();
    }
    void access(AccessType accessType) {
        if(accessType == WRITE)
            _isDirty = true;
        // 多次重复访问改变LRU_cnt， 但不改变访问顺序
        LRU_cnt = _BasePolicy::increase_access_times();
    }

    bool operator<(const LRUPolicy& r) const {
        return LRU_cnt < r.LRU_cnt;
    }
};
