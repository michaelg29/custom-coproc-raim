
#ifndef QUEUE_H
#define QUEUE_H

/** Generic wrapper for a queue. */
template <typename T>
class queue {

    public:

        /** Constructor. */
        queue(T *q, uint32_t size, uint32_t mask)
            : _q(q), _size(size), _mask(mask), _head(0), _tail(0) {}

        /** Enqueue. */
        bool enqueue(T val) {
            int i = (_tail - _head);
            i -= _size;

            // reject if full
            if (!i) return false;

            // add at tail
            i = _tail & _mask;
            _q[i] = val;

            // increment pointer
            _tail++;

            return true;
        }

        /** Dequeue. */
        bool dequeue(T &val) {
            if (peek(val)) {
                // increment pointer
                _head++;
                return true;
            }

            return false;
        }

        /** Peek the head of the queue. */
        bool peek(T &val) {
            int i = _tail - _head;

            // reject if empty
            if (!i) return false;

            // read head
            i = _head & _mask;
            val = _q[i];

            return true;
        }

    private:

        /** Cursors. */
        uint32_t _head;
        uint32_t _tail;
        uint32_t _size;
        uint32_t _mask;

        /** Buffer. */
        T *_q;

};

#endif // QUEUE_H
