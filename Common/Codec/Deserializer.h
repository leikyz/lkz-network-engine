#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include <vector>
#include <cstdint>
#include <stdexcept>

// Little Endian Deserializer

class Deserializer 
{
private:
   
    size_t m_position = 0;

public:
    explicit Deserializer(const std::vector<uint8_t>& buf) : m_buffer(buf) {}
    const std::vector<uint8_t>& m_buffer;


	// Reads an integer (4 bytes) from the buffer
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

	// Reads a float (4 bytes) from the buffer
    float readFloat()
    {
        if (m_position + sizeof(float) > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readFloat");
        }

        float value;

		// Use memcpy to avoid strict aliasing issues
        std::memcpy(&value, &m_buffer[m_position], sizeof(float));
        m_position += sizeof(float);
        return value;
    }

	// Reads a boolean (1 byte) from the buffer
    bool readBool()
    {
        if (m_position + 1 > m_buffer.size()) {
            throw std::out_of_range("Buffer overflow in readBool");
        }

        bool value = m_buffer[m_position] != 0;
        m_position += 1;
        return value;
    }

	// Reads an unsigned short (2 bytes) from the buffer
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

	// Reads an unsigned byte (1 byte) from the buffer
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
