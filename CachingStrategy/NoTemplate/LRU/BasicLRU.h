#pragma once
#include<iostream>
#include <unordered_map>
using namespace std;

class BasicLRU {
private:
	struct Node {
		int key;
		int value;
		Node* pre, * next;
		Node(int key,int value) {
			this->key = key;
			this->value = value;
			pre = nullptr;
			next = nullptr;
		}
	};
	int n;//cache capacity
	unordered_map<int, Node*> hash;
	Node* L, * R;

	//first,remove the data from the doubly linked list
	//and then,remove it from the hash table
	void remove(int key) {
		Node* target = hash[key];

		target->pre->next = target->next;
		target->next->pre = target->pre;

		delete target;

		hash.erase(key);
	}

	//you need to first add the data to the doubly linked list
	//and then add it to the hash table
	void insert(int key, int value) {
		Node* new_node = new Node(key, value);	

		Node* nn_pre = R->pre;
		Node* nn_next = R;

		nn_pre->next = new_node;
		new_node->next = nn_next;

		nn_next->pre = new_node;
		new_node->pre = nn_pre;

		hash[key] = new_node;
	}

	


public:
	BasicLRU(int capacity) {
		this->n = capacity;

		L = new Node(-1, -1);
		R = new Node(-1, -1);

		L->next = R;
		R->pre = L;
	}

	int get(int key) {
		if (hash.find(key) == hash.end()) return -1;
		else {
			int value = hash[key]->value;
			remove(key);
			insert(key, value);
			return value;
		}
	}
	void put(int key, int value) {
		if (hash.find(key) != hash.end()) {
			remove(key);
			insert(key, value);
			return;
		}
		else {
			if (hash.size() == n) {
				remove(L->next->key);
				insert(key, value);
				return;
			}
			else {
				insert(key, value);
				return;
			}
		}
	}
};