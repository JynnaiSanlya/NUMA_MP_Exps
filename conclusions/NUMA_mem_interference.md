`parameters:$MEM_ALLOC_WAY(0 for node0, 1 for node1, others for interleaved)`

- 内核调度器自己会把线程调度到比较合适的核心上。
> 经过实验验证，设置矩阵大小为8192*8192，大约为512M。如果不刻意指定CPU，无论全部分布在node 0、node 1还是交叉分布，运行时间差别并不大。线程为6个，均为128.8秒左右，这说明内核是会做基本的移动的，事实也确实如此。

- interleaved并不差
> interleave是指把申请到的内存均匀分散到两个NUMA节点中。可以用taskset -c 0,1,2,3,4,5 ./NUMA_mem_interference <分布方式>使得进程只能在这6个CPU核心，也就是NUMA node1上运行。在这一前提下实测：
> | mem storage | node 0 | interleaved | node 1 |
> | - | - | - | - |
> | time | 123.315 | 129.895 | 199.556 |
> 
> 显然如果要访问的内存全部在另一个NUMA node上，会久很多。但是interleave并没有想象中那么差，至少显然比两者取平均好得多（一半放在node0，一半放在node1）。主要原因推测有两个：一是本地内存的带宽有限，能够放一些在另一个node，虽然那边确实慢，但是好歹多了一条路，变相增加了带宽；二是能够一定程度上缓解内存争用，也即memory interference。