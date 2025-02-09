#pragma once
#include<memory>
using namespace std;

template<typename Key, typename Value>
class LfuNode {
private:
	Key key_;
	Value value_;
	unsigned int freq_;
	shared_ptr<LfuNode<Key, Value>> pre_;
	shared_ptr<LfuNode<Key, Value>> next_;

public:
	LfuNode() = delete;
	LfuNode(Key key, Value value)
		: key_{ key }
		, value_{ value }
		, freq_{ 1 } 
		, pre_{ nullptr }
		, next_{ nullptr }
	{}
	const Key getKey() { return this->key_; }
	void setKey(const Key& key) { this->key_ = key; }
	Value getValue() { return this->value_; }
	void setValue(const Value& value) { this->value_ = value; }
	unsigned int getFrequency() { return freq_; }
	void setFrequency(const unsigned int& freq) { this->freq_ = freq; }
	void increaseFrequency() { this->freq_++; }
	void decreaseFrequency() { this->freq_--; }
	shared_ptr<LfuNode<Key, Value>> getPre() { return this->pre_; }
	void setPre(const shared_ptr<LfuNode<Key, Value>>& node) { this->pre_ = node; }
	shared_ptr<LfuNode<Key, Value>> getNext() { return this->next_; }
	void setNext(const shared_ptr<LfuNode<Key, Value>>& node) { this->next_ = node; }


};
