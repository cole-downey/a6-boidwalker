#pragma once
#ifndef _PCBuffer_H_                   // include file only once
#define _PCBuffer_H_

#include <string>
#include <thread>
#include <queue>

#include "Semaphore.h"


class PCBuffer {
public:
  PCBuffer(int _size);
  ~PCBuffer();

  int deposit(std::string _item);
  std::string retrieve();

private:
  std::queue<std::string>* buffer;
  int size;
  Semaphore mutex;
  Semaphore full, empty;
};


#endif


