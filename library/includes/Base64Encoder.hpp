#ifndef BASE_64_ENCODER_HPP_
#define BASE_64_ENCODER_HPP_
#include <cstdint>
#include <bitset>
#include <vector>
#include <array>
#include <string>

namespace Phobos
{
class Encoder24bits
{
public:
	Encoder24bits() : m_data{ 0u }, m_validByteCount{ 0u } {}

	// Won't account for endianness. So, for any primitive larger than a byte,
	// the correct bit sized encoder should be used instead. Also
	// only loads 24bits/3 bytes.
	void LoadData(void const* dataHandle, size_t byteCount) noexcept;

	[[nodiscard]]
	bool IsByteValid(size_t index) const noexcept;
	[[nodiscard]]
	bool AreAllBytesValid() const noexcept;

	[[nodiscard]]
	char Encode6bits(size_t index) const noexcept;
	[[nodiscard]]
	char Encode6bitsWithCheck(size_t index) const noexcept;

	[[nodiscard]]
	std::array<char, 4u> Encode() const noexcept;
	[[nodiscard]]
	std::array<char, 4u> EncodeWithCheck() const noexcept;
	[[nodiscard]]
	std::string EncodeStr() const noexcept;
	[[nodiscard]]
	std::string EncodeStrWithCheck() const noexcept;

	[[nodiscard]]
	std::bitset<24u> GetData() const noexcept { return m_data; }

private:
	[[nodiscard]]
	size_t Get6BitValue(size_t index) const noexcept;

private:
	std::bitset<24u> m_data;
	std::uint32_t    m_validByteCount;
};

class Encoder16bits
{
public:
	Encoder16bits()
		: m_first{ 0u }, m_second{ 0u }, m_hasRemainingValue{ false }, m_validByteCount{ 0u }
	{}

	// The element count could be either 1 or 2. Returns the number of element loaded.
	// Since 16bits are fewer than 24bits, we can't load 2 elements fully and 8bits
	// would remain. So, on the next turn only one element will be loaded.
	size_t LoadData(void const* dataHandle, size_t elementCount) noexcept;

	[[nodiscard]]
	std::array<char, 4u> Encode() const noexcept;
	[[nodiscard]]
	std::string EncodeStr() const noexcept;

private:
	[[nodiscard]]
	Encoder24bits LoadEncoder24bits() const noexcept;

private:
	std::uint16_t m_first;
	std::uint16_t m_second;
	bool          m_hasRemainingValue;
	std::uint8_t  m_validByteCount;
};

[[nodiscard]]
std::vector<std::uint8_t> EncodeBase64(
	void const* dataHandle, size_t elementCount, size_t primitiveSize
) noexcept;
}
#endif
