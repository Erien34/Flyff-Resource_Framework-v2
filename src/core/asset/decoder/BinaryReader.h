#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <optional>
#include <type_traits>

namespace asset::decode
{
    // Minimal binary reader for "sniffing" / simple header parsing.
    // (Full format parsing can be layered on top later.)
    class BinaryReader
    {
    public:
        explicit BinaryReader(const std::vector<unsigned char>& bytes)
            : m_bytes(bytes)
        {}

        size_t size() const { return m_bytes.size(); }
        size_t tell() const { return m_pos; }
        bool eof() const { return m_pos >= m_bytes.size(); }

        bool seek(size_t pos)
        {
            if (pos > m_bytes.size()) return false;
            m_pos = pos;
            return true;
        }

        std::vector<unsigned char> peekBytes(size_t n) const
        {
            const size_t end = (m_pos + n > m_bytes.size()) ? m_bytes.size() : (m_pos + n);
            return std::vector<unsigned char>(m_bytes.begin() + static_cast<std::ptrdiff_t>(m_pos),
                                              m_bytes.begin() + static_cast<std::ptrdiff_t>(end));
        }

        std::vector<unsigned char> readBytes(size_t n)
        {
            auto out = peekBytes(n);
            m_pos += out.size();
            return out;
        }

        template<typename T>
        std::optional<T> readLE()
        {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
            if (m_pos + sizeof(T) > m_bytes.size())
                return std::nullopt;

            T v{};
            std::memcpy(&v, m_bytes.data() + m_pos, sizeof(T));
            m_pos += sizeof(T);

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
            // If you ever build on big-endian, add byte swap here.
#endif
            return v;
        }

        static std::string toHex(const std::vector<unsigned char>& bytes)
        {
            static const char* hex = "0123456789ABCDEF";
            std::string out;
            out.reserve(bytes.size() * 2);
            for (unsigned char b : bytes)
            {
                out.push_back(hex[(b >> 4) & 0xF]);
                out.push_back(hex[b & 0xF]);
            }
            return out;
        }

    private:
        const std::vector<unsigned char>& m_bytes;
        size_t m_pos = 0;
    };
} // namespace asset::decode
