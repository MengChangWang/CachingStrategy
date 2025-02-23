#pragma once
#include "../ICachePolicy.h"
#include "ArcNodeList.h"
#include <unordered_map>
#include <mutex>
#include <optional>

template<typename Key,typename Value>
class ArcLru:public ICachePolicy<Key,Value>
{
private:
	using Node = ArcNode<Key, Value>;
	using NodePtr = shared_ptr<Node>;
	using DList = ArcNodeList<Key, Value>;
	using NodeHash = unordered_map<Key,NodePtr>;
private:
	DList  lruList_;
	DList ghostList_;
	NodeHash nodeHash_;
	NodeHash ghostHash_;
	size_t capacity_;
	mutex mutex_;
	
public:
	ArcLru() = delete;
	ArcLru(const size_t& capacity);
	optional<Value> get(const Key& key);
	optional<Value> get(const Key& key,bool& flag);
	void put(const Key& key, const Value& value);
	bool isExists(const Key& key);
	bool remove(const Key& key);
	void increaseCapacity();
	void decreaseCapacity();
	bool checkGhost(const Key& key);
private:
	void moveToFront(const NodePtr node);
	void insertIntoGhost(const NodePtr node);
	void evictLeastNode();
	void insertNewNode(const Key& key, const Value& value);
	
};

template<typename Key, typename Value>
ArcLru<Key, Value>::ArcLru(const size_t& capacity):
	capacity_{capacity}
{
}

template<typename Key, typename Value>
optional<Value> ArcLru<Key, Value>::get(const Key& key)
{
	lock_guard<mutex> lock{ this->mutex_ };
	auto hash_it = this->nodeHash_.find(key);
	if (hash_it == this->nodeHash_.end()) return nullopt;
	NodePtr node = hash_it->second;

	node->increaseAccessCount();
	moveToFront(node);
	
	return node->getValue();
}

template<typename Key, typename Value>
inline optional<Value> ArcLru<Key, Value>::get(const Key& key, bool& flag)
{
	lock_guard<mutex> lock{ this->mutex_ };
	auto hash_it = this->nodeHash_.find(key);
	if (hash_it == this->nodeHash_.end()) {
		flag = false;
		return nullopt;
	}
	NodePtr node = hash_it->second;
	node->increaseAccessCount();
	moveToFront(node);
	flag = true;

	return node->getValue();
}

template<typename Key, typename Value>
void ArcLru<Key, Value>::put(const Key& key, const Value& value)
{
	lock_guard<mutex> lock{ this->mutex_ };
	auto it = this->nodeHash_.find(key);

	if (it != this->nodeHash_.end()) {
		it->second->increaseAccessCount();
		it->second->setValue(value);
		return;
	}

	if (this->capacity_ <= this->nodeHash_.size()) {
		evictLeastNode();
	}

	insertNewNode(key, value);
}

template<typename Key, typename Value>
bool ArcLru<Key, Value>::isExists(const Key& key)
{
	auto it = this->nodeHash_.find(key);
	return it != this->nodeHash_.end();
}


template<typename Key, typename Value>
bool ArcLru<Key, Value>::remove(const Key& key)
{
	auto it = this->nodeHash_.find(key);
	if (it == this->nodeHash_.end()) return false;
	this->lruList_.removeNode(it->second);
	this->nodeHash_.erase(it);
	return true;
}

template<typename Key, typename Value>
void ArcLru<Key, Value>::increaseCapacity()
{
	this->capacity_++;
}

template<typename Key, typename Value>
inline void ArcLru<Key, Value>::decreaseCapacity()
{
	if (this->capacity_ >= 1)
		this->capacity_--;
}

template<typename Key, typename Value>
void ArcLru<Key, Value>::moveToFront(const NodePtr node)
{
	this->lruList_.removeNode(node);
	this->lruList_.insertNode(node);
}

template<typename Key, typename Value>
void ArcLru<Key, Value>::insertIntoGhost(const NodePtr node)
{
	if (this->ghostHash_.size() >= this->capacity_) {
		NodePtr leastGhostNode = this->ghostList_.getLeastNode();
		Key key = leastGhostNode->getKey();
		this->ghostHash_.erase(key);
		this->ghostList_.removeNode(leastGhostNode);
	}
	this->ghostHash_.emplace(node->getKey(),node);
	this->ghostList_.insertNode(node);
}

template<typename Key, typename Value>
void ArcLru<Key, Value>::evictLeastNode()
{
	NodePtr leastNode = this->lruList_.getLeastNode();
	insertIntoGhost(leastNode);

	this->lruList_.removeNode(leastNode);
	this->nodeHash_.erase(leastNode->getKey());
}

template<typename Key, typename Value>
void ArcLru<Key, Value>::insertNewNode(const Key& key, const Value& value)
{
	NodePtr newNode = make_shared<Node>(key, value);
	this->nodeHash_.emplace(key,newNode);
	this->lruList_.insertNode(newNode);
}

template<typename Key, typename Value>
bool ArcLru<Key, Value>::checkGhost(const Key& key)
{
	auto it = this->ghostHash_.find(key);
	if (it == this->ghostHash_.end()) return false;
	
	this->ghostList_.removeNode(it->second);
	this->ghostHash_.erase(it);
	
	return true;
}
