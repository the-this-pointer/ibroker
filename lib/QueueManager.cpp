#include "QueueManager.h"
#include <iostream>

using namespace thisptr::broker;

std::shared_ptr<QueueManager> thisptr::broker::QueueManager::m_ins = nullptr;

bool QueueManager::newQueue(const std::string &name, const std::string &bindingKey) {
  std::unique_lock<std::mutex> lk(m_mutex);
  if (m_queues.find(name) != m_queues.end())
    return false;

  std::shared_ptr<Queue> queue = std::make_shared<Queue>();

  if (m_queueBindings.find(bindingKey) == m_queueBindings.end()) {
    m_trie.insert(bindingKey);
  }

  m_queues[name] = queue;
  m_queueBindings.insert({bindingKey, name});
  return true;
}

void QueueManager::publish(const std::string& bindingKey, const std::shared_ptr<MessagePacket> &packet) {
  auto matches = m_trie.parse_text(bindingKey);
  for(const auto& match: matches) {
    std::cout << "[matcher] match found: " << match.first.get_keyword() << std::endl;
    for (const auto& binding: m_queueBindings)
      if (binding.first == match.first.get_keyword()) {
        m_queues[binding.second]->publish(packet);
      }
  }
}

std::shared_ptr<Queue> QueueManager::bind(const std::string &name) {
  if (m_queues.find(name) == m_queues.end())
    return nullptr;
  return m_queues[name];
}
