#pragma once
#include"LfuNode.h"

template<typename Key,typename Value>
class NodeList {
private:
	using Node = LfuNode<Key, Value>;
	using NodePtr = shared_ptr<Node>;

	NodePtr dummyHead_;
	NodePtr dummyTail_;
public:
	NodeList();
	~NodeList() = default;
	void insertNode(NodePtr&);
	void removeNode(NodePtr&);
	NodePtr getLeastNode();
	bool isEmpty();
};

template<typename Key,typename Value>
NodeList<Key, Value>::NodeList() {
	this->dummyHead_->setNext(this->dummyTail_);
	this->dummyTail_->setPre(this->dummyHead_);
}

template<typename Key,typename Value>
void NodeList<Key, Value>::insertNode(NodePtr& node) {
	node->setNext(this->dummyHead_->getNext());
	node->setPre(this->dummyHead_);

	this->dummyHead_->getNext()->setPre(node);
	this->dummyHead_->setNext(node);
}

template<typename Key,typename Value>
void NodeList<Key, Value>::removeNode(NodePtr& node) {
	node->getPre()->setNext(node->getNext());
	node->getNext()->setPre(node->getPre());
}

template<typename Key, typename Value>
NodePtr NodeList<Key, Value>::getLeastNode() {
	return this->dummyTail_->getNext();
}

template<typename Key,typename Value>
bool NodeList<Key, Value>::isEmpty() {
	if (this->dummyHead_->getNext() == this->dummyTail_)
		return true;
	else
		return false;
}
