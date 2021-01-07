#include "BasicConfig.h"
#undef BLOCK_SIZE
#undef N_BLOCKS
// 测试条件：块大小16B Buffer共4块
#define BLOCK_SIZE 16
#define N_BLOCKS 4

#include "Buffer.h"

using namespace std;
//Test1: 向文件中依次写入32个块，然后检查数据完整性
void test1() {
    int block_num = 32;
    string filePath = "./testFile";
    {
        BufferManager bfMgr;

        for (int i = 0; i < block_num; i++)
        {
            Block &block = bfMgr.get_block(filePath, i * BLOCK_SIZE);
            char *ptr = block.write();
            ptr[0] = ptr[BLOCK_SIZE - 1] = i;
        }
    }
    {
        BufferManager bfMgr;
        int i = 0;
        for (; i < block_num; i++)
        {
            Block &block = bfMgr.get_block(filePath, i * BLOCK_SIZE);
            const char *ptr = block.read();
            if (ptr[0] != i)
            {
                //check fail
                cout << "Test1 Error" << endl;
                break;
            }
        }
        if (i == block_num)
            cout << "Test1 Success" << endl;
    }
}

// Test 2: 边界测试，检查是否能从不能存在的空文件中读取数据
// 应当能获取全0的块
void test2() {
    int block_num = 32;
    string filePath = "./NonExistFile";
    {
        BufferManager bfMgr;
        int i = 0;
        Block &block = bfMgr.get_block(filePath, 4096 * BLOCK_SIZE);
        const char *ptr = block.read();
        const int *tmp = (const int *)ptr;
        int flag = 0;
        for (; (const char *)tmp < ptr + BLOCK_SIZE; tmp++)
            flag |= *tmp;
        if(flag)
            cout << "Test2 Error" << endl;
        else
            cout << "Test2 Success" << endl;
    }
}

// Test 3: 随机测试，用随机构造的一组多文件访问序列检查是否正确
void test3() {
    // TODO
}


int main(int argc, char** argv) {
    test1();
    test2();
    test3();
    system("pause");
    return 0;
}
