# API

## Buffer Manager API
```
// 实例化BufferManager后，调用该接口获得指定的Block进行IO
Block &get_block(const std::string &filePath, size_t offset)

// 区分高层对Buffer的访问是读还是写过于困难，直接使用read(),write()方法提供缓冲区的裸指针; 高层不能保存这个指针，每次对缓冲区访问都需要调用下面这两个接口
const byte *read();
byte *write();

// 例如:
// Outer Scope
BufferManager bfMgr;
...
{
    Block& block = bfMgr.get_block();
    const my_struct * tmp_block_rdata = block.read();
    //...read...

    my_struct * tmp_block_wdata = block.write();
    //...write...
}
//Wrong: 永不保存通过两个接口获得的裸指针
data = tmp_block_rdata->data_member
...
```
