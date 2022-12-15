#ifndef IBROKER_QUEUE_H
#define IBROKER_QUEUE_H

#include "Message.h"
#include "MessageQueue.h"
#include <mutex>
#include <memory>

class Queue {
public:
  virtual void publish(const std::shared_ptr<MessagePacket>& packet)
  {
    std::unique_lock<std::mutex> lk(m_mutex);
    m_messages.push_back(packet);
  }

protected:
  MessageQueue<std::shared_ptr<MessagePacket>> m_messages;
  std::mutex m_mutex;
};

#endif //IBROKER_QUEUE_H
