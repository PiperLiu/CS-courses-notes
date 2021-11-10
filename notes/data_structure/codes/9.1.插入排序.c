void InsertionSort(ElementType A[], int N)
{ /* 插入排序 */
    int P, i;
    ElementType Tmp;

    for (P = 1; P < N; P++)
    {
        Tmp = A[P]; /* 取出未排序序列中的第一个元素*/
        for (i = P; i > 0 && A[i - 1] > Tmp; i--)
            A[i] = A[i - 1]; /*依次与已排序序列中元素比较并右移*/
        A[i] = Tmp;          /* 放进合适的位置 */
    }
}
