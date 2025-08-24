#include "ds_elementTypeInit.h"

typedef struct GNode *GList;
struct GNode
{
    int Tag; // 标志域：0表示结点是单元素，1表示结点是广义表
    union    // 子表指针域与单元素数据域Data复用，即共用存储空间
    {
        ElementType Data;
        GList SubList;
    } URegion;
    GList Next; // 指向后继结点
};
