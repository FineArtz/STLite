#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

    template<class T>
    void swap(T &x, T &y) {
        T t = x;
        x = y;
        y = t;
    }
    /**
    * a container like std::priority_queue which is a heap internal.
    * it should be based on the vector written by yourself.
    */
    template<typename T, class Compare = std::less<T>>
    class priority_queue {
    public:
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

    private:
        class _Node {
        public:
            _Node() = default;
            _Node(const _Node &n) : data(n.data), dist(n.dist), l(n.l), r(n.r) {}
            _Node(const T &t, const int &d = -1, _Node *L = nullptr, _Node *R = nullptr) :
                data(t), dist(d), l(L), r(R) {}
            T data;
            int dist = -1;
            _Node *l = nullptr, *r = nullptr;
        };

        _Node *_merge(_Node *x, _Node *y) {
            if (x == nullptr) return y;
            if (y == nullptr) return x;
            if (Compare()(x->data, y->data))
                sjtu::swap(x, y);
            x->r = _merge(x->r, y);
            if (x->l == nullptr || x->l->dist < x->r->dist)
                sjtu::swap(x->l, x->r);
            if (x->r == nullptr)
                x->dist = 0;
            else
                x->dist = x->r->dist + 1;
            return x;
        }

        void _dispose(_Node *x) {
            if (x == nullptr)
                return;
            if (x->l != nullptr) {
                _dispose(x->l);
                x->l = nullptr;
            }
            if (x->r != nullptr) {
                _dispose(x->r);
                x->r = nullptr;
            }
            delete x;
        }

        void _copy(_Node *x, _Node *y) {
            if (y->l != nullptr) {
                x->l = new _Node(*(y->l));
                _copy(x->l, y->l);
            }
            if (y->r != nullptr) {
                x->r = new _Node(*(y->r));
                _copy(x->r, y->r);
            }
        }
        _Node *root = nullptr;
        size_type _size = 0;

        void _clear() {
            if (root != nullptr) {
                _dispose(root);
                root = nullptr;
            }
            _size = 0;
        }

    public:
        /**
        * TODO constructors
        */
        priority_queue() : root(nullptr), _size(0) {}
        priority_queue(const priority_queue &other) {
            if (other.root != nullptr) {
                root = new _Node(*(other.root));
                _copy(root, other.root);
            }
            _size = other._size;
        }
        ~priority_queue() {
            _clear();
        }
        /**
        * TODO Assignment operator
        */
        priority_queue &operator =(const priority_queue &other) {
            if (other.root == root)
                return *this;
            _clear();
            if (other.root != nullptr) {
                root = new _Node(*(other.root));
                _copy(root, other.root);
            }
            _size = other._size;
            return *this;
        }
        /**
        * get the top of the queue.
        * @return a reference of the top element.
        * throw container_is_empty if empty() returns true;
        */
        const T & top() const {
            if (root == nullptr)
                throw container_is_empty();
            return root->data;
        }
        /**
        * TODO
        * push new element to the priority queue.
        */
        void push(const T &e) {
            _Node *tmp = new _Node(e, 0);
            root = _merge(root, tmp);
            ++_size;
        }
        /**
        * TODO
        * delete the top element.
        * throw container_is_empty if empty() returns true;
        */
        void pop() {
            if (root == nullptr)
                throw container_is_empty();
            _Node *tmp = root;
            root = _merge(root->l, root->r);
            delete tmp;
            --_size;
        }
        /**
        * return the number of the elements.
        */
        size_type size() const {
            return _size;
        }
        /**
        * check if the container has at least an element.
        * @return true if it is empty, false if it has at least an element.
        */
        bool empty() const {
            return (root == nullptr);
        }
        /**
        * return a merged priority_queue with at least O(logn) complexity.
        */
        void merge(priority_queue &other) {
            root = _merge(root, other.root);
            _size += other._size;
            other.root = nullptr;
            other._size = 0;
        }
    };

}

#endif
