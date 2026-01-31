#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <span>

// Little Endian Deserializer using std::span for performance
class Deserializer
{
private:
    std::span<const uint8_t> m_buffer;
    size_t m_position = 0;

public:
   
    explicit Deserializer(std::span<const uint8_t> buf) : m_buffer(buf) {}

    const uint8_t* data() const { return m_buffer.data(); }
    size_t size() const { return m_buffer.size(); }
    size_t remaining() const { return m_buffer.size() - m_position; }

    // Read an int (4 bytes)
    int readInt()
    {
        if (m_position + 4 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readInt");
        }

        int value = m_buffer[m_position] |
            (m_buffer[m_position + 1] << 8) |
            (m_buffer[m_position + 2] << 16) |
            (m_buffer[m_position + 3] << 24);

        m_position += 4;
        return value;
    }

    // read a float (4 bytes)
    float readFloat()
    {
        if (m_position + sizeof(float) > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readFloat");
        }

        float value;
        std::memcpy(&value, &m_buffer[m_position], sizeof(float));
        m_position += sizeof(float);
        return value;
    }

    bool readBool()
    {
        if (m_position + 1 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readBool");
        }

        bool value = m_buffer[m_position] != 0;
        m_position += 1;
        return value;
    }

    uint16_t readUInt16()
    {
        if (m_position + 2 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readUInt16");
        }

        uint16_t value = m_buffer[m_position] |
            (m_buffer[m_position + 1] << 8);
        m_position += 2;
        return value;
    }

    uint8_t readByte()
    {
        if (m_position + 1 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readByte");
        }

        uint8_t value = m_buffer[m_position];
        m_position += 1;
        return value;
    }
};

#endif // DESERIALIZER_H