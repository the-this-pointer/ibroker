
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../lib/Message.h"

TEST_CASE("serializing and deserializing message", "[message]") {
  SECTION("simple serialize") {
    Message msg;
    msg.type = queueDeclare;
    msg.size = 10;
    std::string data = "hi there!";
    memcpy(msg.payload, data.data(), data.length());

    MessagePacket packet(msg);

    std::string strPacket = static_cast<std::string>(packet);

    MessagePacket dpacket(msg);
    dpacket.fromString(strPacket);

    REQUIRE(msg.type == queueDeclare);
    REQUIRE(msg.size == 10);
    REQUIRE(std::string((const char*)msg.payload, msg.size-1) == data);
  }
  SECTION("serialize with message indicators") {
    Message msg;
    msg.type = queueDeclare;
    msg.size = 10; // should change to correct value during serialization.
    std::string data = "hi there!";
    memcpy(msg.payload, data.data(), data.length());

    MessagePacket packet(msg, true);

    std::string strPacket = static_cast<std::string>(packet);

    MessagePacket dpacket(msg);
    dpacket.fromString(strPacket);

    REQUIRE(msg.type == queueDeclare);
    REQUIRE(msg.size == 10);
    REQUIRE(std::string((const char*)msg.payload, msg.size-1) == data);
  }
}
