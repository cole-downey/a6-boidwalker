#pragma once
#ifndef _Semaphore_H_
#define _Semaphore_H_

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
  Semaphore(int _val);
  ~Semaphore();

  int P();
  int V();

private:
  int value;
  std::mutex mtx;
  //std::unique_lock<std::mutex> m;
  std::condition_variable c;
};

#endif
