#pragma once
#include<memory>
using namespace std;

template<typename Key,typename Value>
class LruNode {
private:
	Key key_;
	Value value_;
	unsigned int accessCount_;
	shared_ptr<LruNode<Key,Value>> pre_;
	shared_ptr<LruNode<Key, Value>> next_;

public:
	LruNode() = delete;
	LruNode(Key key, Value value)
		: key_{ key }
		, value_{ value }
		, accessCount_{ 1 } //once something is placed in there, it is considered to have been visited
							//therefore the initial value is set to 1
		, pre_{ nullptr }
		, next_{ nullptr }
	{}
	const Key getKey() { return this->key_; }
	void setKey(const Key& key) { this->key_ = key; }
	const Value getValue() { return this->value_; }
	void setValue(const Value& value) { this->value_ = value; }
	const unsigned int getAccessCount() { return accessCount_; }
	void increaseAccessCount() { this->accessCount_++; }
	shared_ptr<LruNode<Key, Value>> getPre(){ return this->pre_; }
	void setPre(const shared_ptr<LruNode<Key, Value>>& node) { this->pre_ = node; }
	shared_ptr<LruNode<Key, Value>> getNext(){ return this->next_; }
	void setNext(const shared_ptr<LruNode<Key, Value>>& node) { this->next_ = node; }

	//friend class LruCache<Key, Value>;
};