## Caching Strategy

本项目作为C++的练手项目，利用模板和C++11和C++17的部分特性 对LRU，LFU，ARC这三种经典的缓存算法做了简易的实现，也对LRU和LFU的优化算法做了一些简易实现，如LRU-k，HashLRU，AgingLFU

由于本项目没有经历过实际应用环境的检验，其内部可能还存在一些问题，同时性能方面还有待提高，欢迎大家给出意见

以下是在缓存空间大小为100，模拟数据量在10000条件下的测试情况

![image-20250225193116337](https://cdn.jsdelivr.net/gh/MengChangWang/Blog_Image@main/img/image-20250225193116337.png)