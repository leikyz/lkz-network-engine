#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <vector>
#include <cstdint>
#include <cstring>

// Big Endian Serializer (Network Byte Order)
class Serializer
{
private:
    std::vector<uint8_t> m_buffer;

public:

    // Writes a signed integer (4 bytes) in big endian
    void writeInt(int32_t value)
    {
        writeUInt32(static_cast<uint32_t>(value));
    }

    // Writes an unsigned integer (1 byte)
    void writeUInt8(uint8_t value)
    {
        m_buffer.push_back(value);
    }

    // Writes an unsigned integer (2 bytes) in big endian
    void writeUInt16(uint16_t value)
    {
        m_buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }

    // Writes an unsigned integer (4 bytes) in big endian
    void writeUInt32(uint32_t value)
    {
        m_buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>(value & 0xFF));
    }

    // Writes a float (4 bytes) in big endian (IEEE 754 assumed)
    void writeFloat(float value)
    {
        static_assert(sizeof(float) == sizeof(uint32_t), "Unexpected float size");

        uint32_t temp;
        std::memcpy(&temp, &value, sizeof(float));
        writeUInt32(temp); // ensures big endian output
    }

    // Writes a boolean (1 byte)
    void writeBool(bool value)
    {
        m_buffer.push_back(value ? 1 : 0);
    }

    // Writes a byte (1 byte)
    void writeByte(uint8_t value)
    {
        m_buffer.push_back(value);
    }

    // Returns the internal buffer
    std::vector<uint8_t>& getBuffer()
    {
        return m_buffer;
    }

    // Resets the internal buffer
    void reset()
    {
        m_buffer.clear();
    }
};

#endif // SERIALIZER_H
