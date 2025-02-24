#pragma once
#include "..\ICachePolicy.h"
#include "LfuNode.h"
#include "NodeList.h"
#include <mutex>
#include <unordered_map>
#include <map>
#include <stdexcept>
#include <algorithm>


template<typename Key, typename Value>
class AgingLfuCache : public ICachePolicy<Key, Value> {
private:
    using Node = LfuNode<Key, Value>;
    using NodePtr = shared_ptr<Node>;
    using NodeHash = unordered_map<Key, NodePtr>;
    using FreqList = NodeList<Key, Value>;
    using FreqPtr = shared_ptr<FreqList>;
    using FreqHash = map<unsigned int, FreqPtr>;
    

private:
    mutex mutex_;
    NodeHash nodeHash_;
    FreqHash freqHash_;
    unsigned int capacity_;
    unsigned int maxAverageFreqNum_;
    unsigned int curAverageFreqNum_;
    unsigned int totalFreqNum_;

public:
    AgingLfuCache() = delete;
    AgingLfuCache(unsigned int capacity, unsigned int maxAverageFreqNum = 10);
    ~AgingLfuCache() = default;

    void put(const Key& key, const Value& value) override;
    optional<Value> get(const Key& key) override;
    bool isExists(const Key& key)  override;
    bool remove(const Key& key) override;

private:
    void updateNode(const NodePtr& node, const Value& value);
    void insertNewNode(const Key& key, const NodePtr& node);
    void insertIntoFreqHash(const NodePtr& node);
    void insertIntoNodeHash(const Key& key, const NodePtr& node);
    void removeNode(const NodePtr& node);
    void removeFromFreqHash(const NodePtr& node);
    void removeFromNodeHash(const NodePtr& node);
    void evictLeastFrequentNode();
    void increaseTotalFreqNum();
    void decreaseTotalFreqNum(unsigned int freq);
    void handleOverMaxAverageFreqNum();
};

template<typename Key, typename Value>
AgingLfuCache<Key, Value>::AgingLfuCache(unsigned int capacity, unsigned int maxAverageFreqNum)
    : capacity_(capacity),
    maxAverageFreqNum_(maxAverageFreqNum),
    curAverageFreqNum_(0),
    totalFreqNum_(0) {
    if (this->capacity_ == 0) {
        throw std::invalid_argument("In AgingLfuCache.h-----Capacity must be greater than 0.");
    }
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::put(const Key& key, const Value& value) {
    lock_guard<mutex> lock(mutex_);

    auto it = this->nodeHash_.find(key);
    if (it != this->nodeHash_.end()) {
        NodePtr node = it->second;
        updateNode(node, value); 
        increaseTotalFreqNum();
        return;
    }

    if (nodeHash_.size() >= capacity_) {
        evictLeastFrequentNode();
    }

    NodePtr newNode = make_shared<Node>(key, value);
    insertNewNode(key, newNode);
    increaseTotalFreqNum();
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::updateNode(const NodePtr& node, const Value& value) {
    node->setValue(value);
    removeFromFreqHash(node);
    node->increaseFrequency();
    insertIntoFreqHash(node);
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::insertNewNode(const Key& key, const NodePtr& node) {
    insertIntoNodeHash(key, node);
    insertIntoFreqHash(node);
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::insertIntoFreqHash(const NodePtr& node) {
    const unsigned int freq = node->getFrequency();
    auto& freqList = freqHash_[freq];  // 自动插入 nullptr 若不存在
    if (!freqList) {
        freqList = make_shared<FreqList>();
    }
    freqList->insertNode(node);
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::insertIntoNodeHash(const Key& key, const NodePtr& node) {
    this->nodeHash_.emplace(key, node);
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::removeNode(const NodePtr& node) {
    removeFromFreqHash(node);
    removeFromNodeHash(node);
    decreaseTotalFreqNum(node->getFrequency());
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::removeFromFreqHash(const NodePtr& node) {
    unsigned int freq = node->getFrequency();
    auto it = freqHash_.find(freq);
    if (it == freqHash_.end()) return;

    FreqPtr& freqList = it->second;
    freqList->removeNode(node);

    if (freqList->isEmpty()) {
        freqHash_.erase(it);
       
    }
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::removeFromNodeHash(const NodePtr& node) {
    this->nodeHash_.erase(node->getKey());
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::evictLeastFrequentNode() {
    auto it = this->freqHash_.begin();
    if (it == this->freqHash_.end() || it->second->isEmpty()) {       
        throw logic_error("In AgingLfuCache.h-----Cannot evict from an empty cache."); 
        return;
    }

    NodePtr node = it->second->getLeastNode();
    if (!node) {
        throw logic_error("In AgingLfuCache.h-----Cannot find least frequent node.");
    }
    else {
    removeNode(node);
    }
}

template<typename Key, typename Value>
std::optional<Value> AgingLfuCache<Key, Value>::get(const Key& key) {
    lock_guard<mutex> lock(mutex_);
    

    auto it = nodeHash_.find(key);
    if (it == nodeHash_.end()) {
        return nullopt;
    }

    increaseTotalFreqNum();

    NodePtr node = it->second;
    Value value = node->getValue();
    updateNode(node, value);
    return value;
}

template<typename Key, typename Value>
bool AgingLfuCache<Key, Value>::isExists(const Key& key) {
    return nodeHash_.find(key) != nodeHash_.end();
}

template<typename Key, typename Value>
bool AgingLfuCache<Key, Value>::remove(const Key& key) {
    lock_guard<mutex> lock(mutex_);
    auto it = this->nodeHash_.find(key);
    if (it == this->nodeHash_.end()) {
        return false;
    }
    removeNode(it->second);
    return true;
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::increaseTotalFreqNum() {
    this->totalFreqNum_++;
    this->curAverageFreqNum_ = this->nodeHash_.empty() ? 0 : totalFreqNum_ / nodeHash_.size();
    if (this->curAverageFreqNum_ > this->maxAverageFreqNum_) {
        handleOverMaxAverageFreqNum();
    }
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::decreaseTotalFreqNum(unsigned int freq) {
    this->totalFreqNum_ -= freq;
    this->curAverageFreqNum_ = this->nodeHash_.empty() ? 0 : this->totalFreqNum_ / this->nodeHash_.size();
}

template<typename Key, typename Value>
void AgingLfuCache<Key, Value>::handleOverMaxAverageFreqNum() {
    const unsigned int subtrahend = this->maxAverageFreqNum_ / 2;
    this->totalFreqNum_ = 0;

    for (auto& pair : this->nodeHash_) {
        NodePtr node = pair.second;
        removeFromFreqHash(node);
        unsigned int newFreq = max(node->getFrequency() - subtrahend, 1u);
        node->setFrequency(newFreq);
        this->totalFreqNum_ += newFreq;
        insertIntoFreqHash(node);
    }

    this->curAverageFreqNum_ = this->nodeHash_.empty() ? 0 : this->totalFreqNum_ / this->nodeHash_.size();
}