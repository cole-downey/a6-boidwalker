#include "Semaphore.h"

#include <mutex>

#include <iostream>

using namespace std;

Semaphore::Semaphore(int _val) : value(_val) {
  m = unique_lock<mutex>(mtx); // comes locked by default
  m.unlock();
  //c = condition_variable();
  cout << "init done" << endl;
}

Semaphore::~Semaphore() {}

int Semaphore::P() {
  m.lock();
  //cout << "P() called" << endl;
  value--;
  if (value < 0) {
    //cout << "thread blocked for now!" << endl;
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
  // cout << "V() called" << endl;
  value++;
  if (value < 1) {
    //cout << "thread unblocked!" << endl;
    // unblock waiting thread
    c.notify_all();
  }
  m.unlock();
  return value;
}