`矩阵大小为4M。`

`parameters: $THREAD_NUM $THREAD_NUMA_NODE $MEM_NUMA_NODE $ALLOW_MOVE`

| parameters | 6,0,0,0 | 6,1,1,0 | 6,0,1,0 | 6,0,1,1 |
| - | - | - | - | - |
| res(s) | 15.2 | 15.3 | 48.6 | 18.3 |

- 矩阵的大小（也就是需要移动的数据量）会很显著地影响结果。在<=1M的时候后两组参数的差别并不太大。
- 跨NUMA节点迁移数据在数据量足够大的情况下能够显著改善性能。