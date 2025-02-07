#pragma once
#include<unordered_map>
#include<mutex>
#include"LruNode.h"
#include"UseTemplate\ICachePolicy.h"

template<typename Key,typename Value>
class LruCache :public ICachePolicy<Key,Value>{
public:
	using LruNodeType = LruNode<Key, Value>;
	using NodePtr = shared_ptr<LruNodeType>;
	using NodeHash = unordered_map<Key, NodePtr>;

private:
	size_t capacity_;
	mutex mutex_;
	NodeHash nodeHash_;
	NodePtr dummyHead_;
	NodePtr dummyTail_;

public:
	LruCache(unsigned int capacity) : capacity_(capacity) {
		initializeList();
	}

	~LruCache() override = default;

	void put(Key key, Value& value) override {
		if (this->capacity_ <= 0) return;
		lock_guard<mutex> lock(mutex_);
		auto it = nodeHash_.find(key);
		if (it != nodeHash_.end()) {
			updateExitingNode(it->second,value);//update the value in the node and adjust the node's position in the linked list
			return;
		}
		addNewNode(key, value);
	}

	bool isExit(Key key) override {
		auto it = nodeHash_.find(key);
		if (it == nodeHash_.end()) {
			return false;
		}
		return true;
	}

	Value get(Key key) override{
		lock_guard<mutex> lock{ mutex_ };
		Value value{};
		if (isExit(key) == false) return value;
		else {
			NodePtr nodeAddress = nodeHash_[key];
			value = nodeAddress->getValue();
			nodeAddress->increaseAccessCount();
			moveToRecentPosition(nodeAddress);//adjust the node's position in the linked list
			return value;
		}	
	}
	
	//to delete the specified element
	bool remove(Key key) override {
		lock_guard<mutex> lock{ mutex_ };
		if (isExit(key) == false) return false;
		removeNode(nodeHash_[key]);
		nodeHash_.erase(key);
		return true;
	}

	NodePtr getNode(Key key) {
		return nodeHash_[key];
	}

	void moveToRecentPosition(NodePtr node) {
		removeNode(node);
		insertNode(node);
	}

private:
	void initializeList() {
		dummyHead_ = make_shared<LruNodeType>(Key(), Value());
		dummyTail_ = make_shared<LruNodeType>(Key(), Value());
		dummyHead_->setNext(dummyTail_);
		dummyTail_->setPre(dummyHead_);
	}

	void insertNode(NodePtr node) {
		node->setNext(dummyTail_);
		node->setPre(dummyTail_->getPre());

		dummyTail_->getPre()->setNext(node);
		dummyTail_->setPre(node);
	}

	void removeNode(NodePtr node) {
		node->getPre()->setNext(node->getNext());
		node->getNext()->setPre(node->getPre());
	}


	//before using this function, it is necessary to ensure that the node already exists
	void updateExitingNode(NodePtr node,const Value& value) {
		node->setValue(value);
		node->increaseAccessCount();
		moveToRecentPosition(node);
	}

	void evictLeastAccessNode() {
		NodePtr leastNode = dummyHead_->getNext();
		removeNode(leastNode);
		nodeHash_.erase(leastNode->getKey());
	}

	void addNewNode(Key key, const Value& value) {
		if (nodeHash_.size() >= this->capacity_) {
			evictLeastAccessNode();
		}
		NodePtr newNode = make_shared<LruNodeType>(key, value);
		insertNode(newNode);
		nodeHash_[key] = newNode;
	}

};