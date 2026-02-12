#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <span>

// Big Endian Deserializer (Network Byte Order)
class Deserializer
{
private:
    std::span<const uint8_t> m_buffer;
    size_t m_position = 0;

public:

    explicit Deserializer(std::span<const uint8_t> buf)
        : m_buffer(buf) {
    }

    const uint8_t* data() const { return m_buffer.data(); }
    size_t size() const { return m_buffer.size(); }
    size_t remaining() const { return m_buffer.size() - m_position; }

    // Read a signed int (4 bytes) big endian
    int32_t readInt()
    {
        return static_cast<int32_t>(readUInt32());
    }

    // Read an unsigned int (4 bytes) big endian
    uint32_t readUInt32()
    {
        if (m_position + 4 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readUInt32");
        }

        uint32_t value =
            (static_cast<uint32_t>(m_buffer[m_position]) << 24) |
            (static_cast<uint32_t>(m_buffer[m_position + 1]) << 16) |
            (static_cast<uint32_t>(m_buffer[m_position + 2]) << 8) |
            (static_cast<uint32_t>(m_buffer[m_position + 3]));

        m_position += 4;
        return value;
    }

    // Read an unsigned short (2 bytes) big endian
    uint16_t readUInt16()
    {
        if (m_position + 2 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readUInt16");
        }

        uint16_t value =
            (static_cast<uint16_t>(m_buffer[m_position]) << 8) |
            (static_cast<uint16_t>(m_buffer[m_position + 1]));

        m_position += 2;
        return value;
    }

    // Read a float (4 bytes) big endian
    float readFloat()
    {
        static_assert(sizeof(float) == sizeof(uint32_t), "Unexpected float size");

        uint32_t temp = readUInt32();
        float value;
        std::memcpy(&value, &temp, sizeof(float));
        return value;
    }

    // Read a bool (1 byte)
    bool readBool()
    {
        if (m_position + 1 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readBool");
        }

        bool value = m_buffer[m_position] != 0;
        m_position += 1;
        return value;
    }

    // Read a byte (1 byte)
    uint8_t readByte()
    {
        if (m_position + 1 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readByte");
        }

        return m_buffer[m_position++];
    }
};

#endif // DESERIALIZER_H
