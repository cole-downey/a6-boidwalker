#include "PCBuffer.h"

#include <thread>
#include <queue>
#include <string>
#include <iostream>

using namespace std;

PCBuffer::PCBuffer(int _size) : mutex(1), full(0), empty(_size), size(_size) { // what should empty be init to?
  size = _size;
  buffer = new queue<string>();
}

PCBuffer::~PCBuffer() {
    delete buffer;
}

int PCBuffer::deposit(string _item) {
  empty.P();
  //cout << "h" << endl;
  mutex.P();

  buffer->push(_item);

  mutex.V();
  full.V();
  return size;
}

string PCBuffer::retrieve() {
  full.P();
  mutex.P();

  string s = buffer->front();
  buffer->pop();

  mutex.V();
  empty.V();
  return s;
}