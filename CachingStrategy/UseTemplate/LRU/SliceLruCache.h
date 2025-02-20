#pragma once
#include"LruCache.h"

template<typename Key, typename Value>
class SliceLruCache :public ICachePolicy<Key,Value>{
private:
	unsigned int sliceNum_;
	unsigned int capacity_;
	vector<unique_ptr<LruCache<Key, Value>>>  sliceLruCache_;
public:
	SliceLruCache(unsigned int sliceNum, unsigned int capacity) : sliceNum_{ sliceNum }, capacity_{ capacity } {
		initialize();
	}
	~SliceLruCache() = default;

	bool isExists(const Key& key) {
		size_t sliceIndex = hashFun(key) % this->sliceNum_;
		return this->sliceLruCache_[sliceIndex]->isExists(key);
	}

	optional<Value> get(const Key& key) {
		size_t sliceIndex = hashFun(key) % this->sliceNum_;
		return this->sliceLruCache_[sliceIndex]->get(key);
	}

	void put(const Key& key,const Value& value) {
		size_t sliceIndex = hashFun(key) % this->sliceNum_;
		this->sliceLruCache_[sliceIndex]->put(key, value);
	}

	bool remove(const Key& key) {
		size_t sliceIndex = hashFun(key) % this->sliceNum_;
		return this->sliceLruCache_[sliceIndex]->remove(key);
	}

private:
	void initialize() {
		unsigned int sliceCapacity= static_cast<unsigned int>(ceil(static_cast<double>(this->capacity_)/ static_cast<double>(this->sliceNum_)));
		for (unsigned int i = 0; i < this->sliceNum_; i++) {
			sliceLruCache_.emplace_back(make_unique<LruCache<Key, Value>>(sliceCapacity));
		}
	}
	size_t hashFun(Key key) {
		hash<Key> hash;
		return hash(key);
	}
};