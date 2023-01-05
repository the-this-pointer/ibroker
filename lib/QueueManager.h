#ifndef IBROKER_QUEUEMANAGER_H
#define IBROKER_QUEUEMANAGER_H

#include <unordered_map>
#include <set>
#include <map>
#include <mutex>
#include <string>
#include <memory>
#include <aho_corasick.hpp>
#include <utility>
#include "Queue.h"

namespace thisptr {
  namespace broker {
    class TestQueueManager;

    class QueueManager {
    public:
      static std::shared_ptr<QueueManager> instance() {
        if (!m_ins)
          m_ins = std::make_shared<QueueManager>();
        return m_ins;
      }

      QueueManager() = default;

      ~QueueManager() = default;

      bool newQueue(const std::string &name, const std::string &bindingKey);

      void publish(const std::string &bindingKey, const std::shared_ptr<MessagePacket> &packet);

      std::shared_ptr<Queue> bind(const std::string &name);

    protected:
      std::mutex m_mutex;
      // map of queue names and queues
      std::unordered_map<std::string, std::shared_ptr<Queue>> m_queues;
      // map of binding keys and queue names
      std::multimap<std::string, std::string> m_queueBindings;

      aho_corasick::trie m_trie;
    private:
      static std::shared_ptr<QueueManager> m_ins;

      friend class TestQueueManager;
    };

    class TestQueueManager {
    public:
      explicit TestQueueManager(std::shared_ptr<QueueManager> qm) : m_qm(std::move(qm)) {}

      std::unordered_map<std::string, std::shared_ptr<Queue>> &queues() { return m_qm->m_queues; }

      std::multimap<std::string, std::string> &queueBindings() { return m_qm->m_queueBindings; }

    private:
      std::shared_ptr<QueueManager> m_qm;
    };
  }
}

#endif //IBROKER_QUEUEMANAGER_H
