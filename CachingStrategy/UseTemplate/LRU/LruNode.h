#pragma once
#include<memory>
using namespace std;

template<typename Key,typename Value>
class LruNode {
private:
	Key key_;
	Value value_;
	size_t accessCount_;
	shared_ptr<LruNode> pre_;
	shared_ptr<LruNode> next_;

public:
	LruNode(Key key, Value value)
		: key_{ key }
		, value_{ value }
		, accessCount_{ 1 } //once something is placed in there, it is considered to have been visited
							//therefore the initial value is set to 1
		, pre_{ nullptr }
		, next_{ nullptr }
	{}
	Key getKey() { return this->key_; }
	void setKey(Key key) { this->key_ = key; }
	Value getValue() { return this->value_; }
	void setValue(Value value) {  this->value_ = value; }
	size_t getAccessCount() { return accessCount_; }
	void increaseAccessCount() { this->accessCount_++; }
	shared_ptr<LruNode> getPre(){ return this->pre_; }
	void setPre(shared_ptr<LruNode> node) { this->pre_ = node; }
	shared_ptr<LruNode> getNext(){ return this->next_; }
	void setNext(shared_ptr<LruNode> node) { this->next_ = node; }

	//friend class LruCache<Key, Value>;
};