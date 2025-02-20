#pragma once
#include<unordered_set>
#include<queue>
using namespace std;
template<typename T>
class unique_min_heap {
private:
	unordered_set<T> set_;
	priority_queue<T, vector<T>, greater<T>> minHeap_;
public:
	unsigned int size();
	bool empty();
	T top();
	T pop();
	void push(const T&);
};

template<typename T>
unsigned int unique_min_heap<T>::size()
{
	return this->set_.size();
}

template<typename T>
bool unique_min_heap<T>::empty()
{
	return this->set_.empty();
}

template<typename T>
T unique_min_heap<T>::top()
{
	return this->minHeap_.top;
}

template<typename T>
T unique_min_heap<T>::pop()
{
	T t = this->minHeap_.pop();
	this->set_.erase(t);
}

template<typename T>
inline void unique_min_heap<T>::push(const T& t)
{
	if (this->set_.insert(t).second) {
		this->minHeap_.push(t);
	}
}
