#include "ThreadSafeQueue.h"

#include <thread>
#include <channel>
#include <gtest/gtest.h>
#include <chrono>
#include <atomic>
#include "Logger.h"

using namespace Gsage;

TEST(TestThreadSafeQueue, TestSequential)
{
  ThreadSafeQueue<int> queue;
  int elements[] = {1, 2, 3};

  for(int i = 0; i < 3; i++) {
    queue << elements[i];
  }

  int element;
  int index = 0;
  while(queue.get(element) > 0) {
    ASSERT_EQ(element, elements[index++]);
  }
}

TEST(TestThreadSafeQueue, TestLimit)
{
  ThreadSafeQueue<int> queue(3);
  int elements[] = {1, 2, 3, 4, 5};

  for(int i = 0; i < 5; i++) {
    queue << elements[i];
  }

  int element;
  // should start from offset 2
  int index = 2;
  while(queue.get(element) > 0) {
    ASSERT_EQ(element, elements[index++]);
  }
}

TEST(TestThreadSafeQueue, TestParallel)
{
  std::vector<std::thread> threads;

  ThreadSafeQueue<int> queue(2048);

  int numWriters = 20;
  int numReaders = 20;

  int dataSize = 2000;

  cpp::channel<int> input;

  std::vector<int> data;

  std::atomic_bool shutdown(false);

  auto main = [&] () {
    for(int i = 0; i < dataSize; ++i) {
      input.send(i);
    }

    for(int i = 0; i < numWriters; ++i) {
      input.send(-1);
    }
    shutdown.store(true);
  };

  auto write = [&] () {
    while(!shutdown.load()) {
      int value = input.recv();
      queue << value;
      if(value == -1) {
        break;
      }
    }
  };

  std::vector<int> result;
  std::atomic_int index(0);
  result.reserve(dataSize);

  auto read = [&] () {
    int prevTail = 0;
    while(!shutdown.load() || prevTail > 0) {
      int number;
      int tail = queue.get(number);
      if(tail > 0 && number != -1) {
        int i = index.fetch_add(1, std::memory_order_relaxed);
        result[i] = number;
      }
      prevTail = tail;
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
  };

  // spawn main thread
  threads.push_back(std::thread(main));

  // start N writers
  for(int i = 0; i < numWriters; ++i) {
    threads.push_back(std::thread(write));
  }

  // start N readers
  for(int i = 0; i < numReaders; ++i) {
    threads.push_back(std::thread(read));
  }

  int count = 0;
  // wait for threads to finish
  for(std::thread& t : threads) {
    t.join();
    LOG(INFO) << ++count << " threads were stopped";
  }

  for(int i = 0; i < result.size(); ++i) {
    ASSERT_EQ(result[i], i);
  }
}
