#pragma once
#include <optional>
using namespace std;

template<typename Key,typename Value>
class ICachePolicy {
public:
	virtual ~ICachePolicy() {}
	virtual void put(const Key&,const Value&) = 0;
	virtual optional<Value> get(const Key&) = 0;
	virtual bool remove(const Key&) = 0;
	virtual bool isExit(const Key&) = 0;
};