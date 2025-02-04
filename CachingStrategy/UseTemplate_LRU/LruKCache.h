#pragma once
#include"LruCache.h"

template<typename Key,typename Value>
class LruKCache :public LruCache<Key,Value> {
private:
	unsigned int k_;
	unique_ptr<LruCache<Key, Value>> historyList_;
public:
	LruKCache(unsigned int capacity, unsigned int historyCapacity, unsigned int k)
		:LruCache<Key, Value>{ capacity }, 
		k_{ k }, historyList_{make_unique<LruCache<Key,Value>>(historyCapacity)}
	{}
	~LruKCache() = default;

	Value get(Key key) {
		if (this->historyList_->isExit(key)) {
			if(isGreaterThanK(key))
				putIntoLruCache(key, historyList_->get(key));
		}

		return LruCache<Key, Value>::get(key);
	}

	void put(Key key, Value& value) {
		if (this->historyList_->isExit(key) == false) {
			this->historyList_->put(key, value);
			return;
		}
		if (isGreaterThanK(key))
			putIntoLruCache(key, historyList_->get(key));
	}

	bool remove(Key key) {
		return LruCache<Key, Value>::remove(key);
	}

private:
	void putIntoLruCache(Key key, Value value) {
		historyList_->remove(key);
		LruCache<Key, Value>::put(key, value);
	}
	bool isGreaterThanK(Key key) {
		shared_ptr<LruNode<Key, Value>> node = this->historyList_->getNode(key);
		node->increaseAccessCount();
		size_t accessCount = node->getAccessCount();
		this->historyList_->moveToRecentPosition(node);
		if (accessCount >= this->k_)
			return true;
		else
			return false;
	}
};