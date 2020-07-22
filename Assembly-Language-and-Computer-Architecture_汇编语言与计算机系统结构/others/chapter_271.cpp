/* 题意理解：有字节 0 - 3，函数会抽取一个，并符号扩展为32位int
    | 字节 | 1111 1111 | 1111 1111 | 1111 1111 | 1111 1111 |
    | 编号 |     3     |     2     |     1     |     0     |

*/

/* Failed attempt at xbyte */
/*
int xbyte(packed_t word, int bytenum)
{
    return (word >> (bytenum << 3)) & 0xFF;
}

A. 这段代码错在哪里？
假设我们的 packed_t 是这样的：
    | 字节 | 0000 0100 | 1111 1011 | 0000 0000 | 0000 0000 |
    | 编号 |     3     |     2     |     1     |     0     |

按照这个错代码的逻辑，我们尝试取出第 2 个字节。

我们取出第 2 个字节的过程是没有问题的：
    `int bytenum = 2;`
    `word >> (bytenum << 3)` 这可以取出 `0000 0100 1111 1011`
    接着与 0xFF 即 1111 1111 做 & 运算，则取出了 `1111 1011`
    如果直接 return ，则高位自动补 0 ，这不对，不是“符号扩展”
    错误代码的得到的结果为：
        `0000 0000 0000 0000 0000 0000 1111 1011`
    但我们希望得到的正确结果为：
        `1111 1111 1111 1111 1111 1111 1111 1011`
    因此，“前任”错在没有进行符号扩展，只做了无符号扩展。

我的思路是：
    注意到目标与其更高位的字节： `0000 0100 1111 1011`
    向左移 3 - bytenum 位，以去除 `0000 0100` 这个干扰项
    以得到 `1111 1011 xxxx xxxx xxxx xxxx xxxx xxxx`
    x 是什么不重要，一会儿就会被去除
    接着，将其转换为有符号数 int
    再向右移 3 位，以获得符号扩展
    `yyyy yyyy yyyy yyyy yyyy yyyy 1111 1011`
    y 由 `1111 1011` 的最左高位决定（因为是有符号数）

B. 给出函数的正确实现
*/
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace std;

typedef unsigned packed_t;

// 这个是“前任”的代码
int wrong_xbyte(packed_t word, int bytenum)
{
    return (word >> (bytenum << 3)) & 0xFF;
}

// 这个是我的解答
int xbyte(packed_t word, int bytenum)
{   
    int tmp = int(word << ((3 - bytenum) << 3));
    return tmp >> (3 << 3);
}

// just for showing result
void print_num(int num, int format = 10);

int main(int argc, char const *argv[])
{
    packed_t example = 0b00000100111110110000000000000000;

    printf("We input ");
    print_num(example, 2);

    printf("We want 2nd byte, \n");
    
    printf("The ex's result is ");
    int wrong_answer = wrong_xbyte(example, 2);
    print_num(wrong_answer, 2);

    printf("The right one should be ");
    int right_answer = xbyte(example, 2);
    print_num(right_answer, 2);
}

void print_num(int num, int format)
{
    if (format == 2)
    {
        char trans[40];
        itoa(num, trans, 2);
        printf("bin %032s \n\n", trans);
    }
    else if (format == 8)
    {
        printf("oct %011o \n\n", num);
    }
    else if (format == 10)
    {
        printf("hex %010d \n\n", num);
    }
    else if (format == 16)
    {
        printf("dec %08x \n\n", num);
    }
    else
    {
        printf("format must be 2, 8, 10 or 16!");
    }
}
