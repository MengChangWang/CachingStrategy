#pragma once
#include<iostream>
#include<unordered_map>
#include<ctime>
using namespace std;

class LRUWithDiffTTL {
private:
	struct Node {
		int key;
		int value;
		time_t expire_time;

		Node* next;
		Node* pre;
		Node(int key, int value, time_t expire_time) {
			this->key = key;
			this->value = value;
			this->expire_time = expire_time;

			this->next = nullptr;
			this->pre = nullptr;
		}
	};
	unordered_map<int, Node*> hash;
	Node* L, * R;
	int capacity;

	void insert(int key, int value, time_t ttl) {
		time_t cur_time = time(nullptr);
		Node* new_node = new Node(key, value, cur_time + ttl);

		Node* nn_next = this->R;
		Node* nn_pre = this->R->pre;

		nn_pre->next = new_node;
		new_node->next = nn_next;

		nn_next->pre = new_node;
		new_node->pre = nn_pre;

		this->hash[key] = new_node;
	}

	void remove(int key) {
		Node* temp = this->hash[key];

		temp->next->pre = temp->pre;
		temp->pre->next = temp->next;

		delete temp;
		this->hash.erase(key);
	}
public:
	LRUWithDiffTTL(int capacity) {
		this->capacity = capacity;

		L = new Node(-1, -1, 0);
		R = new Node(-1, -1, 0);

		L->next = R;
		R->pre = L;
	}

	int get(int key) {
		if (hash.find(key) == hash.end()) return -1;
		else {
			Node* node = hash[key];
			time_t cur_time = time(nullptr);
			if (difftime(node->expire_time, cur_time) <= 0) {
				remove(key);
				return -1;
			}
			else {
				time_t time_left = difftime(node->expire_time, cur_time);
				int value = node->value;
				remove(key);
				insert(key, value,time_left);
				return value;
			}
		}
	}
	void put(int key, int value, time_t ttl) {
		if (this->hash.find(key) != hash.end()) {
			remove(key);
			insert(key, value, ttl);
		}
		else {
			if (this->hash.size() == this->capacity) {
				Node* node = hash[key];
				time_t cur_time = time(nullptr);
				unordered_map<int, Node*>::iterator it;

				bool has_expire = false;
				for (it = hash.begin(); it != hash.end(); it++) {
					if (difftime(it->second->expire_time, cur_time) <= 0) {
						remove(it->first);
						has_expire = true;
						break;
					}
				}
				if(has_expire)	insert(key, value, ttl);
				else {
					remove(L->next->key);
					insert(key, value, ttl);
				}
			}
			else {
				insert(key, value, ttl);
			}
		}
	}
};