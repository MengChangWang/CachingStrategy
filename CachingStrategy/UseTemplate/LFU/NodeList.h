#pragma once
#include"LfuNode.h"

template<typename Key,typename Value>
class NodeList {
public:
	using Node = LfuNode<Key, Value>;
	using NodePtr = shared_ptr<Node>;

	NodePtr dummyHead_;
	NodePtr dummyTail_;
public:
	NodeList();
	~NodeList() = default;
	void insertNode(const NodePtr&);
	void removeNode(const NodePtr&);
	NodePtr getLeastNode();
	bool isEmpty();
};

template<typename Key,typename Value>
NodeList<Key, Value>::NodeList() {
	this->dummyHead_ = make_shared<Node>(Key(), Value());
	this->dummyTail_ = make_shared<Node>(Key(), Value());
	this->dummyHead_->setNext(this->dummyTail_);
	this->dummyTail_->setPre(this->dummyHead_);
}

template<typename Key,typename Value>
void NodeList<Key, Value>::insertNode(const NodePtr& node) {
	node->setNext(this->dummyHead_->getNext());
	node->setPre(this->dummyHead_);

	this->dummyHead_->getNext()->setPre(node);
	this->dummyHead_->setNext(node);
}

template<typename Key,typename Value>
void NodeList<Key, Value>::removeNode(const NodePtr& node) {
	node->getPre()->setNext(node->getNext());
	node->getNext()->setPre(node->getPre());
}


//this may return dummyHead
//it is need to optimize the logic of function
template<typename Key, typename Value>
typename NodeList<Key, Value>::NodePtr NodeList<Key, Value>::getLeastNode() {
	return this->dummyTail_->getPre();
}

template<typename Key,typename Value>
bool NodeList<Key, Value>::isEmpty() {
	if (this->dummyHead_->getNext() == this->dummyTail_)
		return true;
	else
		return false;
}
