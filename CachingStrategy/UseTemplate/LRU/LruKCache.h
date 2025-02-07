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

    Value get(Key key);
    void put(Key key, Value& value);
    bool remove(Key key);

private:
    void putIntoLruCache(Key key, Value value);
    bool isGreaterThanK(Key key);
};

template<typename Key, typename Value>
LruKCache<Key, Value>::LruKCache(unsigned int capacity, unsigned int historyCapacity, unsigned int k)
    : LruCache<Key, Value>{ capacity },
    k_{ k }, historyList_{ std::make_unique<LruCache<Key, Value>>(historyCapacity) }
{}

template<typename Key, typename Value>
Value LruKCache<Key, Value>::get(Key key) {
    if (this->historyList_->isExit(key)) {
        if (isGreaterThanK(key))
            putIntoLruCache(key, historyList_->get(key));
    }

    return LruCache<Key, Value>::get(key);
}

template<typename Key, typename Value>
void LruKCache<Key, Value>::put(Key key, Value& value) {
    if (this->historyList_->isExit(key) == false) {
        this->historyList_->put(key, value);
        return;
    }
    if (isGreaterThanK(key))
        putIntoLruCache(key, historyList_->get(key));
}

template<typename Key, typename Value>
bool LruKCache<Key, Value>::remove(Key key) {
    return LruCache<Key, Value>::remove(key);
}

template<typename Key, typename Value>
void LruKCache<Key, Value>::putIntoLruCache(Key key, Value value) {
    historyList_->remove(key);
    LruCache<Key, Value>::put(key, value);
}

template<typename Key, typename Value>
bool LruKCache<Key, Value>::isGreaterThanK(Key key) {
    std::shared_ptr<LruNode<Key, Value>> node = this->historyList_->getNode(key);
    node->increaseAccessCount();
    size_t accessCount = node->getAccessCount();
    this->historyList_->moveToRecentPosition(node);
    return accessCount >= this->k_;
}
