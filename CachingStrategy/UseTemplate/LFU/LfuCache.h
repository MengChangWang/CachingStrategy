#pragma once
#include "..\ICachePolicy.h"
#include "LfuNode.h"
#include"NodeList.h"
#include <mutex>
#include <unordered_map>

template<typename Key, typename Value>
class LfuCache :public ICachePolicy<Key, Value> {
private:
	using Node = LfuNode<Key, Value>;
	using NodePtr = shared_ptr<Node>;
	using NodeHash = unordered_map<Key, NodePtr>;
	using FreqList = NodeList<Key, Value>;
	using FreqPtr = shared_ptr<FreqList>;
	using FreqHash = unordered_map<unsigned int, FreqPtr>;

private:
	mutex mutex_;
	NodeHash nodeHash_;
	FreqHash freqHash_;
	unsigned int capacity_;
	unsigned int minFreq_;

public:
	LfuCache() = delete;
	LfuCache(unsigned int capacity);
	~LfuCache() = default;
	void put(const Key&, const Value&);
	optional<Value> get(const Key&);
	bool isExit(const Key&);
	bool remove(const Key&);
private:
	void updateNode(const NodePtr&, const Value&);
	void insertNewNode(const Key&, const NodePtr&);
	void insertIntoFreqHash(const NodePtr&);
	void insertIntoNodeHash(const Key&, const NodePtr&);
	void removeNode(const NodePtr&);
	void removeFromFreqHash(const NodePtr&, const char&&);
	void removeFromNodeHash(const NodePtr&);
	void evictLeastFrequentNode();
	void updateMinFreq();

};

template<typename Key, typename Value>
LfuCache<Key, Value>::LfuCache(unsigned int capacity) :capacity_{ capacity }, minFreq_{ (UINT_MAX) } {}


template<typename Key, typename Value>
void LfuCache<Key, Value>::put(const Key& key, const Value& value) {
	lock_guard<mutex> lock{ this->mutex_ };
	if (this->nodeHash_.find(key) != this->nodeHash_.end()) {
		NodePtr node = this->nodeHash_[key];
		updateNode(node, value);
		return;
	}
	if (this->nodeHash_.size() >= this->capacity_) {
		evictLeastFrequentNode();
	}

	NodePtr newNode = make_shared<Node>(key, value);
	insertNewNode(key, newNode);
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::updateNode(const NodePtr& node, const Value& value) {
	node->setValue(value);
	removeFromFreqHash(node, 'u');
	node->increaseFrequency();
	insertIntoFreqHash(node);
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::updateMinFreq() {

	this->minFreq_ = UINT_MAX;
	for (const auto& pair : this->freqHash_) {
		if (!pair.second->isEmpty() && pair.first < this->minFreq_)
			this->minFreq_ = pair.first;
	}
	if (this->minFreq_ == UINT_MAX)
		this->minFreq_ = 1;
	/*const unsigned int start = this->minFreq_;
	const unsigned int maximums = this->freqHash_.size();
	for (int i = start; i < maximums; i++) {
		if (this->freqHash_[i]->isEmpty())
			continue;
		else {
			this->minFreq_ = i;
			break;
		}
	}*/
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::insertNewNode(const Key& key, const NodePtr& node) {
	insertIntoNodeHash(key, node);
	insertIntoFreqHash(node);
	this->minFreq_ = 1;
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::insertIntoFreqHash(const NodePtr& node) {
	const unsigned int index = node->getFrequency();
	auto it = this->freqHash_.find(index);
	if (it == this->freqHash_.end()) {
		FreqPtr newFreqPtr = make_shared<FreqList>();
		this->freqHash_[index] = newFreqPtr;
	}
	this->freqHash_[index]->insertNode(node);
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::insertIntoNodeHash(const Key& key, const NodePtr& node) {
	this->nodeHash_[key] = node;
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::removeNode(const NodePtr& node) {
	removeFromFreqHash(node, 'd');
	removeFromNodeHash(node);
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::removeFromNodeHash(const NodePtr& node) {
	const Key key = node->getKey();
	this->nodeHash_.erase(key);
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::removeFromFreqHash(const NodePtr& node, const char&& flag) {
	unsigned int index = node->getFrequency();
	FreqPtr freqPtr = this->freqHash_[index];
	freqPtr->removeNode(node);

	if (freqPtr->isEmpty()) {
		this->freqHash_.erase(index);
		if (index == this->minFreq_) {
			if (flag == 'd')
				updateMinFreq();
			else if (flag == 'u')
				this->minFreq_++;
		}
	}
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::evictLeastFrequentNode() {
	FreqPtr freqList = this->freqHash_[this->minFreq_];
	NodePtr node = freqList->getLeastNode();
	removeNode(node);

	/*removeFromNodeHash(node);
	* freqList->removeNode(node);
	*/
}

template<typename Key, typename Value>
optional<Value> LfuCache<Key, Value>::get(const Key& key) {
	lock_guard<mutex> lock{ this->mutex_ };
	if (isExit(key) == false) return nullopt;
	NodePtr node = this->nodeHash_[key];
	const Value value = node->getValue();
	updateNode(node, value);
	return value;
}

template<typename Key, typename Value>
bool LfuCache<Key, Value>::isExit(const Key& key) {
	if (this->nodeHash_.find(key) != this->nodeHash_.end())
		return true;
	else
		return false;
}

template<typename Key, typename Value>
bool LfuCache<Key, Value>::remove(const Key& key) {
	if (isExit(key) == false) return false;
	NodePtr node = this->nodeHash_[key];
	removeNode(node);
	return true;
}