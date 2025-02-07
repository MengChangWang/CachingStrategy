#pragma once
#include <unordered_map>
#include <mutex>
#include "LruNode.h"
#include "UseTemplate\ICachePolicy.h"

template<typename Key, typename Value>
class LruCache : public ICachePolicy<Key, Value> {
public:
    using LruNodeType = LruNode<Key, Value>;
    using NodePtr = std::shared_ptr<LruNodeType>;
    using NodeHash = std::unordered_map<Key, NodePtr>;

private:
    size_t capacity_;
    std::mutex mutex_;
    NodeHash nodeHash_;
    NodePtr dummyHead_;
    NodePtr dummyTail_;

public:
    LruCache(unsigned int capacity);
    ~LruCache() override = default;

    void put(Key key, Value& value) override;
    bool isExit(Key key) override;
    Value get(Key key) override;
    bool remove(Key key) override;
    NodePtr getNode(Key key);
    void moveToRecentPosition(NodePtr node);
    

private:
    void initializeList();
    void insertNode(NodePtr node);
    void removeNode(NodePtr node);
    void updateExitingNode(NodePtr node, const Value& value);
    void evictLeastAccessNode();
    void addNewNode(Key key, const Value& value);
    
};

template<typename Key, typename Value>
LruCache<Key, Value>::LruCache(unsigned int capacity) : capacity_(capacity) {
    initializeList();
}

template<typename Key, typename Value>
void LruCache<Key, Value>::put(Key key, Value& value) {
    if (this->capacity_ <= 0) return;
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = nodeHash_.find(key);
    if (it != nodeHash_.end()) {
        updateExitingNode(it->second, value);
        return;
    }
    addNewNode(key, value);
}

template<typename Key, typename Value>
bool LruCache<Key, Value>::isExit(Key key) {
    auto it = nodeHash_.find(key);
    if (it == nodeHash_.end()) {
        return false;
    }
    return true;
}

template<typename Key, typename Value>
Value LruCache<Key, Value>::get(Key key) {
    std::lock_guard<std::mutex> lock{ mutex_ };
    Value value{};
    if (isExit(key) == false) return value;
    else {
        NodePtr nodeAddress = nodeHash_[key];
        value = nodeAddress->getValue();
        nodeAddress->increaseAccessCount();
        moveToRecentPosition(nodeAddress);
        return value;
    }
}

template<typename Key, typename Value>
bool LruCache<Key, Value>::remove(Key key) {
    std::lock_guard<std::mutex> lock{ mutex_ };
    if (isExit(key) == false) return false;
    removeNode(nodeHash_[key]);
    nodeHash_.erase(key);
    return true;
}

template<typename Key, typename Value>
typename LruCache<Key, Value>::NodePtr LruCache<Key, Value>::getNode(Key key) {
    return nodeHash_[key];
}

template<typename Key, typename Value>
void LruCache<Key, Value>::moveToRecentPosition(NodePtr node) {
    removeNode(node);
    insertNode(node);
}

template<typename Key, typename Value>
void LruCache<Key, Value>::initializeList() {
    dummyHead_ = std::make_shared<LruNodeType>(Key(), Value());
    dummyTail_ = std::make_shared<LruNodeType>(Key(), Value());
    dummyHead_->setNext(dummyTail_);
    dummyTail_->setPre(dummyHead_);
}

template<typename Key, typename Value>
void LruCache<Key, Value>::insertNode(NodePtr node) {
    node->setNext(dummyTail_);
    node->setPre(dummyTail_->getPre());

    dummyTail_->getPre()->setNext(node);
    dummyTail_->setPre(node);
}

template<typename Key, typename Value>
void LruCache<Key, Value>::removeNode(NodePtr node) {
    node->getPre()->setNext(node->getNext());
    node->getNext()->setPre(node->getPre());
}

template<typename Key, typename Value>
void LruCache<Key, Value>::updateExitingNode(NodePtr node, const Value& value) {
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
void LruCache<Key, Value>::addNewNode(Key key, const Value& value) {
    if (nodeHash_.size() >= this->capacity_) {
        evictLeastAccessNode();
    }
    NodePtr newNode = std::make_shared<LruNodeType>(key, value);
    insertNode(newNode);
    nodeHash_[key] = newNode;
}
