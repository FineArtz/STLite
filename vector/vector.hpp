#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include "exceptions.hpp"

#include <climits>
#include <cstddef>
#include <cstdlib>

namespace sjtu {

template <class I>
struct iterator_traits{
    using iterator_category = typename I::iterator_category;
    using value_type        = typename I::value_type;
    using difference_type   = typename I::difference_type;
    using pointer           = typename I::pointer;
    using reference         = typename I::reference;
};
template <class T>
struct iterator_traits<T*>{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = T *;
    using reference         = T &;
};
template <class T>
struct iterator_traits<const T*>{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = T *;
    using reference         = T &;
};

/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template<typename T>
class vector{
public:
    using value_type        = T;
    using pointer           = T *;
    using reference         = T &;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;

public:
    class iterator;
    class const_iterator;
    friend class iterator;
    friend class const_iterator;
protected:
    iterator start, finish, end_of_storage;

public:
	class const_iterator;
	class iterator{
	    friend class const_iterator;
	    friend class vector;
    public:
	    using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;
	private:
		T *_ptr;
	    vector *_container;
	public:
        iterator(T *_p = nullptr, vector *_c = nullptr) : _ptr(_p), _container(_c){}
        iterator(const iterator &i) : _ptr(i._ptr), _container(i._container) {}
        iterator(const const_iterator &ci) : _ptr(ci._ptr), _container(ci._container) {}

		iterator operator +(const int &n) const {
		    return iterator(_ptr + n, _container);
		}
		iterator operator -(const int &n) const {
			return iterator(_ptr - n, _container);
		}
		difference_type operator -(const iterator &rhs) const {
            if (_container != rhs._container)
                throw invalid_iterator();
            return _ptr - rhs._ptr;
		}
		iterator operator +=(const int &n) {
			_ptr += n;
			return iterator(_ptr, _container);
		}
		iterator operator -=(const int &n) {
			_ptr -= n;
			return iterator(_ptr, _container);
		}
		iterator operator ++(int){
            T *_p = _ptr;
            ++_ptr;
            return iterator(_p, _container);
		}
		iterator &operator ++(){
		    ++_ptr;
            return *this;
		}
		iterator operator --(int){
            T *_p = _ptr;
            --_ptr;
            return iterator(_p, _container);
		}
		iterator& operator --(){
		    --_ptr;
            return *this;
		}
		T& operator *() const {
            return *_ptr;
		}
		bool operator ==(const iterator &rhs) const {
            return _ptr == rhs._ptr;
		}
		bool operator ==(const const_iterator &rhs) const {
            return _ptr == rhs._ptr;
		}
		bool operator !=(const iterator &rhs) const {
            return _ptr != rhs._ptr;
		}
		bool operator !=(const const_iterator &rhs) const {
            return _ptr != rhs._ptr;
		}
	};
    //end of class iterator

	class const_iterator {
	    friend class iterator;
	    friend class vector;
    public:
	    using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;
	private:
		T *_ptr;
	    vector *_container;
	public:
        const_iterator(T *_p = nullptr, vector *_c = nullptr) : _ptr(_p), _container(_c){}
        const_iterator(const iterator &i) : _ptr(i._ptr), _container(i._container) {}
        const_iterator(const const_iterator &ci) : _ptr(ci._ptr), _container(ci._container) {}

		const_iterator operator +(const int &n) const {
		    return const_iterator(_ptr + n, _container);
		}
		const_iterator operator -(const int &n) const {
			return const_iterator(_ptr - n, _container);
		}
		difference_type operator -(const const_iterator &rhs) const {
            if (_container != rhs._container)
                throw invalid_iterator();
            return _ptr - rhs._ptr;;
		}
		const_iterator operator +=(const int &n) {
			_ptr += n;
			return const_iterator(_ptr, _container);
		}
		const_iterator operator -=(const int &n) {
			_ptr -= n;
			return const_iterator(_ptr, _container);
		}
		const_iterator operator ++(int){
            T *_p = _ptr;
            ++_ptr;
            return const_iterator(_p, _container);
		}
		const_iterator &operator ++(){
		    ++_ptr;
            return *this;
		}
		const_iterator operator --(int){
            T *_p = _ptr;
            --_ptr;
            return const_iterator(_p, _container);
		}
		const_iterator& operator --(){
		    --_ptr;
            return *this;
		}
		T& operator *() const {
            return *_ptr;
		}
		bool operator ==(const iterator &rhs) const {
            return _ptr == rhs._ptr;
		}
		bool operator ==(const const_iterator &rhs) const {
            return _ptr == rhs._ptr;
		}
		bool operator !=(const iterator &rhs) const {
            return _ptr != rhs._ptr;
		}
		bool operator !=(const const_iterator &rhs) const {
            return _ptr != rhs._ptr;
		}
	};
    //end of class const_iterator

	vector() : start(nullptr, this), finish(nullptr, this), end_of_storage(nullptr, this) {}
	vector(const vector &other) {
        start._container = this;
        finish._container = this;
        end_of_storage._container = this;
        start._ptr = (T *) malloc(sizeof(T) * other.capacity());
        finish._ptr = start._ptr + other.size();
        end_of_storage._ptr = start._ptr + other.capacity();
        for (size_type i = 0; i < other.size(); ++i)
            *(start._ptr + i) = *(other.start._ptr + i);
	}
	~vector() {
        for (iterator _it = start; _it != finish; ++_it)
            _it._ptr->~T();
        free(start._ptr);
        start._ptr = finish._ptr = end_of_storage._ptr = nullptr;
	}
	vector &operator =(const vector &other) {
        if (other.start._container == this)
            return *this;
        clear();
        start._container = this;
        finish._container = this;
        end_of_storage._container = this;
        start._ptr = (T *)malloc(sizeof(T) * other.capacity());
        finish._ptr = start._ptr + other.size();
        end_of_storage._ptr = start._ptr + other.capacity();
        for (size_type i = 0; i < other.size(); ++i)
            *(start._ptr + i) = *(other.start._ptr + i);
        return *this;
	}

	T &at(const size_type &pos) {
	    if (pos >= size())
            throw index_out_of_bound();
        return *(start + pos);
    }
	const T &at(const size_type &pos) const {
        if (pos >= size())
            throw index_out_of_bound();
        return *(start + pos);
	}
	T &operator [](const size_t &pos) {
        if (pos >= size())
            throw index_out_of_bound();
        return *(start + pos);
	}
	const T &operator [](const size_type &pos) const {
        if (pos >= size())
            throw index_out_of_bound();
        return *(start + pos);
	}
	const T &front() const {
        if (start == finish)
            throw container_is_empty();
        return *start;
	}
	const T &back() const {
        if (start == finish)
            throw container_is_empty();
        return *(finish - 1);
	}
	iterator begin(){
        return start;
	}
	const_iterator cbegin() const {
        return start;
	}
	iterator end(){
        return finish;
	}
	const_iterator cend() const {
        return finish;
	}
	bool empty() const {
        return start == finish;
	}
	size_type size() const {
        return finish - start;
	}
	size_type capacity() const {
        return end_of_storage - start;
	}
	void clear(){
	    for (iterator _it = start; _it != finish; ++_it)
            _it._ptr->~T();
        free(start._ptr);
        start._ptr = finish._ptr = end_of_storage._ptr = nullptr;
	}

protected:
    void _reallocate(){
        if (start._ptr != nullptr){
            size_type _size = size();
            T *_newstart = (T *)malloc(sizeof(T) * (_size * 2 + 1));
            for (size_type i = 0; i < _size; ++i){
                new (_newstart + i) T(*(start._ptr + i));
                (start._ptr + i)->~T();
            }
            free(start._ptr);
            start._ptr = _newstart;
            finish._ptr = start._ptr + _size;
            end_of_storage._ptr = start._ptr + _size * 2;
        }
        else{
            start._ptr = (T *)malloc(sizeof(T) * 2);
            finish._ptr = start._ptr;
            end_of_storage._ptr = start._ptr + 1;
        }
    }

public:
	iterator insert(iterator pos, const T &value){
	    if (finish == end_of_storage){
            size_type _dif = pos - start;
            _reallocate();
            pos = start + _dif;
	    }
        for (iterator _it = finish; _it != pos; --_it){
            *_it = *(_it - 1);
        }
        *pos = value;
        ++finish;
        return pos;
	}
	iterator insert(const size_type &ind, const T &value) {
        if (ind > size())
            throw index_out_of_bound();
        return insert(start + ind, value);
	}
	iterator erase(iterator pos) {
        for (iterator _it = pos; _it != finish; ++_it)
            *_it = *(_it + 1);
        finish._ptr->~T();
        --finish;
        return pos;
	}
	iterator erase(const size_t &ind) {
        if (ind >= size())
            throw index_out_of_bound();
        return erase(start + ind);
	}
	void push_back(const T &value) {
        if (finish == end_of_storage)
            _reallocate();
        new (finish._ptr) T(value);
        ++finish;
    }
	void pop_back() {
        if (start == finish)
            throw container_is_empty();
        finish._ptr->~T();
        --finish;
	}
};
}

#endif
