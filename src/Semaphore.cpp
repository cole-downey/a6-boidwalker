#include "Semaphore.h"

#include <mutex>

#include <iostream>

using namespace std;

Semaphore::Semaphore(int _val) : value(_val) {
  m = unique_lock<mutex>(mtx); // comes locked by default
  m.unlock();
}

Semaphore::~Semaphore() {}

int Semaphore::P() {
  m.lock();
  value--;
  if (value < 0) {
    // block
    c.wait(m);
    m.unlock();
  } else {
    m.unlock();
  }
  return value;
}

int Semaphore::V() {
  m.lock();
  value++;
  if (value < 1) {
    // unblock waiting thread
    c.notify_all();
  }
  m.unlock();
  return value;
}