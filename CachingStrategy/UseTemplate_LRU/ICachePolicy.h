#pragma once
using namespace std;

template<typename Key,typename Value>
class ICachePolicy {
public:
	virtual ~ICachePolicy() {}
	virtual void put(Key, Value) = 0;
	virtual Value get(Key) = 0;
	virtual bool remove(Key) = 0;
	virtual bool isExit(Key) = 0;
};