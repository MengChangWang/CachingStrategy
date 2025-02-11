#pragma once
#include <unordered_map>
#include <mutex>

#include "LruNode.h"
#include "..\ICachePolicy.h"

template<typename Key, typename Value>
class LruCache : public ICachePolicy<Key, Value> {
public:
    using Node = LruNode<Key, Value>;
    using NodePtr = shared_ptr<Node>;
    using NodeHash = unordered_map<Key, NodePtr>;

private:
    size_t capacity_;
    mutex mutex_;
    NodeHash nodeHash_;
    NodePtr dummyHead_;
    NodePtr dummyTail_;

public:
    LruCache(unsigned int capacity);
    ~LruCache() override = default;

    void put(const Key& key,const Value& value) override;
    bool isExit(const Key& key) override;
    optional<Value> get(const Key& key) override;
    bool remove(const Key& key) override;
    NodePtr getNode(const Key& key);
    void moveToRecentPosition(const NodePtr& node);
    

private:
    void initializeList();
    void insertNode(const NodePtr& node);
    void removeNode(const NodePtr& node);
    void updateExitingNode(const NodePtr& node, const Value& value);
    void evictLeastAccessNode();
    void addNewNode(const Key& key, const Value& value);
    
};

template<typename Key, typename Value>
LruCache<Key, Value>::LruCache(unsigned int capacity) : capacity_(capacity) {
    initializeList();
}

template<typename Key, typename Value>
void LruCache<Key, Value>::put(const Key& key,const Value& value) {
    if (this->capacity_ <= 0) return;
    lock_guard<mutex> lock{ this->mutex_ };
    auto it = nodeHash_.find(key);
    if (it != nodeHash_.end()) {
        updateExitingNode(it->second, value);
        return;
    }
    addNewNode(key, value);
}

template<typename Key, typename Value>
bool LruCache<Key, Value>::isExit(const Key& key) {
    auto it = nodeHash_.find(key);
    if (it == nodeHash_.end()) {
        return false;
    }
    return true;
}

template<typename Key, typename Value>
optional<Value> LruCache<Key, Value>::get(const Key& key) {
    lock_guard<mutex> lock{ this->mutex_ };
    Value value{};
    if (isExit(key) == false) return nullopt;
    else {
        NodePtr nodeAddress = nodeHash_[key];
        value = nodeAddress->getValue();
        nodeAddress->increaseAccessCount();
        moveToRecentPosition(nodeAddress);
        return value;
    }
}

template<typename Key, typename Value>
bool LruCache<Key, Value>::remove(const Key& key) {
    std::lock_guard<std::mutex> lock{ mutex_ };
    if (isExit(key) == false) return false;
    removeNode(nodeHash_[key]);
    nodeHash_.erase(key);
    return true;
}

template<typename Key, typename Value>
typename LruCache<Key, Value>::NodePtr LruCache<Key, Value>::getNode(const Key& key) {
    return nodeHash_[key];
}

template<typename Key, typename Value>
void LruCache<Key, Value>::moveToRecentPosition(const NodePtr& node) {
    removeNode(node);
    insertNode(node);
}

template<typename Key, typename Value>
void LruCache<Key, Value>::initializeList() {
    dummyHead_ = make_shared<Node>(Key(), Value());
    dummyTail_ = make_shared<Node>(Key(), Value());
    dummyHead_->setNext(dummyTail_);
    dummyTail_->setPre(dummyHead_);
}

template<typename Key, typename Value>
void LruCache<Key, Value>::insertNode(const NodePtr& node) {
    node->setNext(dummyTail_);
    node->setPre(dummyTail_->getPre());

    dummyTail_->getPre()->setNext(node);
    dummyTail_->setPre(node);
}

template<typename Key, typename Value>
void LruCache<Key, Value>::removeNode(const NodePtr& node) {
    node->getPre()->setNext(node->getNext());
    node->getNext()->setPre(node->getPre());
}

template<typename Key, typename Value>
void LruCache<Key, Value>::updateExitingNode(const NodePtr& node, const Value& value) {
    node->setValue(value);
    node->increaseAccessCount();
    moveToRecentPosition(node);
}

template<typename Key, typename Value>
void LruCache<Key, Value>::evictLeastAccessNode() {
    NodePtr leastNode = dummyHead_->getNext();
    removeNode(leastNode);
    nodeHash_.erase(leastNode->getKey());
}

template<typename Key, typename Value>
void LruCache<Key, Value>::addNewNode(const Key& key, const Value& value) {
    if (nodeHash_.size() >= this->capacity_) {
        evictLeastAccessNode();
    }
    NodePtr newNode = make_shared<Node>(key, value);
    insertNode(newNode);
    nodeHash_[key] = newNode;
}
