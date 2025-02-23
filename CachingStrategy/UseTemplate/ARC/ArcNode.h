#pragma once
#include<memory>
using namespace std;

template <typename Key,typename Value>
class ArcNode {
private:
	Key key_;
	Value value_;
	size_t accessCount_;

	shared_ptr< ArcNode<Key, Value>> pre_;
	shared_ptr< ArcNode<Key, Value>> next_;
public:
	ArcNode() = delete;
	ArcNode(const Key&, const Value&);
	~ArcNode() = default;

	Key getKey();
	Value getValue();
	void setValue(const Value&);
	size_t getAccessCount();
	void increaseAccessCount();
	void decreaseAccessCount();
	void setAccessCount(const size_t&);

	shared_ptr<ArcNode<Key, Value>> getPre() { return this->pre_; }
	void setPre(const shared_ptr<ArcNode<Key, Value>>& node) { this->pre_ = node; }
	shared_ptr<ArcNode<Key, Value>> getNext() { return this->next_; }
	void setNext(const shared_ptr<ArcNode<Key, Value>>& node) { this->next_ = node; }
};

template<typename Key, typename Value>
ArcNode<Key, Value>::ArcNode(const Key& key, const Value& value):
	key_{key},value_{value},pre_{nullptr},next_{nullptr},accessCount_{1}
{
}

template<typename Key, typename Value>
Key ArcNode<Key, Value>::getKey()
{
	return this->key_;
}

template<typename Key, typename Value>
Value ArcNode<Key, Value>::getValue()
{
	return this->value_;
}

template<typename Key, typename Value>
void ArcNode<Key, Value>::setValue(const Value& value)
{
	this->value_ = value;
}

template<typename Key, typename Value>
size_t ArcNode<Key, Value>::getAccessCount()
{
	return this->accessCount_;
}

template<typename Key, typename Value>
void ArcNode<Key, Value>::increaseAccessCount()
{
	this->accessCount_++;
}

template<typename Key, typename Value>
void ArcNode<Key, Value>::decreaseAccessCount()
{
	this->accessCount_--;
}

template<typename Key, typename Value>
void ArcNode<Key, Value>::setAccessCount(const size_t& times)
{
	this->accessCount_ = times;
}

