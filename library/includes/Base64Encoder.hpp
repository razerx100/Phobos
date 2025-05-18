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
	// the EncodeBase64 function at the bottom should be used instead. Also
	// only loads 24bits/3 bytes.
	void LoadData(void const* dataHandle, size_t byteCount) noexcept;

	[[nodiscard]]
	bool IsByteValid(size_t index) const noexcept;

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

[[nodiscard]]
std::vector<std::uint8_t> EncodeBase64(
	void const* dataHandle, size_t elementCount, size_t primitiveSize
) noexcept;
}
#endif
