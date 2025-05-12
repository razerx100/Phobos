#ifndef BASE_64_ENCODER_HPP_
#define BASE_64_ENCODER_HPP_
#include <cstdint>
#include <bitset>

namespace Phobos
{
class Loader24bits
{
public:
	Loader24bits() : m_data{ 0u }, m_validByteCount{ 0u } {}

	void LoadData(void const* dataHandle, size_t byteCount) noexcept;

	[[nodiscard]]
	bool IsByteValid(std::uint32_t index) const noexcept;

	[[nodiscard]]
	std::bitset<24u> GetData() const noexcept { return m_data; }

private:
	std::bitset<24u> m_data;
	std::uint32_t    m_validByteCount;
};
}
#endif
