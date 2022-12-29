#include "Message.h"
#include <cstring>

using namespace thisptr::broker;

template<>
void thisptr::broker::serializeBody(const MessageResponse_t &data, void *ptr) {
  memcpy(ptr, (void *)&data, sizeof(MessageResponse_t));
}
