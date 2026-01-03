/*
 * RingBuffer.h
 *
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <cstddef>
#include <array>

template <typename T, size_t N>
class RingBuffer {
public:
    bool push( T const & Val ) {
        auto next = ( head_ + 1 ) % N;
        if ( next == tail_ ) return false;
        data_[head_] = Val;
        head_ = next;
        return true;
    }

    bool pop( T& Val ) {
        if ( tail_ == head_ ) return false;
        Val = data_[tail_];
        tail_ = ( tail_ + 1 ) % N;
        return true;
    }
private:
    std::array<T,N> data_;
    volatile size_t head_ {};
    volatile size_t tail_ {};
};



#endif /* SRC_RINGBUFFER_H_ */
