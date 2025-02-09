#pragma once
#include "..\ICachePolicy.h"
#include "LfuNode.h"
#include"NodeList.h"
#include <mutex>
#include <unordered_map>

template<typename Key,typename Value>
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
	void put(const Key&,const Value&);
	optional<Value> get(const Key&);
	bool isExit(const Key&);
	bool remove(const Key&);
private:
	void updateNode(NodePtr&,const Value&);
	void insertNewNode(const Key&,NodePtr&);
	void insertIntoFreqHash(NodePtr&);
	void insertIntoNodeHash(const Key&,NodePtr&);
	void removeNode(NodePtr&);
	void removeFromFreqHash(NodePtr&);
	void removeFromNodeHash(NodePtr&);
	void evictLeastFrequentNode();
	void updateMinFreq();
	
};

template<typename Key,typename Value>
LfuCache<Key, Value>::LfuCache(unsigned int capacity) :capacity_{ capacity }, minFreq_{INT64_MAX} {}


template<typename Key,typename Value>
void LfuCache<Key, Value>::put(const Key& key,const Value& value) {
	if (this->nodeHash_.find(key) != this->nodeHash_.end()) {
		NodePtr node = this->nodeHash_[key];
		updateNode(node,value);
		return;
	}
	if (this->nodeHash_.size() >= this->capacity_) {
		evictLeastFrequentNode();
	}

	NodePtr newNode = make_shared<Node>(key,value);
	insertNode(key,newNode);
}

template<typename Key,typename Value>
void LfuCache<Key, Value>::updateNode(NodePtr& node, const Value& value) {
	node->setValue(value);
	removeFromFreqHash(node);
	node->increaseFrequency();
	insertIntoFreqHash(node);
}

template<typename Key,typename Value>
void LfuCache<Key, Value>::updateMinFreq() {
	const unsigned int start = this->minFreq_;
	const unsigned int maximums = this->freqHash_.size();
	for (int i = start; i < maximums; i++) {
		if (this->freqHash_[i]->isEmpty())
			continue;
		this->minFreq_ = i;
		break;
	}
}

template<typename Key,typename Value>
void LfuCache<Key, Value>::insertNewNode(const Key& key, NodePtr& node) {
	insertIntoNodeHash(key, node);
	insertIntoFreqHash(node);
	this->minFreq_ = 1;
}

template<typename Key,typename Value>
void LfuCache<Key, Value>::insertIntoFreqHash(NodePtr& node) {
	const unsigned int index = node->getFrequency();
	const FreqPtr freqPtr = this->freqHash_[index];
	freqPtr->insertNode(node);
}

template<typename Key,typename Value>
void LfuCache<Key,Value>::insertIntoNodeHash(const Key& key,NodePtr& node){
	this->nodeHash_[key] = node;
}

template<typename Key, typename Value>
void LfuCache<Key, Value>::removeNode(NodePtr& node) {
	removeFromFreqHash(node);
	removeFromNodeHash(node);
}

template<typename Key,typename Value>
void LfuCache<Key, Value>::removeFromNodeHash(NodePtr& node) {
	const Key key = node->getKey();
	this->nodeHash_.erase(key);
}

//there is a doubt about this
//when remove operation is used in different situations,
//it fails to achieve the expected effect
template<typename Key, typename Value>
void LfuCache<Key, Value>::removeFromFreqHash(NodePtr& node) {
	unsigned int index = node->getFrequency();
	FreqPtr freqPtr = this->freqHash_[index];
	freqList->removeNode(node);

	if (index = this->minFreq_) {
		if (freqPtr->isEmpty()) {
			updateMinFreq();
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