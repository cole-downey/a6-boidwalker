#pragma once
#ifndef _PCBuffer_H_                   // include file only once
#define _PCBuffer_H_

#include <string>
#include <thread>
#include <queue>

#include "Semaphore.h"

template <class T>
class PCBuffer {
public:
  PCBuffer(int _size);
  ~PCBuffer();

  int deposit(T item);
  T retrieve();

private:
  std::queue<T>* buffer;
  int size;
  Semaphore mutex;
  Semaphore full, empty;
};



#endif


