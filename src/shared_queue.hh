#pragma once

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <ostream>
#include <queue>

/**
 * Implementation of a thread-safe Queue. Code from
 * https://stackoverflow.com/questions/36762248/why-is-stdqueue-not-thread-safe
 */
template <typename T>
class SharedQueue {
 public:
  SharedQueue() = default;
  ~SharedQueue() = default;

  SharedQueue(SharedQueue&& q) {
    if (this == &q) return;
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_ = q.queue_;
    mutex_ = q.mutex_;
    cond_ = q.cond_;
  }

  auto operator=(SharedQueue&& q) -> SharedQueue& {
    if (this == &q) return;
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_ = q.queue_;
    mutex_ = q.mutex_;
    cond_ = q.cond_;
  }

  auto front() -> T&;
  auto pop_front() -> T&;

  void push_back(const T& item);
  void push_back(T&& item);
  void clear();

  auto size() -> size_t;
  auto empty() -> bool;

 private:
  std::deque<T> queue_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

template <typename T>
auto SharedQueue<T>::front() -> T& {
  std::unique_lock<std::mutex> mlock(mutex_);
  while (queue_.empty()) {
    cond_.wait(mlock);
  }
  return queue_.front();
}

template <typename T>
auto SharedQueue<T>::pop_front() -> T& {
  std::unique_lock<std::mutex> mlock(mutex_);
  while (queue_.empty()) {
    cond_.wait(mlock);
  }
  T& front = queue_.front();
  queue_.pop_front();
  return front;
}

template <typename T>
void SharedQueue<T>::push_back(const T& item) {
  std::unique_lock<std::mutex> mlock(mutex_);
  queue_.push_back(item);
  mlock.unlock();      // unlock before notification to minimize mutex con
  cond_.notify_one();  // notify one waiting thread
}

template <typename T>
void SharedQueue<T>::push_back(T&& item) {
  std::unique_lock<std::mutex> mlock(mutex_);
  queue_.push_back(std::move(item));
  mlock.unlock();      // unlock before notification to minimize mutex con
  cond_.notify_one();  // notify one waiting thread
}

template <typename T>
auto SharedQueue<T>::size() -> size_t {
  std::unique_lock<std::mutex> mlock(mutex_);
  size_t size = queue_.size();
  mlock.unlock();
  return size;
}

template <typename T>
auto SharedQueue<T>::empty() -> bool {
  return size() == 0;
}

/**
 * Empties the queue
 */
template <typename T>
void SharedQueue<T>::clear() {
  std::unique_lock<std::mutex> mlock(mutex_);
  std::deque<T>().swap(queue_);
  mlock.unlock();
  cond_.notify_one();
}
