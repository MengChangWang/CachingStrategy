#pragma once
#include "ArcNode.h"
template<typename Key, typename Value>
class ArcNodeList {
public:
	using Node = ArcNode<Key, Value>;
	using NodePtr = shared_ptr<Node>;

	NodePtr dummyHead_;                        
	NodePtr dummyTail_;
public:
	ArcNodeList();
	~ArcNodeList() = default;
	void insertNode(const NodePtr&);
	void removeNode(const NodePtr&);
	NodePtr getLeastNode();
	bool isEmpty();
};

template<typename Key, typename Value>
ArcNodeList<Key, Value>::ArcNodeList() {
	this->dummyHead_ = make_shared<Node>(Key(), Value());
	this->dummyTail_ = make_shared<Node>(Key(), Value());
	this->dummyHead_->setNext(this->dummyTail_);
	this->dummyTail_->setPre(this->dummyHead_);
}

template<typename Key, typename Value>
void ArcNodeList<Key, Value>::insertNode(const NodePtr& node) {
	node->setNext(this->dummyHead_->getNext());
	node->setPre(this->dummyHead_);

	this->dummyHead_->getNext()->setPre(node);
	this->dummyHead_->setNext(node);
}

template<typename Key, typename Value>
void ArcNodeList<Key, Value>::removeNode(const NodePtr& node) {
	node->getPre()->setNext(node->getNext());
	node->getNext()->setPre(node->getPre());
}


//this may return dummyHead
//it is need to optimize the logic of function
template<typename Key, typename Value>
typename ArcNodeList<Key, Value>::NodePtr ArcNodeList<Key, Value>::getLeastNode() {
	NodePtr leastNode = this->dummyTail_->getPre();
	if (leastNode == this->dummyHead_) {
		exit(111111);
	}
	return leastNode;
}

template<typename Key, typename Value>
bool ArcNodeList<Key, Value>::isEmpty() {
	if (this->dummyHead_->getNext() == this->dummyTail_)
		return true;
	else
		return false;
}
