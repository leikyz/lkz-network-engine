//#ifndef PLAYER_STATE_MESSAGE_H
//#define PLAYER_STATE_MESSAGE_H
//
//#include "LKZ/Protocol/Message/Message.h"
//
//struct PlayerStateMessage : public Message
//{
//    static constexpr uint8_t ID = 16;
//
//    PlayerStateMessage();
//    PlayerStateMessage(uint16_t entityId, uint8_t flags);
//
//    uint16_t entityId = 0;
//	uint8_t flags = 0; // Bitmask for isArmed, isAiming, isRunning
//
//	bool isArmed() const { return flags & 0x01; }
//	bool isAiming() const { return flags & 0x02; }
//	bool isRunning() const { return flags & 0x04; }
//
//    uint8_t getId() const override;
//
//    std::vector<uint8_t>& serialize(Serializer& serializer) const override;
//    void deserialize(Deserializer& deserializer) override;
//    void process(const sockaddr_in& senderAddr, SOCKET tcpSocket) override;
//};
//
//#endif // PLAYER_STATE_MESSAGE_H
//