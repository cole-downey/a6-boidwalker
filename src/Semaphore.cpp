#include "Semaphore.h"

#include <mutex>

#include <iostream>

using namespace std;

Semaphore::Semaphore(int _val) : value(_val) {
  //m = unique_lock<mutex>(mtx); // comes locked by default
  //m.unlock();
}

Semaphore::~Semaphore() {}

int Semaphore::P() {
  unique_lock<mutex> lock(mtx);
  value--;
  if (value < 0) {
    // block
    c.wait(lock);
  }
  return value;
}

int Semaphore::V() {
  unique_lock<mutex> lock(mtx);
  value++;
  if (value < 1) {
    // unblock waiting thread
    c.notify_one();
  }
  return value;
}