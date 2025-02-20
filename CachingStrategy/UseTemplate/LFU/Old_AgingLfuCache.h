#pragma once
#include "..\ICachePolicy.h"
#include "LfuNode.h"
#include"NodeList.h"
#include <mutex>
#include <unordered_map>

template<typename Key, typename Value>
class AgingLfuCache :public ICachePolicy<Key, Value> {
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
	unsigned int maxAverageFreqNum_;
	unsigned int curAverageFreqNum_;
	unsigned int totalFreqNum_;

public:
	AgingLfuCache() = delete;
	AgingLfuCache(unsigned int, unsigned int = 20);
	~AgingLfuCache() = default;
	void put(const Key&, const Value&);
	optional<Value> get(const Key&);
	bool isExists(const Key&);
	bool remove(const Key&);
private:
	void updateNode(const NodePtr&, const Value&);
	void insertNewNode(const Key&, const NodePtr&);
	void insertIntoFreqHash(const NodePtr&);
	void insertIntoNodeHash(const Key&, const NodePtr&);
	void removeNode(const NodePtr&);
	void removeFromFreqHash(const NodePtr&,const char&&='n');
	void removeFromNodeHash(const NodePtr&);
	void evictLeastFrequentNode();
	void updateMinFreq();
	void increaseTotalFreqNum();
	void decreaseTotalFreqNum(const unsigned int&);
	void handleOverMaxAverageFreqNum();
};

template<typename Key, typename Value>
AgingLfuCache<Key, Value>::AgingLfuCache(unsigned int capacity, unsigned int maxAverageFreqNum) :
capacity_{ capacity },
maxAverageFreqNum_{ maxAverageFreqNum }, curAverageFreqNum_{ 0 },
totalFreqNum_{ 0 }, minFreq_{ static_cast<unsigned int>(INT64_MAX) } {}


template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::put(const Key& key, const Value& value) {
	lock_guard<mutex> lock{ this->mutex_ };
	
	if (this->nodeHash_.find(key) != this->nodeHash_.end()) {
		NodePtr node = this->nodeHash_[key];
		updateNode(node, value);
		increaseTotalFreqNum();
		return;
	}
	if (this->nodeHash_.size() >= this->capacity_) {
		evictLeastFrequentNode();
	}

	NodePtr newNode = make_shared<Node>(key, value);
	insertNewNode(key, newNode);
	increaseTotalFreqNum();
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::updateNode(const NodePtr& node, const Value& value) {
	node->setValue(value);
	removeFromFreqHash(node,'u');
	node->increaseFrequency();
	insertIntoFreqHash(node);
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::updateMinFreq() {
	this->minFreq_ = UINT_MAX;
	for (const auto& pair : this->freqHash_) {
		if (!pair.second->isEmpty() && pair.first < this->minFreq_) {
			this->minFreq_ = pair.first;
		}
	}
	if (this->minFreq_ == UINT_MAX) {
		this->minFreq_ = 1;
	}
}



template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::insertNewNode(const Key& key, const NodePtr& node) {
	insertIntoNodeHash(key, node);
	insertIntoFreqHash(node);
	this->minFreq_ = 1;
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::insertIntoFreqHash(const NodePtr& node) {
	const unsigned int index = node->getFrequency();
	auto it = this->freqHash_.find(index);
	if (it == this->freqHash_.end()) {
		FreqPtr newFreqPtr = make_shared<FreqList>();
		this->freqHash_[index] = newFreqPtr;
	}
	this->freqHash_[index]->insertNode(node);
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::insertIntoNodeHash(const Key& key, const NodePtr& node) {
	this->nodeHash_[key] = node;
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::removeNode(const NodePtr& node) {
	removeFromFreqHash(node,'d');
	removeFromNodeHash(node);
	decreaseTotalFreqNum(node->getFrequency());
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::removeFromNodeHash(const NodePtr& node) {
	const Key key = node->getKey();
	this->nodeHash_.erase(key);
}


template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::removeFromFreqHash(const NodePtr& node,const char&& flag) {
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
void AgingLfuCache<Key, Value>::evictLeastFrequentNode() {
	FreqPtr freqList = this->freqHash_[this->minFreq_];
	NodePtr node = freqList->getLeastNode();
	removeNode(node);
}

template<typename Key, typename Value>
optional<Value> AgingLfuCache<Key, Value>::get(const Key& key) {
	lock_guard<mutex> lock{ this->mutex_ };
	increaseTotalFreqNum();
	if (isExists(key) == false) return nullopt;
	NodePtr node = this->nodeHash_[key];
	const Value value = node->getValue();
	updateNode(node, value);
	return value;
}

template<typename Key, typename Value>
bool AgingLfuCache<Key, Value>::isExists(const Key& key) {
	if (this->nodeHash_.find(key) != this->nodeHash_.end())
		return true;
	else
		return false;
}

template<typename Key, typename Value>
bool AgingLfuCache<Key, Value>::remove(const Key& key) {
	if (isExists(key) == false) return false;
	NodePtr node = this->nodeHash_[key];
	removeNode(node);
	return true;
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::increaseTotalFreqNum()
{
	this->totalFreqNum_++;
	this->curAverageFreqNum_ = this->totalFreqNum_ / this->nodeHash_.size();
	if (this->curAverageFreqNum_ > this->maxAverageFreqNum_) {
		handleOverMaxAverageFreqNum();
	}
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::decreaseTotalFreqNum(const unsigned int& index)
{
	this->totalFreqNum_ -= index;
	if (this->nodeHash_.empty())
		this->curAverageFreqNum_ = 0;
	else if (this->totalFreqNum_ <= 0) 
		this->curAverageFreqNum_ = 0;
	else
	this->curAverageFreqNum_ = this->totalFreqNum_ / this->nodeHash_.size();
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::handleOverMaxAverageFreqNum()
{
	unsigned int subtrahend = this->maxAverageFreqNum_ / 2;
	this->totalFreqNum_ = 0;
	for (const auto& pair : this->nodeHash_) {
		NodePtr curNode = pair.second;
		removeFromFreqHash(curNode);
		unsigned int curFrequency = curNode->getFrequency();
		unsigned int realFrequency = curFrequency - subtrahend;
		if (realFrequency < 1) {
			curNode->setFrequency(1);
			this->totalFreqNum_++;
		}
		else {
			curNode->setFrequency(realFrequency);
			this->totalFreqNum_ += realFrequency;
		}
		insertIntoFreqHash(curNode);
	}
	this->curAverageFreqNum_ = this->totalFreqNum_ / this->nodeHash_.size();
	updateMinFreq();
}