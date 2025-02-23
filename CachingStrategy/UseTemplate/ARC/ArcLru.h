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
	DList  list_;
	NodeHash nodeHash_;
	size_t capacity_;
	mutex mutex_;
public:
	ArcLru() = delete;
	ArcLru(const size_t& capacity);
	optional<Value> get(const Key& key);
	void put(const Key& key, const Value& value);
	bool isExists(const Key& key);
	bool remove(const Key& key);
private:
	void moveToFront(const NodePtr node);
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
	this->list_.removeNode(it->second);
	this->nodeHash_.erase(it);
	return true;
}

template<typename Key, typename Value>
void ArcLru<Key, Value>::moveToFront(const NodePtr node)
{
	this->list_.removeNode(node);
	this->list_.insertNode(node);
}

template<typename Key, typename Value>
void ArcLru<Key, Value>::evictLeastNode()
{
	NodePtr leastNode = this->list_.getLeastNode();
	this->list_.removeNode(leastNode);
	this->nodeHash_.erase(leastNode->getKey());

}

template<typename Key, typename Value>
void ArcLru<Key, Value>::insertNewNode(const Key& key, const Value& value)
{
	NodePtr newNode = make_shared<Node>(key, value);
	this->nodeHash_.emplace(key,newNode);
	this->list_.insertNode(newNode);
}
