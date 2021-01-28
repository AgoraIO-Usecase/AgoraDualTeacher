//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <AgoraBase.h>
#include <string>
#include <list>

namespace agora {
	namespace util {

class StringImpl : public IString
{
public:
	StringImpl(const char* str)
		:impl_(str)
	{}
	StringImpl(const std::string& str)
		:impl_(str)
	{}
	StringImpl(std::string&& str)
		:impl_(std::move(str))
	{}
	virtual bool empty() const override { return impl_.empty(); }
	virtual const char* c_str() override { return impl_.c_str(); }
	virtual const char* data() override { return impl_.data(); }
	virtual size_t length() override { return impl_.length(); }
    virtual IString* clone() override { return new StringImpl(impl_); }
    virtual void release() override { delete this; }
private:
	std::string impl_;
};

template<class T>
class ListImpl : public IContainer
{
public:
    void push_back(T&& e) {
        container_.push_back(std::move(e));
    }
    virtual IIterator* begin() override;
    virtual size_t size() const override { return container_.size(); }
    virtual void release() override { delete this; }
private:
    std::list<T> container_;
};

template<class T>
class IteratorImpl : public IIterator
{
    using container_type = std::list<T>;
    using iterator_type = typename container_type::iterator;
public:
    IteratorImpl(container_type& c)
        :container_(c)
        , iterator_(c.begin())
    {}
    virtual void* current() override {
        if (valid())
            return &(*iterator_);
        return nullptr;
    }
    virtual const void* const_current() const override {
        if (valid())
            return &(*iterator_);
        return nullptr;
    }
    virtual bool next() override {
        if (valid())
            ++iterator_;
        return valid();
    }
    virtual void release() override { delete this; }
private:
    bool valid() const { return iterator_ != container_.end(); }
private:
    container_type& container_;
    iterator_type iterator_;
};

template<class T>
inline IIterator* ListImpl<T>::begin() { return new IteratorImpl<T>(container_); }

template<class T>
struct AutoObjectHolder
{
    T* ptr_;
    AutoObjectHolder(T* p)
        :ptr_(p)
    {}
    ~AutoObjectHolder()
    {
        if (ptr_)
            delete ptr_;
    }
    T* release()
    {
        T* tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }
};

}}
