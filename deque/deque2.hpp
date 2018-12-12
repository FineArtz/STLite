#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"


#include <cstddef>

#define maxLengthOfBlock 1024
namespace sjtu {

    template<class T>
    class deque {
    public:
        struct Block {
            int length;
            int maxLengthBlock;
            T *data[maxLengthOfBlock];
            Block *prev, *next;

            Block() {
                length = 0;
                maxLengthBlock = maxLengthOfBlock;
                prev = nullptr;
                next = nullptr;
                for (int i = 0; i < maxLengthOfBlock; i++)
                    data[i] = nullptr;
            }

            Block(const T &v) {
                length = 1;
                maxLengthBlock = maxLengthOfBlock;
                prev = nullptr;
                next = nullptr;
                data[0] = new T(v);
                for (int i = 1; i < maxLengthOfBlock; i++)
                    data[i] = nullptr;
            }

            Block(const Block &rhs) {
                length = rhs.length;
                maxLengthBlock = maxLengthOfBlock;
                for (int i = 0; i < length; i++)
                    data[i] = new T(*(rhs.data[i]));
            }

            ~Block() {
                for (int i = 0; i < length; i++) if (data[i] != nullptr) delete data[i];
            }
        };

    private:
        int totalLength;
        Block *head, *tail;
    public:
        class const_iterator;

        class iterator {
            friend class deque;

            friend class const_iterator;

        public:
            deque<T> *dequePointer;
            Block *blockPointer;
            int position, insideBlockPosition;
        public:
            iterator() {
                dequePointer = nullptr;
                blockPointer = nullptr;
                position = 0;
                insideBlockPosition = 0;
            }

            iterator(const iterator &rhs) {
                dequePointer = rhs.dequePointer;
                blockPointer = rhs.blockPointer;
                position = rhs.position;
                insideBlockPosition = rhs.insideBlockPosition;
            }

            iterator(deque<T> *_dequePointer, const int &_index) {
                dequePointer = _dequePointer;
                position = _index;
                int tmp_ibp = _index;
                Block *currentBlock = dequePointer->head->next;
                while (currentBlock != dequePointer->tail && tmp_ibp >= currentBlock->length) {
                    tmp_ibp -= currentBlock->length;
                    currentBlock = currentBlock->next;
                }
                insideBlockPosition = tmp_ibp;
                blockPointer = currentBlock;
            }

            iterator(deque<T> *_dequePointer, Block *_blockPointer, const int &_index, const int &_inside) {
                dequePointer = _dequePointer;
                blockPointer = _blockPointer;
                position = _index;
                insideBlockPosition = _inside;
            }

            iterator operator+(const int &n) const {
                if (n < 0) return operator-(-n);
                if (n == 0) return iterator(dequePointer, blockPointer, position, insideBlockPosition);
                Block *currentBlock = blockPointer;
                int tmp_ibp = insideBlockPosition + n;
                while (tmp_ibp >= currentBlock->length && currentBlock->next != dequePointer->tail) {
                    tmp_ibp -= currentBlock->length;
                    currentBlock = currentBlock->next;
                }
                return iterator(dequePointer, currentBlock, position + n, tmp_ibp);
            }

            iterator operator-(const int &n) const {
                if (n < 0) return operator+(-n);
                if (n == 0) return *this;

                Block *currentBlock = blockPointer;
                int tmp_ibp = insideBlockPosition - n;
                while (tmp_ibp < 0) {
                    currentBlock = currentBlock->prev;
                    tmp_ibp += currentBlock->length;
                }
                return iterator(dequePointer, currentBlock, position - n, tmp_ibp);
            }

            int operator-(const iterator &rhs) const {
                if (rhs.dequePointer != dequePointer) throw 1;
                return position - rhs.position;
            }

            iterator operator+=(const int &n) {
                if (n < 0) return operator-=(-n);
                if (n == 0) return *this;
                Block *currentBlock = blockPointer;
                int tmp_ibp = insideBlockPosition + n;
                while (tmp_ibp >= currentBlock->length && currentBlock->next != dequePointer->tail) {
                    tmp_ibp -= currentBlock->length;
                    currentBlock = currentBlock->next;
                }
                blockPointer = currentBlock;
                insideBlockPosition = tmp_ibp;
                position += n;
                return *this;
            }

            iterator operator-=(const int &n) {
                if (n < 0) return operator+=(-n);
                if (n == 0) return *this;
                Block *currentBlock = blockPointer;
                int tmp_ibp = insideBlockPosition - n;
                while (tmp_ibp < 0) {
                    currentBlock = currentBlock->prev;
                    tmp_ibp += currentBlock->length;
                }
                blockPointer = currentBlock;
                insideBlockPosition = tmp_ibp;
                position -= n;
                return *this;
            }

            iterator operator++(int) {
                auto tmp = *this;
                if (insideBlockPosition + 1 < blockPointer->length) insideBlockPosition++;
                else {
                    insideBlockPosition = 0;
                    blockPointer = blockPointer->next;
                }
                position++;
                return tmp;
            }

            iterator &operator++() {
                if (insideBlockPosition + 1 < blockPointer->length) insideBlockPosition++;
                else {
                    insideBlockPosition = 0;
                    blockPointer = blockPointer->next;
                }
                position++;
                return *this;
            }

            iterator operator--(int) {
                auto tmp = *this;
                if (insideBlockPosition > 0) insideBlockPosition--;
                else {
                    blockPointer = blockPointer->prev;
                    insideBlockPosition = blockPointer->length - 1;
                }
                position--;
                return tmp;
            }

            iterator &operator--() {
                if (insideBlockPosition > 0) insideBlockPosition--;
                else {
                    blockPointer = blockPointer->prev;
                    insideBlockPosition = blockPointer->length - 1;
                }
                position--;
                return *this;
            }

            T &operator*() const {
                if (blockPointer == dequePointer->tail || blockPointer->data[insideBlockPosition] == nullptr
                    || insideBlockPosition < 0)
                    throw 1;
                return *(blockPointer->data[insideBlockPosition]);
            }

            T *operator->() const noexcept {
                if (blockPointer == dequePointer->tail || blockPointer->data[insideBlockPosition] == nullptr) throw 1;
                return blockPointer->data[insideBlockPosition];
            }

            bool operator==(const iterator &rhs) const {
                return dequePointer == rhs.dequePointer && position == rhs.position;
            }

            bool operator==(const const_iterator &rhs) const {
                return dequePointer == rhs.dequePointer && position == rhs.position;
            }

            bool operator!=(const iterator &rhs) const {
                return dequePointer != rhs.dequePointer || position != rhs.position;

            }

            bool operator!=(const const_iterator &rhs) const {
                return dequePointer != rhs.dequePointer || position != rhs.position;
            }
        };

        class const_iterator {
            friend class deque;

            friend class iterator;

        public:
            const deque<T> *dequePointer;
            const Block *blockPointer;
            int position;
            int insideBlockPosition;
        public:
            const_iterator() {
                dequePointer = nullptr;
                blockPointer = nullptr;
                position = 0;
                insideBlockPosition = 0;
            }

            const_iterator(const deque<T> *_deque, const Block *_block, int _pos, int tmp_iib) :
                    dequePointer(_deque), blockPointer(_block) {
                position = _pos;
                insideBlockPosition = tmp_iib;
            }

            const_iterator(const const_iterator &other) : dequePointer(other.dequePointer),
                                                          blockPointer(other.blockPointer) {
                position = other.position;
                insideBlockPosition = other.insideBlockPosition;
            }

            const_iterator(const iterator &other) : dequePointer(other.dequePointer),
                                                    blockPointer(other.blockPointer) {
                position = other.position;
                insideBlockPosition = other.insideBlockPosition;
            }

            const_iterator operator+(const int &n) const {
                if (n < 0) return operator-(-n);
                const Block *currentBlock = blockPointer;
                int tmp_ibp = insideBlockPosition + n;
                while (tmp_ibp > currentBlock->length) {
                    tmp_ibp -= currentBlock->length;
                    currentBlock = currentBlock->next;
                }
                return const_iterator(dequePointer, currentBlock, position + n, tmp_ibp);
            }

            const_iterator operator-(const int &n) const {
                if (n < 0) return operator+(-n);
                const Block *currentBlock = blockPointer;
                int tmp_ibp = insideBlockPosition - n;
                while (tmp_ibp < 0) {
                    currentBlock = currentBlock->prev;
                    tmp_ibp += currentBlock->length;
                }
                return const_iterator(dequePointer, currentBlock, position - n, tmp_ibp);
            }

            int operator-(const const_iterator &rhs) const {
                if (rhs.dequePointer != dequePointer) throw 1;
                return position - rhs.position;
            }

            const_iterator operator+=(const int &n) {
                if (n < 0) return operator-=(-n);
                const Block *currentBlock = blockPointer;
                int tmp_ibp = insideBlockPosition + n;
                while (tmp_ibp > currentBlock->length) {
                    tmp_ibp -= currentBlock->length;
                    currentBlock = currentBlock->next;
                }
                blockPointer = currentBlock;
                insideBlockPosition = tmp_ibp;
                position += n;
                return *this;
            }

            const_iterator operator-=(const int &n) {
                if (n < 0) return operator+=(-n);
                const Block *currentBlock = blockPointer;
                int tmp_ibp = insideBlockPosition - n;
                while (tmp_ibp < 0) {
                    currentBlock = currentBlock->prev;
                    tmp_ibp += currentBlock->length;
                }
                blockPointer = currentBlock;
                insideBlockPosition = tmp_ibp;
                position -= n;
                return *this;
            }

            const_iterator operator++(int) {
                auto tmp = *this;
                if (insideBlockPosition + 1 < blockPointer->length) insideBlockPosition++;
                else {
                    insideBlockPosition = 0;
                    blockPointer = blockPointer->next;
                }
                position++;
                return tmp;
            }

            const_iterator &operator++() {
                if (insideBlockPosition + 1 < blockPointer->length) insideBlockPosition++;
                else {
                    insideBlockPosition = 0;
                    blockPointer = blockPointer->next;
                }
                position++;
                return *this;
            }

            const_iterator operator--(int) {
                auto tmp = *this;
                if (insideBlockPosition > 0) insideBlockPosition--;
                else {
                    blockPointer = blockPointer->prev;
                    insideBlockPosition = blockPointer->length - 1;
                }
                position--;
                return tmp;
            }

            const_iterator &operator--() {
                if (insideBlockPosition > 0) insideBlockPosition--;
                else {
                    blockPointer = blockPointer->prev;
                    insideBlockPosition = blockPointer->length - 1;
                }
                position--;
                return *this;
            }

            T &operator*() const {
                if (blockPointer == dequePointer->tail || blockPointer->data[insideBlockPosition] == nullptr) throw 1;
                return *(blockPointer->data[insideBlockPosition]);
            }

            T *operator->() const noexcept {
                if (blockPointer == dequePointer->tail || blockPointer->data[insideBlockPosition] == nullptr) throw 1;
                return blockPointer->data[insideBlockPosition];
            }

            bool operator==(const iterator &rhs) const {
                return dequePointer == rhs.dequePointer && position == rhs.position;
            }

            bool operator==(const const_iterator &rhs) const {
                return dequePointer == rhs.dequePointer && position == rhs.position;
            }

            bool operator!=(const iterator &rhs) const {
                return dequePointer != rhs.dequePointer || position != rhs.position;

            }

            bool operator!=(const const_iterator &rhs) const {
                return dequePointer != rhs.dequePointer || position != rhs.position;
            }
        };

        deque() {
            head = new Block;
            tail = new Block;
            head->next = tail;
            tail->prev = head;
            totalLength = 0;
        }

        deque(const deque &other) {
            if (other.totalLength == 0) {
                head = new Block;
                tail = new Block;
                head->next = tail;
                tail->prev = head;
                totalLength = 0;
            } else {
                head = new Block;
                tail = new Block;
                Block *currentBlock = head;
                Block *currentCopyBlock = other.head->next;
                while (currentCopyBlock != other.tail) {
                    Block *newBlock = new Block(*currentCopyBlock);
                    currentBlock->next = newBlock;
                    newBlock->prev = currentBlock;
                    currentCopyBlock = currentCopyBlock->next;
                    currentBlock = currentBlock->next;
                }
                currentBlock->next = tail;
                tail->prev = currentBlock;
                totalLength = other.totalLength;
            }
        }

        ~deque() {
            clear();
            delete head;
            delete tail;
        }

        deque &operator=(const deque &other) {
            if (this == &other) return *this;
            clear();
            totalLength = other.totalLength;
            if (totalLength == 0) {
                head->next = tail;
                tail->prev = head;
                return *this;
            }
            Block *currentBlock = head;
            Block *currentCopyBlock = other.head->next;
            while (currentCopyBlock != other.tail) {
                Block *newBlock = new Block(*currentCopyBlock);
                currentBlock->next = newBlock;
                newBlock->prev = currentBlock;
                currentCopyBlock = currentCopyBlock->next;
                currentBlock = currentBlock->next;
            }
            currentBlock->next = tail;
            tail->prev = currentBlock;
            totalLength = other.totalLength;
            return *this;
        }

        T &at(const size_t &pos) {
            if (pos < 0 || pos >= totalLength) throw index_out_of_bound();
            int tmppos = pos;
            Block *currentBlock = head->next;
            while (tmppos >= currentBlock->length) {
                tmppos -= currentBlock->length;
                currentBlock = currentBlock->next;
            }
            return *(currentBlock->data[tmppos]);
        }

        const T &at(const size_t &pos) const {
            if (pos < 0 || pos >= totalLength) throw index_out_of_bound();
            int tmppos = pos;
            Block *currentBlock = head->next;
            while (tmppos >= currentBlock->length) {
                tmppos -= currentBlock->length;
                currentBlock = currentBlock->next;
            }
            return *(currentBlock->data[tmppos]);
        }

        T &operator[](const size_t &pos) {
            if (pos < 0 || pos >= totalLength) throw index_out_of_bound();
            int tmppos = pos;
            Block *currentBlock = head->next;
            while (tmppos >= currentBlock->length) {
                tmppos -= currentBlock->length;
                currentBlock = currentBlock->next;
            }
            return *(currentBlock->data[tmppos]);
        }

        const T &operator[](const size_t &pos) const {
            if (pos < 0 || pos >= totalLength) throw index_out_of_bound();
            int tmppos = pos;
            Block *currentBlock = head->next;
            while (tmppos >= currentBlock->length) {
                tmppos -= currentBlock->length;
                currentBlock = currentBlock->next;
            }
            return *(currentBlock->data[tmppos]);
        }

        const T &front() const {
            if (totalLength == 0) throw 1;
            return *(head->next->data[0]);
        }

        const T &back() const {
            if (totalLength == 0) throw 1;
            return *(tail->prev->data[tail->prev->length - 1]);
        }

        iterator begin() {
            return iterator(this, 0);
        }

        const_iterator cbegin() const {
            return const_iterator(this, head->next, 0, 0);
        }

        iterator end() {
            return iterator(this, tail->prev, totalLength, tail->prev->length);
        }

        const_iterator cend() const {
            return const_iterator(this, tail->prev, totalLength, tail->prev->length);

        }

        bool empty() const { return totalLength == 0; }

        size_t size() const { return totalLength; }

        void clear() {
            if (totalLength == 0) return;
            Block *currentBlock = head->next;
            while (currentBlock != tail) {
                Block *tmp = currentBlock;
                currentBlock = currentBlock->next;
                delete tmp;
            }
            head->next = tail;
            tail->prev = head;
            totalLength = 0;
        }

        bool isThatMergeable(Block *lhs, Block *rhs) {
            if (lhs == nullptr || rhs == nullptr || lhs == head || rhs == head || lhs == tail || rhs == tail)
                return false;
            return lhs->length + rhs->length < lhs->maxLengthBlock;
        }

        void mergeBlocks(Block *lhs, Block *rhs) {
            for (int i = 0; i < rhs->length; ++i)
                lhs->data[lhs->length + i] = new T(*(rhs->data[i]));
            lhs->length += rhs->length;
            Block *nextBlock = rhs->next;
            nextBlock->prev = lhs;
            lhs->next = nextBlock;
            delete rhs;
        }

        Block *split(Block *block, const int &position) {
            Block *newBlock = new Block;
            newBlock->length = block->length - position;
            for (int i = 0; i < newBlock->length; i++)
                newBlock->data[i] = new T(*(block->data[position + i]));
            for (int i = position; i < block->length; i++)
                delete block->data[i], block->data[i] = nullptr;
            block->length -= newBlock->length;
            Block *nextBlock = block->next;
            block->next = newBlock;
            newBlock->next = nextBlock;
            nextBlock->prev = newBlock;
            newBlock->prev = block;
            return block;
        }

        void deleteItem(Block *block, const int &position) {
            delete block->data[position];
            for (int i = position; i < block->length - 1; i++)
                block->data[i] = block->data[i + 1];
            block->data[block->length - 1] = nullptr;
            block->length--;
        }

        void insertItem(Block *block, int position, const T &value) {
            for (int i = block->length; i > position; i--)
                block->data[i] = block->data[i - 1];
            block->length++;
            block->data[position] = new T(value);
        }

        iterator insert(iterator pos, const T &value) {
            if (pos.dequePointer != this || pos.blockPointer == nullptr || pos.position > totalLength ||
                pos.position < 0)
                throw 1;
            if (totalLength == 0) {
                Block *newBlock = new Block(value);
                head->next = newBlock;
                newBlock->next = tail;
                tail->prev = newBlock;
                newBlock->prev = head;
                totalLength++;
                return begin();
            }
            if (pos.blockPointer != pos.dequePointer->tail &&
                pos.blockPointer->length + 1 <= maxLengthOfBlock) {
                totalLength++;
                insertItem(pos.blockPointer, pos.insideBlockPosition, value);
                return pos;
            }
            totalLength++;
            Block *newBlock = new Block(value);
            Block *prevBlock, *nextBlock;
            if (pos.insideBlockPosition == 0) {
                prevBlock = pos.blockPointer->prev;
                nextBlock = pos.blockPointer;
            } else {
                prevBlock = split(pos.blockPointer, pos.insideBlockPosition);
                nextBlock = prevBlock->next;
            }
            prevBlock->next = newBlock;
            newBlock->next = nextBlock;
            nextBlock->prev = newBlock;
            newBlock->prev = prevBlock;
            if (isThatMergeable(prevBlock, newBlock)) mergeBlocks(prevBlock, newBlock);
            if (isThatMergeable(nextBlock->prev, nextBlock)) mergeBlocks(nextBlock->prev, nextBlock);
            return iterator(this, pos.position);
        }

        iterator erase(iterator pos) {
            if (pos.dequePointer == nullptr) throw index_out_of_bound();
            if (pos.dequePointer->empty()) throw 1;
            if (pos.dequePointer != this) throw 1;
            if (pos.position >= totalLength) throw 1;
            if (pos.position < 0) throw 1;

            if (totalLength == 1) {
                Block *requireToDelete = head->next;
                head->next = tail;
                tail->prev = head;
                delete requireToDelete;
                totalLength--;
                return end();
            }
            if (pos.blockPointer->length == 1) {
                Block *prevBlock = pos.blockPointer->prev;
                Block *nextBlock = pos.blockPointer->next;
                prevBlock->next = nextBlock;
                nextBlock->prev = prevBlock;
                delete pos.blockPointer;
                if (isThatMergeable(prevBlock, nextBlock)) mergeBlocks(prevBlock, nextBlock);
                totalLength--;
                return iterator(this, pos.position);
            }
            Block *prevBlock = pos.blockPointer->prev;
            Block *nextBlock = pos.blockPointer->next;
            deleteItem(pos.blockPointer, pos.insideBlockPosition);
            if (isThatMergeable(prevBlock, pos.blockPointer)) mergeBlocks(prevBlock, pos.blockPointer);
            else if (isThatMergeable(pos.blockPointer, nextBlock)) mergeBlocks(pos.blockPointer, nextBlock);
            totalLength--;
            return iterator(this, pos.position);
        }

        void push_back(const T &value) {
            totalLength++;
            if (totalLength == 1) {
                Block *newBlock = new Block(value);
                head->next = newBlock;
                newBlock->next = tail;
                tail->prev = newBlock;
                newBlock->prev = head;
//                totalLength++;
                return;
            }
            if (tail->prev->length + 1 <= maxLengthOfBlock) insertItem(tail->prev, tail->prev->length, value);
            else {
                Block *newBlockAtTail = new Block(value);
                Block *prev = tail->prev;
                prev->next = newBlockAtTail;
                newBlockAtTail->next = tail;
                tail->prev = newBlockAtTail;
                newBlockAtTail->prev = prev;
//                totalLength++;
            }
        }

        void pop_back() {
            if (totalLength == 0) throw 1;
            if (totalLength == 1) {
                Block *requireToDelete = head->next;
                head->next = tail;
                tail->prev = head;
                delete requireToDelete;
                totalLength--;
                return;
            }

            if (tail->prev->length == 1) {
                Block *newTailPrevBlock = tail->prev->prev;
                Block *requireToDelete = tail->prev;
                newTailPrevBlock->next = tail;
                tail->prev = newTailPrevBlock;
                delete requireToDelete;
                totalLength--;
                return;
            }
            deleteItem(tail->prev, tail->prev->length - 1);
            totalLength--;
        }

        void push_front(const T &value) {
            if (totalLength == 0) {
                Block *newBlock = new Block(value);
                head->next = newBlock;
                newBlock->next = tail;
                tail->prev = newBlock;
                newBlock->prev = head;
                totalLength++;
                return;
            }
            if (head->next->length + 1 <= maxLengthOfBlock) {
                insertItem(head->next, 0, value);
                totalLength++;
                return;
            }
            Block *newBlock = new Block(value);
            Block *currentNextBlock = head->next;
            head->next = newBlock;
            newBlock->next = currentNextBlock;
            currentNextBlock->prev = newBlock;
            newBlock->prev = head;
            totalLength++;
            return;
        }

        void pop_front() {
            if (totalLength == 0) throw container_is_empty();
            if (totalLength == 1) {
                Block *requireToDelete = head->next;
                head->next = tail;
                tail->prev = head;
                delete requireToDelete;
                totalLength--;
                return;
            }
            if (head->next->length == 1) {
                Block *nextBlock = head->next->next;
                Block *requireToDelete = head->next;
                head->next = nextBlock;
                nextBlock->prev = head;
                delete requireToDelete;
                totalLength--;
            } else deleteItem(head->next, 0), totalLength--;
        }
    };

}

#endif
