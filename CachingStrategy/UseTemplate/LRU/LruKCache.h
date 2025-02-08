#pragma once
#include "LruCache.h"

template<typename Key, typename Value>
class LruKCache : public LruCache<Key, Value> {
private:
    unsigned int k_;
    std::unique_ptr<LruCache<Key, Value>> historyList_;

public:
    LruKCache(unsigned int capacity, unsigned int historyCapacity, unsigned int k);
    ~LruKCache() = default;

    optional<Value> get(const Key&);
    void put(const Key&,const Value&);
    bool remove(const Key&);

private:
    void putIntoLruCache(const Key&,const Value&);
    bool isGreaterThanK(const Key&);
};

template<typename Key, typename Value>
LruKCache<Key, Value>::LruKCache(unsigned int capacity, unsigned int historyCapacity, unsigned int k)
    : LruCache<Key, Value>{ capacity },
    k_{ k }, historyList_{ std::make_unique<LruCache<Key, Value>>(historyCapacity) }
{}

template<typename Key, typename Value>
optional<Value> LruKCache<Key, Value>::get(const Key& key) {
    if (this->historyList_->isExit(key)) {
        if (isGreaterThanK(key))
            putIntoLruCache(key, historyList_->get(key).value());
    }

    return LruCache<Key, Value>::get(key);
}

template<typename Key, typename Value>
void LruKCache<Key, Value>::put(const Key& key,const Value& value) {
    if (this->historyList_->isExit(key) == false) {
        this->historyList_->put(key, value);
        return;
    }
    if (isGreaterThanK(key))
        putIntoLruCache(key, historyList_->get(key).value());
}

template<typename Key, typename Value>
bool LruKCache<Key, Value>::remove(const Key& key) {
    return LruCache<Key, Value>::remove(key);
}

template<typename Key, typename Value>
void LruKCache<Key, Value>::putIntoLruCache(const Key& key,const Value& value) {
    historyList_->remove(key);
    LruCache<Key, Value>::put(key, value);
}

template<typename Key, typename Value>
bool LruKCache<Key, Value>::isGreaterThanK(const Key& key) {
    std::shared_ptr<LruNode<Key, Value>> node = this->historyList_->getNode(key);
    node->increaseAccessCount();
    size_t accessCount = node->getAccessCount();
    this->historyList_->moveToRecentPosition(node);
    return accessCount >= this->k_;
}
