#pragma once
#include "ArcNodeList.h"
#include "../ICachePolicy.h"
#include <mutex>
#include <unordered_map>
#include <map>

template<typename Key, typename Value>
class ArcLfu:public ICachePolicy<Key, Value> {
private:
	using Node = ArcNode<Key, Value>;
	using NodePtr = shared_ptr<Node>;
	using NodeHash = unordered_map<Key, NodePtr>;
	using FreqList = ArcNodeList<Key, Value>;
	using FreqPtr = shared_ptr<FreqList>;
	using FreqHash = map<unsigned int, FreqPtr>;

private:
	mutex mutex_;
	NodeHash nodeHash_;
	FreqHash freqHash_;
	unsigned int capacity_;
	//unsigned int minFreq_;

public:
	ArcLfu() = delete;
	ArcLfu(unsigned int capacity);
	~ArcLfu() = default;
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
	void removeFromFreqHash(const NodePtr&);
	void removeFromNodeHash(const NodePtr&);
	void evictLeastFrequentNode();
	//void updateMinFreq();

};

template<typename Key, typename Value>
ArcLfu<Key, Value>::ArcLfu(unsigned int capacity) :capacity_{ capacity } {}


template<typename Key, typename Value>
void ArcLfu<Key, Value>::put(const Key& key, const Value& value) {
	lock_guard<mutex> lock{ this->mutex_ };
	auto it = this->nodeHash_.find(key);
	if (it != this->nodeHash_.end()) {
		NodePtr node = it->second;
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
void ArcLfu<Key, Value>::updateNode(const NodePtr& node, const Value& value) {
	node->setValue(value);
	removeFromFreqHash(node);
	node->increaseFrequency();
	insertIntoFreqHash(node);
}

//template<typename Key, typename Value>
//void LfuCache<Key, Value>::updateMinFreq() {
//
//	this->minFreq_ = UINT_MAX;
//	for (const auto& pair : this->freqHash_) {
//		if (!pair.second->isEmpty() && pair.first < this->minFreq_)
//			this->minFreq_ = pair.first;
//	}
//	if (this->minFreq_ == UINT_MAX)
//		this->minFreq_ = 1;
//}

template<typename Key, typename Value>
void ArcLfu<Key, Value>::insertNewNode(const Key& key, const NodePtr& node) {
	insertIntoNodeHash(key, node);
	insertIntoFreqHash(node);
}

template<typename Key, typename Value>
void ArcLfu<Key, Value>::insertIntoFreqHash(const NodePtr& node) {
	const unsigned int index = node->getFrequency();
	auto it = this->freqHash_.find(index);
	if (it == this->freqHash_.end()) {
		FreqPtr newFreqPtr = make_shared<FreqList>();
		this->freqHash_[index] = newFreqPtr;
	}
	this->freqHash_[index]->insertNode(node);
}

template<typename Key, typename Value>
void ArcLfu<Key, Value>::insertIntoNodeHash(const Key& key, const NodePtr& node) {
	this->nodeHash_.emplace(key, node);
}

template<typename Key, typename Value>
void ArcLfu<Key, Value>::removeNode(const NodePtr& node) {
	removeFromFreqHash(node);
	removeFromNodeHash(node);
}

template<typename Key, typename Value>
void ArcLfu<Key, Value>::removeFromNodeHash(const NodePtr& node) {
	const Key key = node->getKey();
	this->nodeHash_.erase(key);
}

template<typename Key, typename Value>
void ArcLfu<Key, Value>::removeFromFreqHash(const NodePtr& node) {
	unsigned int freq = node->getFrequency();
	auto it = this->freqHash_.find(freq);
	if (it == this->freqHash_.end())
		return;
	it->second->removeNode(node);
	if (it->second->isEmpty()) {
		this->freqHash_.erase(it);

	}
}

template<typename Key, typename Value>
void ArcLfu<Key, Value>::evictLeastFrequentNode() {
	auto it = this->freqHash_.begin();
	if (it == this->freqHash_.end())
		return;
	NodePtr node = it->second->getLeastNode();
	removeNode(node);

	/*removeFromNodeHash(node);
	* freqList->removeNode(node);
	*/
}

template<typename Key, typename Value>
optional<Value> ArcLfu<Key, Value>::get(const Key& key) {
	lock_guard<mutex> lock{ this->mutex_ };
	auto it = this->nodeHash_.find(key);
	if (it == this->nodeHash_.end())
		return nullopt;
	NodePtr node = it->second;
	const Value value = node->getValue();
	updateNode(node, value);
	return value;
}

template<typename Key, typename Value>
bool ArcLfu<Key, Value>::isExists(const Key& key) {
	return this->nodeHash_.find(key) != this->nodeHash_.end();
}

template<typename Key, typename Value>
bool ArcLfu<Key, Value>::remove(const Key& key) {
	auto it = this->nodeHash_.find(key);
	if (it == this->nodeHash_.end())
		return false;
	NodePtr node = it->second;
	removeNode(node);
	return true;
}