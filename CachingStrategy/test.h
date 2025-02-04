#pragma once
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>
#include <random>
#include <algorithm>
#include <vector>
#include <array>

#include "UseTemplate_LRU\ICachePolicy.h"
#include "UseTemplate_LRU\LruCache.h"
#include "UseTemplate_LRU\LruKCache.h"



class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsed() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// ������������ӡ���
void printResults(const std::string& testName, int capacity,
    const std::vector<int>& get_operations,
    const std::vector<int>& hits) {
    size_t n = hits.size();
    size_t i = 0;
    std::cout << "�����С: " << capacity << std::endl;
    std::cout << "LRU - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "LRU-K - �����ʣ�" << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "LRU-Hash - �����ʣ�" << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "LFU - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
    i++;
    std::cout << "ARC - ������: " << std::fixed << std::setprecision(2)
        << (100.0 * hits[i] / get_operations[i]) << "%" << std::endl;
}

void testHotDataAccess() {
    std::cout << "\n=== ���Գ���1���ȵ����ݷ��ʲ��� ===" << std::endl;

    const int CAPACITY = 5;
    const int OPERATIONS = 100000;
    const int HOT_KEYS = 3;
    const int COLD_KEYS = 5000;

    LruCache<int, std::string> lru(CAPACITY);
    LruKCache<int, std::string> lru_k(CAPACITY, CAPACITY, 2);

    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<ICachePolicy<int, std::string>*, 2> caches = { &lru,&lru_k };
    std::vector<int> hits(5, 0);
    std::vector<int> get_operations(5, 0);

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
            if (caches[i]->isExit(key)) {
                hits[i]++;
            }
        }
    }

    printResults("�ȵ����ݷ��ʲ���", CAPACITY, get_operations, hits);
}

void testLoopPattern() {
    std::cout << "\n=== ���Գ���2��ѭ��ɨ����� ===" << std::endl;

    const int CAPACITY = 3;
    const int LOOP_SIZE = 200;
    const int OPERATIONS = 50000;

    LruCache<int, std::string> lru(CAPACITY);
    LruKCache<int, std::string> lru_k(CAPACITY, CAPACITY, 2);

    std::array<ICachePolicy<int, std::string>*, 2> caches = { &lru,&lru_k };
    std::vector<int> hits(5, 0);
    std::vector<int> get_operations(5, 0);

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
            if (caches[i]->isExit(key)) {
                hits[i]++;
            }
        }
    }

    printResults("ѭ��ɨ�����", CAPACITY, get_operations, hits);
}

void testWorkloadShift() {
    std::cout << "\n=== ���Գ���3���������ؾ��ұ仯���� ===" << std::endl;

    const int CAPACITY = 4;
    const int OPERATIONS = 80000;
    const int PHASE_LENGTH = OPERATIONS / 5;

    LruCache<int, std::string> lru(CAPACITY);
    LruKCache<int, std::string> lru_k(CAPACITY, CAPACITY, 2);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::array<ICachePolicy<int, std::string>*, 2> caches = { &lru,&lru_k };
    std::vector<int> hits(5, 0);
    std::vector<int> get_operations(5, 0);

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
            if (caches[i]->isExit(key)) {
                hits[i]++;
            }

            // �������put���������»�������
            if (gen() % 100 < 30) {  // 30%���ʽ���put
                std::string value = "new" + std::to_string(key);
                caches[i]->put(key, value);
            }
        }
    }

    printResults("�������ؾ��ұ仯����", CAPACITY, get_operations, hits);
}

