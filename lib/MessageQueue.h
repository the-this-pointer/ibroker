#ifndef IBROKER_THREADSAFEQUEUE_H
#define IBROKER_THREADSAFEQUEUE_H

#include <condition_variable>
#include <mutex>
#include <deque>
#include <iterator>
#include <cstddef>

template <typename T>
class MessageQueue: public std::deque<T> {
public:
  MessageQueue() = default;
  MessageQueue(const MessageQueue<T>&) = delete;
  ~MessageQueue() {
    clear();
  }

  const T& front()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return std::deque<T>::front();
  }

  const T& back()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return std::deque<T>::back();
  }

  T pop_front()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    auto t = std::move(std::deque<T>::front());
    std::deque<T>::pop_front();
    return t;
  }

  T pop_back()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    auto t = std::move(std::deque<T>::back());
    std::deque<T>::pop_back();
    return t;
  }

  void push_back(const T& item)
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    std::deque<T>::emplace_back(std::move(item));
  }

  void push_front(const T& item)
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    std::deque<T>::emplace_front(std::move(item));
  }

  bool empty()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return std::deque<T>::empty();
  }

  size_t count()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return std::deque<T>::size();
  }

  void clear()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    std::deque<T>::clear();
  }

private:
  std::mutex m_queueMutex;
  std::deque<T> m_queue;
};

#endif //IBROKER_THREADSAFEQUEUE_H
