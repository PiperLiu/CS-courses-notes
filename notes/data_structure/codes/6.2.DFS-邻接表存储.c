/* 邻接表存储的图 - DFS */

void Visit(Vertex V)
{
    printf("正在访问顶点%d\n", V);
}

/* Visited[]为全局变量，已经初始化为false */
void DFS(LGraph Graph, Vertex V, void (*Visit)(Vertex))
{ /* 以V为出发点对邻接表存储的图Graph进行DFS搜索 */
    PtrToAdjVNode W;

    Visit(V);          /* 访问第V个顶点 */
    Visited[V] = true; /* 标记V已访问 */

    for (W = Graph->G[V].FirstEdge; W; W = W->Next) /* 对V的每个邻接点W->AdjV */
        if (!Visited[W->AdjV])                      /* 若W->AdjV未被访问 */
            DFS(Graph, W->AdjV, Visit);             /* 则递归访问之 */
}