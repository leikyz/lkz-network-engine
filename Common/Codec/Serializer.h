#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <vector>
#include <cstdint>

// Little Endian Serializer
class Serializer 
{
private :
    std::vector<uint8_t> m_buffer;
public:
    
	// Writes an integer (4 bytes) to the buffer
    void writeInt(int value) 
    {
        m_buffer.push_back(static_cast<uint8_t>(value & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    }

	// Writes an unsigned integer (1 byte) to the buffer
    void writeUInt8(uint8_t value) 
    {
        m_buffer.push_back(value); 
    }

	// Writes an unsigned integer (2 bytes) to the buffer
    void writeUInt16(uint16_t value) 
    {
        m_buffer.push_back(static_cast<uint8_t>(value & 0xFF)); 
        m_buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    }

	// Writes an unsigned integer (4 bytes) to the buffer
    void writeUInt32(uint32_t value) 
    {
        m_buffer.push_back(static_cast<uint8_t>(value & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        m_buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    }

	// Writes a float (4 bytes) to the buffer
    void writeFloat(float value) 
    {
        uint8_t bytes[sizeof(float)];
        std::memcpy(bytes, &value, sizeof(float)); 
        m_buffer.insert(m_buffer.end(), bytes, bytes + sizeof(float));
    }

	// Writes a boolean (1 byte) to the buffer
    void writeBool(bool value) 
    {
        m_buffer.push_back(value ? 1 : 0);
    }

	// Writes a byte (1 byte) to the buffer
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
