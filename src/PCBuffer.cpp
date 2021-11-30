#include "PCBuffer.h"

#include <thread>
#include <queue>
#include <string>
#include <iostream>

using namespace std;

template <class T>
PCBuffer<T>::PCBuffer(int _size) : mutex(1), full(0), empty(_size), size(_size) { // what should empty be init to?
  size = _size;
  buffer = new queue<T>();
}

template <class T>
PCBuffer<T>::~PCBuffer() {
  delete buffer;
}

template <class T>
int PCBuffer<T>::deposit(T item) {
  empty.P();
  mutex.P();

  buffer->push(item);

  mutex.V();
  full.V();
  return size;
}

template <class T>
T PCBuffer<T>::retrieve() {
  full.P();
  mutex.P();

  T item = buffer->front();
  buffer->pop();

  mutex.V();
  empty.V();
  return item;
}