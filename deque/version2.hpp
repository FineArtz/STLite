#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>
#include <cstdlib>

namespace sjtu {

const int MAXS = 600;

template<class T>
class deque {
public:
    class iterator;
    class const_iterator;
    friend class iterator;
    friend class const_iterator;

public:
    using value_type        = T;
    using pointer           = T *;
    using reference         = T &;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;

private:
    size_type _size = 0;
    class _Block;

    _Block *head = new _Block(), *tail = new _Block();

    class _Block{
    public:
        T *data = nullptr;
        size_type len = 0;
        _Block *prev = nullptr, *next = nullptr;

        _Block(_Block *p = nullptr, _Block *n = nullptr) : prev(p), next(n) {}
        ~_Block() {}

        void _clear() {
            if (data == nullptr)
                return;
            for (size_type i = 0; i < len; ++i)
                (data + i)->~T();
            free(data);
        }

        void _split(size_type pos) {
            if (pos >= len || pos < 0)
                return;
            _Block *b = new _Block(this, next);
            if (next != nullptr)
                next->prev = b;
            next = b;
            if (prev->prev != nullptr) {
                b->len = len - pos;
                b->data = (T *)malloc(sizeof(T) * (MAXS + 1));
                for (size_type i = 0; i < len - pos; ++i) {
                    new (b->data + i) T(*(data + i + pos));
                    (data + i + pos)->~T();
                }
                len = pos;
            }
            else {
                b->len = pos + 1;
                b->data = (T *)malloc(sizeof(T) * (MAXS + 1));
                for (size_type i = 0; i < pos + 1; ++i) {
                    new (b->data + i) T(*(data + pos - i));
                    (data + pos - i)->~T();
                }
                for (size_type i = pos + 1; i < len; ++i) {
                    new (data + i - pos - 1) T(*(data + i));
                    (data + i)->~T();
                }
                len -= (pos + 1);
            }
        }

        bool _merge() {
            _Block *b = next;
            if (b == nullptr || b->next == nullptr)
                return false;
            if (len + b->len > MAXS)
                return false;
            next = b->next;
            if (next != nullptr)
                next->prev = this;
            if (prev->prev != nullptr) {
                for (size_type i = 0; i < b->len; ++i)
                    new (data + len++) T(*(b->data + i));
            }
            else {
                for (size_type i = len + b->len - 1; i >= b->len; --i) {
                    new (data + i) T(*(data + i - b->len));
                    (data + i - b->len)->~T();
                }
                for (size_type i = 0; i < b->len; ++i) {
                    new (data + i) T(*(b->data + b->len - i - 1));
                }
                len += b->len;
            }
            b->_clear();
            delete b;
            return true;
        }
    };

    void _clear() {
        _Block *p = head, *q;
        while (p != nullptr) {
            q = p;
            p = p->next;
            q->_clear();
            delete q;
        }
        _size = 0;
    }

public:
	class iterator {
        friend class const_iterator;
        friend class deque;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;
	private:
        _Block *_ptr;
        size_type _pos;
        deque *_container;
	public:
        iterator(_Block *_p = nullptr, size_type _pp = 0, deque *_c = nullptr)
            : _ptr(_p), _pos(_pp), _container(_c) {}
        iterator(const iterator &i)
            : _ptr(i._ptr), _pos(i._pos), _container(i._container) {}
        iterator(const const_iterator &ci)
            : _ptr(ci._ptr), _pos(ci._ptr), _container(ci,_container) {}

        iterator operator +(const int &n) const {
            if (n < 0)
                return *this - (-n);
            iterator t(*this);
            int d = n;
            while (d > 0) {
                if (t._ptr != _container->head->next) {
                    if (d < t._ptr->len - t._pos) {
                        t._pos += d;
                        d = 0;
                        break;
                    }
                    else {
                        d -= (t._ptr->len - t._pos);
                        t._ptr = t._ptr->next;
                        t._pos = 0;
                        if (t._ptr == t._container->tail)
                            break;
                    }
                }
                else {
                    if (d <= t._pos) {
                        t._pos -= d;
                        d = 0;
                        break;
                    }
                    else {
                        d -= (t._pos + 1);
                        t._ptr = t._ptr->next;
                        t._pos = 0;
                        if (t._ptr == t._container->tail)
                            break;
                    }
                }
            }
            if (d > 0) {
                t._ptr = nullptr;
                t._pos = 0;
            }
            return t;
        }
        iterator operator -(const int &n) const {
            if (n < 0)
                return *this + (-n);
            iterator t(*this);
            int d = n;
            while (d > 0) {
                if (t._ptr != _container->head->next) {
                    if (d <= t._pos) {
                        t._pos -= d;
                        d = 0;
                        break;
                    }
                    else {
                        d -= (t._pos + 1);
                        t._ptr = t._ptr->prev;
                        if (t._ptr != t._container->head) {
                            if (t._ptr->prev != t._container->head)
                                t._pos = t._ptr->len - 1;
                            else
                                t._pos = 0;
                        }
                        else
                            break;
                    }
                }
                else {
                    if (d < t._ptr->len - t._pos) {
                        t._pos += d;
                        d = 0;
                        break;
                    }
                    else {
                        d -= (t._ptr->len - t._pos - 1);
                        t._ptr = t._ptr->prev;
                        t._pos = 0;
                        break;
                    }
                }
            }
            if (d > 0 || t._ptr == t._container->head) {
                t._ptr = nullptr;
                t._pos = 0;
            }
            return t;
        }

        difference_type operator -(const iterator &rhs) const {
            if (_container != rhs._container)
                throw invalid_iterator();
            _Block *b = rhs._ptr;
            if (b == _ptr) {
                if (_ptr == _container->head->next)
                    return (difference_type)rhs._pos - (difference_type)_pos;
                else
                    return (difference_type)_pos - (difference_type)rhs._pos;
            }
            difference_type ans = 0;
            if (b != _container->head->next)
                ans = b->len - rhs._pos;
            else
                ans = rhs._pos + 1;
            b = b->next;
            while (b != _container->tail && b != _ptr) {
                ans += b->len;
                b = b->next;
            }
            if (_ptr == b) {
                ans += _pos;
                return ans;
            }
            b = _ptr;
            if (b != _container->head->next)
                ans = b->len - _pos;
            else
                ans = _pos + 1;
            b = b->next;
            while (b != _container->tail && b != rhs._ptr) {
                ans += b->len;
                b = b->next;
            }
            ans += rhs._pos;
            return -ans;
        }

        iterator operator +=(const int &n) {
            iterator t = *this + n;
            _ptr = t._ptr;
            _pos = t._pos;
            return *this;
        }
        iterator operator -=(const int &n) {
            iterator t = *this - n;
            _ptr = t._ptr;
            _pos = t._pos;
            return *this;
        }

        iterator operator ++(int) {
            if (_ptr == _container->tail)
                throw invalid_iterator();
            iterator t = *this;
            if (_ptr != _container->head->next) {
                ++_pos;
                if (_pos == _ptr->len) {
                    _ptr = _ptr->next;
                    _pos = 0;
                }
            }
            else {
                if (_pos == 0) {
                    _ptr = _ptr->next;
                }
                else
                    --_pos;
            }
            return t;
        }
        iterator &operator ++() {
            if (_ptr == _container->tail)
                throw invalid_iterator();
            if (_ptr != _container->head->next) {
                ++_pos;
                if (_pos == _ptr->len) {
                    _ptr = _ptr->next;
                    _pos = 0;
                }
            }
            else {
                if (_pos == 0) {
                    _ptr = _ptr->next;
                }
                else
                    --_pos;
            }
            return *this;
        }

        iterator operator --(int) {
            iterator t = *this;
            if (_ptr != _container->head->next) {
                if (_pos == 0) {
                    _ptr = _ptr->prev;
                    if (_ptr->prev != _container->head)
                        _pos = _ptr->len - 1;
                    else
                        _pos = 0;
                }
                else
                    --_pos;
            }
            else {
                ++_pos;
                if (_pos == _ptr->len)
                    _ptr = _container->head;
            }
            if (_ptr == _container->head)
                throw invalid_iterator();
            return t;
        }
        iterator &operator --() {
            if (_ptr != _container->head->next) {
                if (_pos == 0) {
                    _ptr = _ptr->prev;
                    if (_ptr->prev != _container->head)
                        _pos = _ptr->len - 1;
                    else
                        _pos = 0;
                }
                else
                    --_pos;
            }
            else {
                ++_pos;
                if (_pos == _ptr->len)
                    _ptr = _container->head;
            }
            if (_ptr == _container->head)
                throw invalid_iterator();
            return *this;
        }

		T &operator *() const {
            if (_ptr == nullptr || _ptr->data == nullptr)
                throw invalid_iterator();
            return *(_ptr->data + _pos);
        }
		T *operator ->() const noexcept {
            if (_ptr == nullptr || _ptr->data == nullptr)
                throw invalid_iterator();
            return _ptr->data + _pos;
        }

		bool operator ==(const iterator &rhs) const {
            return (_ptr == rhs._ptr && _pos == rhs._pos);
        }
		bool operator ==(const const_iterator &rhs) const {
            return (_ptr == rhs._ptr && _pos == rhs._pos);
        }

		bool operator !=(const iterator &rhs) const {
            return (_ptr != rhs._ptr || _pos != rhs._pos);
        }
		bool operator !=(const const_iterator &rhs) const {
            return (_ptr != rhs._ptr || _pos != rhs._pos);
        }
	};
    //end of class iterator

	class const_iterator {
        friend class iterator;
        friend class deque;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = T;
        using difference_type   = ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;
    private:
        _Block *_ptr;
        size_type _pos;
        deque *_container;
    public:
        const_iterator(_Block *_p = nullptr, size_type _pp = 0, deque *_c = nullptr)
            : _ptr(_p), _pos(_pp), _container(_c) {}
        const_iterator(const iterator &i)
            : _ptr(i._ptr), _pos(i._pos), _container(i._container) {}
        const_iterator(const const_iterator &ci)
            : _ptr(ci._ptr), _pos(ci._pos), _container(ci._container) {}

        const_iterator operator +(const int &n) const {
            if (n < 0)
                return *this - (-n);
            const_iterator t(*this);
            int d = n;
            while (d > 0) {
                if (t._ptr != _container->head->next) {
                    if (d < t._ptr->len - t._pos) {
                        t._pos += d;
                        d = 0;
                        break;
                    }
                    else {
                        d -= (t._ptr->len - t._pos);
                        t._ptr = t._ptr->next;
                        t._pos = 0;
                        if (t._ptr == t._container->tail)
                            break;
                    }
                }
                else {
                    if (d <= t._pos) {
                        t._pos -= d;
                        d = 0;
                        break;
                    }
                    else {
                        d -= (t._pos + 1);
                        t._ptr = t._ptr->next;
                        t._pos = 0;
                        if (t._ptr == t._container->tail)
                            break;
                    }
                }
            }
            if (d > 0) {
                t._ptr = nullptr;
                t._pos = 0;
            }
            return t;
        }
        const_iterator operator -(const int &n) const {
            if (n < 0)
                return *this + (-n);
            const_iterator t(*this);
            int d = n;
            while (d > 0) {
                if (t._ptr != _container->head->next) {
                    if (d <= t._pos) {
                        t._pos -= d;
                        d = 0;
                        break;
                    }
                    else {
                        d -= (t._pos + 1);
                        t._ptr = t._ptr->prev;
                        if (t._ptr != t._container->head) {
                            if (t._ptr->prev != t._container->head)
                                t._pos = t._ptr->len - 1;
                            else
                                t._pos = 0;
                        }
                        else
                            break;
                    }
                }
                else {
                    if (d < t._ptr->len - t._pos) {
                        t._pos += d;
                        d = 0;
                        break;
                    }
                    else {
                        d -= (t._ptr->len - t._pos - 1);
                        t._ptr = t._ptr->prev;
                        t._pos = 0;
                        break;
                    }
                }
            }
            if (d > 0 || t._ptr == t._container->head) {
                t._ptr = nullptr;
                t._pos = 0;
            }
            return t;
        }

        difference_type operator -(const const_iterator &rhs) const {
            if (_container != rhs._container)
                throw invalid_iterator();
            _Block *b = rhs._ptr;
            if (b == _ptr) {
                if (_ptr == _container->head->next)
                    return (difference_type)rhs._pos - (difference_type)_pos;
                else
                    return (difference_type)_pos - (difference_type)rhs._pos;
            }
            difference_type ans = 0;
            if (b != _container->head->next)
                ans = b->len - rhs._pos;
            else
                ans = rhs._pos + 1;
            b = b->next;
            while (b != _container->tail && b != _ptr) {
                ans += b->len;
                b = b->next;
            }
            if (_ptr == b) {
                ans += _pos;
                return ans;
            }
            b = _ptr;
            if (b != _container->head->next)
                ans = b->len - _pos;
            else
                ans = _pos + 1;
            b = b->next;
            while (b != _container->tail && b != rhs._ptr) {
                ans += b->len;
                b = b->next;
            }
            ans += rhs._pos;
            return -ans;
        }

        const_iterator operator +=(const int &n) {
            const_iterator t = *this + n;
            _ptr = t._ptr;
            _pos = t._pos;
            return *this;
        }
        const_iterator operator -=(const int &n) {
            const_iterator t = *this - n;
            _ptr = t._ptr;
            _pos = t._pos;
            return *this;
        }

        const_iterator operator ++(int) {
            if (_ptr == _container->tail)
                throw invalid_iterator();
            const_iterator t = *this;
            if (_ptr != _container->head->next) {
                ++_pos;
                if (_pos == _ptr->len) {
                    _ptr = _ptr->next;
                    _pos = 0;
                }
            }
            else {
                if (_pos == 0) {
                    _ptr = _ptr->next;
                }
                else
                    --_pos;
            }
            return t;
        }
        const_iterator &operator ++() {
            if (_ptr == _container->tail)
                throw invalid_iterator();
            if (_ptr != _container->head->next) {
                ++_pos;
                if (_pos == _ptr->len) {
                    _ptr = _ptr->next;
                    _pos = 0;
                }
            }
            else {
                if (_pos == 0) {
                    _ptr = _ptr->next;
                }
                else
                    --_pos;
            }
            return *this;
        }

        const_iterator operator --(int) {
            const_iterator t = *this;
            if (_ptr != _container->head->next) {
                if (_pos == 0) {
                    _ptr = _ptr->prev;
                    if (_ptr->prev != _container->head)
                        _pos = _ptr->len - 1;
                    else
                        _pos = 0;
                }
                else
                    --_pos;
            }
            else {
                ++_pos;
                if (_pos == _ptr->len)
                    _ptr = _container->head;
            }
            if (_ptr == _container->head)
                throw invalid_iterator();
            return t;
        }
        const_iterator &operator --() {
            if (_ptr != _container->head->next) {
                if (_pos == 0) {
                    _ptr = _ptr->prev;
                    if (_ptr->prev != _container->head)
                        _pos = _ptr->len - 1;
                    else
                        _pos = 0;
                }
                else
                    --_pos;
            }
            else {
                ++_pos;
                if (_pos == _ptr->len)
                    _ptr = _container->head;
            }
            if (_ptr == _container->head)
                throw invalid_iterator();
            return *this;
        }

        T &operator *() const {
            if (_ptr == nullptr || _ptr->data == nullptr)
                throw invalid_iterator();
            return *(_ptr->data + _pos);
        }
        T *operator ->() const noexcept {
            if (_ptr == nullptr || _ptr->data == nullptr)
                throw invalid_iterator();
            return _ptr->data + _pos;
        }

        bool operator ==(const iterator &rhs) const {
            return (_ptr == rhs._ptr && _pos == rhs._pos);
        }
        bool operator ==(const const_iterator &rhs) const {
            return (_ptr == rhs._ptr && _pos == rhs._pos);
        }

        bool operator !=(const iterator &rhs) const {
            return (_ptr != rhs._ptr || _pos != rhs._pos);
        }
        bool operator !=(const const_iterator &rhs) const {
            return (_ptr != rhs._ptr || _pos != rhs._pos);
        }
	};
	//end of class const_iterator

	deque() {
        head->next = tail;
        tail->prev = head;
    }
	deque(const deque &other) {
        _Block *p = head, *q = other.head;
        while (q->next != other.tail) {
            q = q->next;
            p->next = new _Block(p);
            p->next->prev = p;
            p = p->next;
            p->data = (T *)malloc(sizeof(T) * (MAXS + 1));
            p->len = q->len;
            for (size_type i = 0; i < p->len; ++i)
                new (p->data + i) T(*(q->data + i));
        }
        p->next = tail;
        tail->prev = p;
        _size = other._size;
    }
	~deque() {
        _clear();
    }

	deque &operator =(const deque &other) {
        if (this == &other)
            return *this;
        _clear();
        head = new _Block();
        tail = new _Block();
        _Block *p = head, *q = other.head;
        while (q->next != other.tail) {
            q = q->next;
            p->next = new _Block(p);
            p->next->prev = p;
            p = p->next;
            p->data = (T *)malloc(sizeof(T) * (MAXS + 1));
            p->len = q->len;
            for (size_type i = 0; i < p->len; ++i)
                new (p->data + i) T(*(q->data + i));
        }
        p->next = tail;
        tail->prev = p;
        _size = other._size;
        return *this;
    }

	T &at(const size_type &pos) {
        if (pos < 0 || pos >= _size)
            throw index_out_of_bound();
        size_type d = pos;
        _Block *p = head->next;
        while (p != tail) {
            if (p != head->next) {
                if (d < p->len) {
                    return *(p->data + d);
                }
                else {
                    d -= p->len;
                    p = p->next;
                }
            }
            else {
                if (d < p->len)
                    return *(p->data + p->len - d - 1);
                else {
                    d -= p->len;
                    p = p->next;
                }
            }
        }
    }
	const T &at(const size_type &pos) const {
        if (pos < 0 || pos >= _size)
            throw index_out_of_bound();
        size_type d = pos;
        _Block *p = head->next;
        while (p != tail) {
            if (p != head->next) {
                if (d < p->len) {
                    return *(p->data + d);
                }
                else {
                    d -= p->len;
                    p = p->next;
                }
            }
            else {
                if (d < p->len)
                    return *(p->data + p->len - d - 1);
                else {
                    d -= p->len;
                    p = p->next;
                }
            }
        }
    }
	T &operator [](const size_type &pos) {
        if (pos < 0 || pos >= _size)
            throw index_out_of_bound();
        size_type d = pos;
        _Block *p = head->next;
        while (p != tail) {
            if (p != head->next) {
                if (d < p->len) {
                    return *(p->data + d);
                }
                else {
                    d -= p->len;
                    p = p->next;
                }
            }
            else {
                if (d < p->len)
                    return *(p->data + p->len - d - 1);
                else {
                    d -= p->len;
                    p = p->next;
                }
            }
        }
    }
	const T &operator [](const size_type &pos) const {
        if (pos < 0 || pos >= _size)
            throw index_out_of_bound();
        size_type d = pos;
        _Block *p = head->next;
        while (p != tail) {
            if (p != head->next) {
                if (d < p->len) {
                    return *(p->data + d);
                }
                else {
                    d -= p->len;
                    p = p->next;
                }
            }
            else {
                if (d < p->len)
                    return *(p->data + p->len - d - 1);
                else {
                    d -= p->len;
                    p = p->next;
                }
            }
        }
    }

	const T &front() const {
        if (_size == 0)
            throw container_is_empty();
        return *(head->next->data + head->next->len - 1);
    }
	const T &back() const {
        if (_size == 0)
            throw container_is_empty();
        if (tail->prev->prev != head)
            return *(tail->prev->data + tail->prev->len - 1);
        else
            return *(tail->prev->data);
    }

	iterator begin() {
        if (head->next != tail)
            return iterator(head->next, head->next->len - 1, this);
        else
            return iterator(tail, 0, this);
    }
	const_iterator cbegin() const {
        if (head->next != tail)
            return const_iterator(head->next, head->next->len - 1, const_cast<deque *>(this));
        else
            return const_iterator(tail, 0, const_cast<deque *>(this));
    }

    iterator end() {
        return iterator(tail, 0, this);
    }
	const_iterator cend() const {
        return const_iterator(tail, 0, const_cast<deque *>(this));
    }

	bool empty() const {
        return (_size == 0);
    }

	size_type size() const {
        return _size;
    }

	void clear() {
        _clear();
        head = new _Block();
        tail = new _Block();
        head->next = tail;
        tail->prev = head;
    }

	iterator insert(iterator pos, const T &value) {
        if (pos._container != this || pos._ptr == nullptr)
            throw invalid_iterator();
        size_type d = pos - begin();
        ++_size;
        if (pos._ptr->len >= MAXS) {
            if (pos._ptr != head->next) {
                pos._ptr->_split(pos._pos);
                new (pos._ptr->data + pos._ptr->len) T(value);
                ++pos._ptr->len;
            }
            else {
                pos._ptr->_split(pos._pos);
                for (size_type i = pos._ptr->len; i > 0; --i) {
                    new (pos._ptr->data + i) T(*(pos._ptr->data + i - 1));
                    (pos._ptr->data + i - 1)->~T();
                }
                new (pos._ptr->data) T(value);
                ++pos._ptr->len;
                pos._pos = 0;
            }
            return pos;
        }
        else if (pos._ptr == tail) {
            if (pos._ptr->prev == head) {
                tail->prev->next = new _Block(tail->prev, tail);
                _Block *b = tail->prev->next;
                b->prev->next = b;
                b->next->prev = b;
                b->data = (T *)malloc(sizeof(T) * (MAXS + 1));
                new (b->data) T(value);
                b->len = 1;
                return iterator(b, 0, this);
            }
            else if (pos._ptr->prev->prev != head){
                _Block *b = tail->prev;
                if (b->len >= MAXS) {
                    b->_split(MAXS / 2);
                    b = b->next;
                }
                new (b->data + b->len) T(value);
                ++(b->len);
                return iterator(b, b->len - 1, this);
            }
            else {
                _Block *b = tail->prev;
                for (size_type i = b->len; i > 0; --i) {
                    new (b->data + i) T(*(b->data + i - 1));
                    (b->data + i - 1)->~T();
                }
                new (b->data) T(value);
                ++(b->len);
                return iterator(b, 0, this);
            }
        }
        else {
            if (pos._ptr->prev != head) {
                for (size_type i = pos._ptr->len; i > pos._pos; --i) {
                    if (i != pos._ptr->len)
                        (pos._ptr->data + i)->~T();
                    new (pos._ptr->data + i) T(*(pos._ptr->data + i - 1));
                }
                (pos._ptr->data + pos._pos)->~T();
                new (pos._ptr->data + pos._pos) T(value);
                ++(pos._ptr->len);
            }
            else {
                for (size_type i = pos._ptr->len; i > pos._pos + 1; --i) {
                    if (i != pos._ptr->len)
                        (pos._ptr->data + i)->~T();
                    new (pos._ptr->data + i) T(*(pos._ptr->data + i - 1));
                }
                if (pos._pos + 1 < pos._ptr->len)
                    (pos._ptr->data + pos._pos + 1)->~T();
                new (pos._ptr->data + pos._pos + 1) T(value);
                ++(pos._ptr->len);
                ++(pos._pos);
            }
            if (pos._ptr->len > MAXS)
                system("PAUSE");
            return pos;
        }
    }

	iterator erase(iterator pos) {
        if (pos._container != this || pos._ptr == nullptr || pos._ptr == tail)
            throw invalid_iterator();
        if (_size == 0)
            throw container_is_empty();
        bool flag = (pos._ptr->len - 1 == pos._pos);
        for (size_type i = pos._pos; i < pos._ptr->len - 1; ++i) {
            (pos._ptr->data + i)->~T();
            new (pos._ptr->data + i) T(*(pos._ptr->data + i + 1));
        }
        (pos._ptr->data + pos._ptr->len - 1)->~T();
        --(pos._ptr->len);
        /*if (pos._ptr->len == 0) {
            pos._ptr->prev->next = pos._ptr->next;
            pos._ptr->next->prev = pos._ptr->prev;
            _Block *b = pos._ptr->next;
            delete pos._ptr;
            pos._ptr = b;
            pos._pos = 0;
            --_size;
            return pos;
        }
        else {
            pos._ptr->_merge();
            --_size;
            if (flag) {
                pos._ptr = pos._ptr->next;
                pos._pos = 0;
            }
            return pos;
        }*/
        if (pos._ptr->len + pos._ptr->next->len <= MAXS) {
            if (pos._ptr->prev == head)
                pos._pos += pos._ptr->next->len;
            if (pos._ptr->len == 0 && pos._ptr->next == tail) {
                _Block *b = pos._ptr;
                pos._ptr = pos._ptr->next;
                pos._pos = 0;
                head->next = tail;
                tail->prev = head;
                delete b;
            }
            else
                pos._ptr->_merge();
        }
        --_size;
        if (pos._ptr == tail)
            return pos;
        if (pos._ptr->prev == head) {
            if (pos._pos != 0)
                --pos._pos;
            else {
                pos._ptr = pos._ptr->next;
                pos._pos = 0;
            }
        }
        else {
            if (flag) {
                pos._ptr = pos._ptr->next;
                pos._pos = 0;
            }
        }
        return pos;
    }

	void push_back(const T &value) {
        _Block *b = tail->prev;
        if (b == head || b->len >= MAXS) {
            b->next = new _Block(b, tail);
            b = b->next;
            b->prev->next = b;
            b->next->prev = b;
            b->data = (T *)malloc(sizeof(T) * (MAXS + 1));
        }
        if (b != head->next) {
            new (b->data + b->len) T(value);
        }
        else {
            for (size_type i = b->len; i > 0; --i) {
                new (b->data + i) T(*(b->data + i - 1));
                (b->data + i - 1)->~T();
            }
            new (b->data) T(value);
        }
        ++(b->len);
        ++_size;
    }

	void pop_back() {
        if (_size == 0)
            throw container_is_empty();
        _Block *b = tail->prev;
        if (b != head->next) {
            (b->data + b->len - 1)->~T();
        }
        else {
            for (size_type i = 0; i < b->len - 1; ++i) {
                (b->data + i)->~T();
                new (b->data + i) T(*(b->data + i + 1));
            }
            (b->data + b->len - 1)->~T();
        }
        --(b->len);
        --_size;
        b->_merge();
        if (b->len == 0) {
            b->prev->next = b->next;
            b->next->prev = b->prev;
            delete b;
        }
    }

	void push_front(const T &value) {
        if (head->next != tail) {
            new (head->next->data + head->next->len) T(value);
            ++(head->next->len);
            if (head->next->len >= MAXS)
                head->next->_split(MAXS / 2);
        }
        else {
            head->next = new _Block(head, tail);
            tail->prev = head->next;
            head->next->data = (T *)malloc(sizeof(T) * (MAXS + 1));
            new (head->next->data) T(value);
            head->next->len = 1;
        }
        ++_size;
    }

	void pop_front() {
        if (_size == 0)
            throw container_is_empty();
        _Block *b = head->next;
        (b->data + b->len - 1)->~T();
        --(b->len);
        --_size;
        b->_merge();
        if (b->len == 0) {
            b->prev->next = b->next;
            b->next->prev = b->prev;
            b->_clear();
            delete b;
        }
    }
};

}

#endif
