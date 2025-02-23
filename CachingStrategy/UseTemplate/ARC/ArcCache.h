#pragma once
#include"../ICachePolicy.h"
#include "ArcLfu.h"
#include "ArcLru.h"

template <typename Key,typename Value>
class ArcCache :public ICachePolicy<Key,Value> {
private:
	using LFU = ArcLfu<Key, Value>;
	using LRU = ArcLru<Key, Value>;
	using pLFU = unique_ptr<ArcLfu<Key, Value>>;
	using pLRU = unique_ptr<ArcLru<Key, Value>>;
private:
	pLFU lfu_;
	pLRU lru_;
	unsigned int capacity_;
	unsigned int threshold_;
public:
	ArcCache() = delete;
	ArcCache(const unsigned int& capacity) :
		capacity_(capacity), 
		lfu_(make_unique<LFU>(capacity>>1)), lru_(make_unique<LRU>(capacity>>1))
	{};
	~ArcCache() = default;
	optional<Value> get(const Key& key);
	void put(const Key& key, const Value& value);
	bool isExists(const Key& key);
	bool remove(const Key& key);
private:
	bool checkGhost(const Key& key);
	void transfer(const Key& key, const Value& value);
};


template<typename Key, typename Value>
inline optional<Value> ArcCache<Key, Value>::get(const Key& key)
{
	if (checkGhost(key)) return nullopt;
	optional<Value> value = optional<Value>();
	bool isOver = false;
	if (this->lru_->isExists(key)) {
		value = this->lru_->get(key,isOver);
		if (isOver) {
			transfer(key, value.value());
		}
	}
	else if (this->lfu_->isExists(key)) {
		value = this->lfu_->get(key);
	}
	return value;
}

template<typename Key, typename Value>
void ArcCache<Key, Value>::put(const Key& key, const Value& value)
{
	checkGhost(key);
	if (this->lru_->isExists(key)) {
		transfer(key, value);
	}
	else if (this->lfu_->isExists(key)) {
		this->lfu_->put(key, value);
	}
	else {
		this->lru_->put(key, value);
	}
}

template<typename Key, typename Value>
bool ArcCache<Key, Value>::isExists(const Key& key)
{
	if (this->lfu_->isExists(key))return true;
	if (this->lru_->isExists(key))return true;
	return false;
}

template<typename Key, typename Value>
bool ArcCache<Key, Value>::remove(const Key& key)
{
	if (this->lfu_->isExists(key)) {
		this->lfu_->remove(key);
		return true;
	}
	if (this->lru_->isExists(key)) {
		this->lru_->remove(key);
		return true;
	}
	return false;
}

template<typename Key, typename Value>
bool ArcCache<Key, Value>::checkGhost(const Key& key)
{
	if (this->lru_->checkGhost(key)) {
		this->lru_->increaseCapacity();
		this->lfu_->decreaseCapacity();
		return true;
	}
	else if (this->lfu_->checkGhost(key)) {
		this->lfu_->increaseCapacity();
		this->lru_->decreaseCapacity();
		return true;
	}
	return false;
}

template<typename Key, typename Value>
void ArcCache<Key, Value>::transfer(const Key& key, const Value& value)
{
	this->lru_->remove(key);
	this->lfu_->put(key, value);
}
