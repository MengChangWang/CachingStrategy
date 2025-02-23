#pragma once

//#define TEST

#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <random>
#include <algorithm>
#include <vector>
#include <array>

#include "UseTemplate\ICachePolicy.h"
#include "UseTemplate\LRU\LruCache.h"
#include "UseTemplate\LRU\LruKCache.h"
#include "UseTemplate\LRU\SliceLruCache.h"
#include "UseTemplate\LFU\LfuCache.h"
#include "UseTemplate\LFU\AgingLfuCache.h"

#include "UseTemplate\ARC\ArcLru.h"
#include "UseTemplate\ARC\ArcLfu.h"


class Timer {
public:
    Timer();
    double elapsed();

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// 辅助函数：打印结果
void printResults(const std::string& testName,
    const std::vector<int>& get_operations,
    const std::vector<int>& hits);

template<typename Key, typename Value>
void testHotDataAccess(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations);

template<typename Key, typename Value>
void testLoopPattern(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations);

template<typename Key, typename Value>
void testWorkloadShift(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations);

void test();

// Implementation

void test() {
#ifdef TEST
    const unsigned int CAPACITY = 10;
#else
    const unsigned int CAPACITY = 100;
#endif
    LruCache<int, std::string> lru(CAPACITY);
    LruKCache<int, std::string> lru_k(CAPACITY, CAPACITY, 2);
    SliceLruCache<int, std::string> slice_lru(CAPACITY / 10, CAPACITY);
    LfuCache<int, std::string> lfu(CAPACITY);
    AgingLfuCache<int, std::string> aging_lfu(CAPACITY);

    ArcLru<int, string> arc_lru(CAPACITY);
    ArcLru<int, string> arc_lfu(CAPACITY);

    std::vector<ICachePolicy<int, std::string>*> caches = { &lru,&lru_k,&arc_lfu,&lfu,&lru};
    std::vector<int> hits(6, 0);
    std::vector<int> get_operations(6, 0);

    testHotDataAccess(caches, hits, get_operations);
    testLoopPattern(caches, hits, get_operations);
    testWorkloadShift(caches, hits, get_operations);
}


Timer::Timer() : start_(std::chrono::high_resolution_clock::now()) {}

double Timer::elapsed() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
}

void printResults(const std::string& testName,
    const std::vector<int>& get_operations,
    const std::vector<int>& hits) {
    size_t n = hits.size();
    size_t i = 0;
    //std::cout << "缓存大小: " << capacity << std::endl;
    std::cout << "LRU - 命中率: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "LRU-K - 命中率：" << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "Slice_LRU - 命中率：" << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "LFU - 命中率: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "Aging_LFU - 命中率: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "ARC - 命中率: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
}

template<typename Key, typename Value>
void testHotDataAccess(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations) {
    std::cout << "\n=== 测试场景1：热点数据访问测试 ===" << std::endl;

#ifdef TEST
    const int OPERATIONS = 10;
    const int HOT_KEYS = 3;
    const int COLD_KEYS = 50;
#else
    const int OPERATIONS = 10000;
    const int HOT_KEYS = 3;
    const int COLD_KEYS = 500;
#endif // DEBUG

    std::random_device rd;
    std::mt19937 gen(rd());

    // 先进行一系列put操作
    for (int i = 0; i < caches.size(); ++i) {
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            if (op % 100 < 40) {  // 40%热点数据
                key = gen() % HOT_KEYS;
            }
            else {  // 60%冷数据
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }
            std::string value = "value" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // 然后进行随机get操作
        for (int get_op = 0; get_op < OPERATIONS / 2; ++get_op) {
            int key;
            if (get_op % 100 < 40) {  // 40%概率访问热点
                key = gen() % HOT_KEYS;
            }
            else {  // 60%概率访问冷数据
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }

            get_operations[i]++;
            if (caches[i]->isExists(key)) {
                hits[i]++;
            }
        
        }
    }

    printResults("热点数据访问测试", get_operations, hits);
}

template<typename Key, typename Value>
void testLoopPattern(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations) {
    std::cout << "\n=== 测试场景2：循环扫描测试 ===" << std::endl;

    const int LOOP_SIZE = 200;
    const int OPERATIONS = 10000;

    std::random_device rd;
    std::mt19937 gen(rd());

    // 先填充数据
    for (int i = 0; i < caches.size(); ++i) {
        for (int key = 0; key < LOOP_SIZE * 2; ++key) {
            std::string value = "loop" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // 然后进行访问测试
        int current_pos = 0;
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            if (op % 100 < 70) {  // 70%顺序扫描
                key = current_pos;
                current_pos = (current_pos + 1) % LOOP_SIZE;
            }
            else if (op % 100 < 85) {  // 15%随机跳跃
                key = gen() % LOOP_SIZE;
            }
            else {  // 15%访问范围外数据
                key = LOOP_SIZE + (gen() % LOOP_SIZE);
            }

            get_operations[i]++;
            if (caches[i]->isExists(key)) {
                hits[i]++;
            }
         
        }
    }

    printResults("循环扫描测试", get_operations, hits);
}

template<typename Key, typename Value>
void testWorkloadShift(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations) {
    std::cout << "\n=== 测试场景3：工作负载剧烈变化测试 ===" << std::endl;

    const int OPERATIONS = 10000;
    const int PHASE_LENGTH = OPERATIONS / 5;

    std::random_device rd;
    std::mt19937 gen(rd());

    // 先填充一些初始数据
    for (int i = 0; i < caches.size(); ++i) {
        for (int key = 0; key < 1000; ++key) {
            std::string value = "init" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // 然后进行多阶段测试
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            // 根据不同阶段选择不同的访问模式
            if (op < PHASE_LENGTH) {  // 热点访问
                key = gen() % 5;
            }
            else if (op < PHASE_LENGTH * 2) {  // 大范围随机
                key = gen() % 1000;
            }
            else if (op < PHASE_LENGTH * 3) {  // 顺序扫描
                key = (op - PHASE_LENGTH * 2) % 100;
            }
            else if (op < PHASE_LENGTH * 4) {  // 局部性随机
                int locality = (op / 1000) % 10;
                key = locality * 20 + (gen() % 20);
            }
            else {  // 混合访问
                int r = gen() % 100;
                if (r < 30) {
                    key = gen() % 5;
                }
                else if (r < 60) {
                    key = 5 + (gen() % 95);
                }
                else {
                    key = 100 + (gen() % 900);
                }
            }

            get_operations[i]++;
            if (caches[i]->isExists(key)) {
                hits[i]++;
            }

            // 随机进行put操作，更新缓存内容
            if (gen() % 100 < 30) {  // 30%概率进行put
                std::string value = "new" + std::to_string(key);
                caches[i]->put(key, value);
            }
        }
    }

    printResults("工作负载剧烈变化测试", get_operations, hits);
}

