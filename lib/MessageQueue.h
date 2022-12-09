#ifndef IBROKER_THREADSAFEQUEUE_H
#define IBROKER_THREADSAFEQUEUE_H

#include <condition_variable>
#include <mutex>
#include <deque>

template <typename T>
class MessageQueue {
public:
  MessageQueue() = default;
  MessageQueue(const MessageQueue<T>&) = delete;
  ~MessageQueue() {
    clear();
  }

  const T& front()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_queue.front();
  }

  const T& back()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_queue.back();
  }

  T pop_front()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    auto t = std::move(m_queue.front());
    m_queue.pop_front();
    return t;
  }

  T pop_back()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    auto t = std::move(m_queue.back());
    m_queue.pop_back();
    return t;
  }

  void push_back(const T& item)
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.emplace_back(std::move(item));
  }

  void push_front(const T& item)
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.emplace_front(std::move(item));
  }

  bool empty()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_queue.empty();
  }

  size_t count()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_queue.size();
  }

  void clear()
  {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_queue.clear();
  }

private:
  std::mutex m_queueMutex;
  std::deque<T> m_queue;
};

#endif //IBROKER_THREADSAFEQUEUE_H
