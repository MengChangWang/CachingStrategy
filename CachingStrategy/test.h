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

// ������������ӡ���
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
    //std::cout << "�����С: " << capacity << std::endl;
    std::cout << "LRU - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "LRU-K - �����ʣ�" << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "Slice_LRU - �����ʣ�" << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "LFU - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "Aging_LFU - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "ARC - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
}

template<typename Key, typename Value>
void testHotDataAccess(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations) {
    std::cout << "\n=== ���Գ���1���ȵ����ݷ��ʲ��� ===" << std::endl;

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

    // �Ƚ���һϵ��put����
    for (int i = 0; i < caches.size(); ++i) {
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            if (op % 100 < 40) {  // 40%�ȵ�����
                key = gen() % HOT_KEYS;
            }
            else {  // 60%������
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }
            std::string value = "value" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // Ȼ��������get����
        for (int get_op = 0; get_op < OPERATIONS / 2; ++get_op) {
            int key;
            if (get_op % 100 < 40) {  // 40%���ʷ����ȵ�
                key = gen() % HOT_KEYS;
            }
            else {  // 60%���ʷ���������
                key = HOT_KEYS + (gen() % COLD_KEYS);
            }

            get_operations[i]++;
            if (caches[i]->isExists(key)) {
                hits[i]++;
            }
        
        }
    }

    printResults("�ȵ����ݷ��ʲ���", get_operations, hits);
}

template<typename Key, typename Value>
void testLoopPattern(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations) {
    std::cout << "\n=== ���Գ���2��ѭ��ɨ����� ===" << std::endl;

    const int LOOP_SIZE = 200;
    const int OPERATIONS = 10000;

    std::random_device rd;
    std::mt19937 gen(rd());

    // ���������
    for (int i = 0; i < caches.size(); ++i) {
        for (int key = 0; key < LOOP_SIZE * 2; ++key) {
            std::string value = "loop" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // Ȼ����з��ʲ���
        int current_pos = 0;
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            if (op % 100 < 70) {  // 70%˳��ɨ��
                key = current_pos;
                current_pos = (current_pos + 1) % LOOP_SIZE;
            }
            else if (op % 100 < 85) {  // 15%�����Ծ
                key = gen() % LOOP_SIZE;
            }
            else {  // 15%���ʷ�Χ������
                key = LOOP_SIZE + (gen() % LOOP_SIZE);
            }

            get_operations[i]++;
            if (caches[i]->isExists(key)) {
                hits[i]++;
            }
         
        }
    }

    printResults("ѭ��ɨ�����", get_operations, hits);
}

template<typename Key, typename Value>
void testWorkloadShift(const std::vector<ICachePolicy<Key, Value>*>& caches, std::vector<int>& hits, std::vector<int>& get_operations) {
    std::cout << "\n=== ���Գ���3���������ؾ��ұ仯���� ===" << std::endl;

    const int OPERATIONS = 10000;
    const int PHASE_LENGTH = OPERATIONS / 5;

    std::random_device rd;
    std::mt19937 gen(rd());

    // �����һЩ��ʼ����
    for (int i = 0; i < caches.size(); ++i) {
        for (int key = 0; key < 1000; ++key) {
            std::string value = "init" + std::to_string(key);
            caches[i]->put(key, value);
        }

        // Ȼ����ж�׶β���
        for (int op = 0; op < OPERATIONS; ++op) {
            int key;
            // ���ݲ�ͬ�׶�ѡ��ͬ�ķ���ģʽ
            if (op < PHASE_LENGTH) {  // �ȵ����
                key = gen() % 5;
            }
            else if (op < PHASE_LENGTH * 2) {  // ��Χ���
                key = gen() % 1000;
            }
            else if (op < PHASE_LENGTH * 3) {  // ˳��ɨ��
                key = (op - PHASE_LENGTH * 2) % 100;
            }
            else if (op < PHASE_LENGTH * 4) {  // �ֲ������
                int locality = (op / 1000) % 10;
                key = locality * 20 + (gen() % 20);
            }
            else {  // ��Ϸ���
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

            // �������put���������»�������
            if (gen() % 100 < 30) {  // 30%���ʽ���put
                std::string value = "new" + std::to_string(key);
                caches[i]->put(key, value);
            }
        }
    }

    printResults("�������ؾ��ұ仯����", get_operations, hits);
}

