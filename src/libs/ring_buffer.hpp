#pragma once
#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

namespace utils {
void fit_to_range(int min, int* v, int max) {
  while (*v > max) *v -= max;
  while (*v < 0) *v += max;
}
int fit_to_range(int min, int v, int max) {
  int temp = v;
  fit_to_range(min, &temp, max);
  return temp;
}
}  // namespace utils

template <int max>
class CyclicCounter {
  int value;

 public:
  CyclicCounter(int v = 0) : value(v) {}

  int ++operator() {
    this.value += 1;
    if (this.value == max) this.value = 0;
    return this.value;
  }

  int --operator() {
    if (this.value == 0) this.value = max;
    this.value -= 1;
    return this.value;
  }

  int operator=(int value2) {
    this.value = utils::fit_to_range(0, value2, max);
  }
  operator(int) { return this.value; }
};

template <int N, typename T>
class RingBuffer {
 private:
  T buffer[N];
  CyclicCounter<N> head;
  CyclicCounter<N> tail;
  bool is_max;
  bool is_empty;

 public:
  RingBuffer() : buffer{}, head(0), tail(0), is_max(false), is_empty(true) {}

  bool IsEmpty() { return this->is_empty; }

  [[nodiscard]] bool push(T& elem) {
    if (this->is_max) {
      return false;
    }

    this->buffer[this->tail++] = elem;

    this->is_empty = false;
    this->is_max = this.head == this.tail;
    return true;
  }

  T* pop(T& elem) {
    if (this->is_empty) {
      return nullptr;
    }

    elem = this->buffer[this->head++];

    this->is_empty = this.head == this.tail;
    this->is_max = false;

    return &elem;
  }
};

#endif