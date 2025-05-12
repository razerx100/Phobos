#include <Base64Encoder.hpp>

namespace Phobos
{
void Loader24bits::LoadData(void const* dataHandle, size_t byteCount) noexcept
{
	std::uint32_t data = 0u;

	auto dataU8 = static_cast<std::uint8_t const*>(dataHandle);

	if (byteCount == 1u)
	{
		data = *dataU8;

		data <<= 16u;
	}
	else if (byteCount == 2u)
	{
		data = *(dataU8);

		data <<= 8u;

		data |= *(dataU8 + 1u);

		data <<= 8u;
	}
	else if (byteCount == 3u)
	{
		data = *(dataU8);

		data <<= 8u;

		data |= *(dataU8 + 1u);

		data <<= 8u;

		data |= *(dataU8 + 2u);
	}

	m_data = data;

	m_validByteCount = static_cast<std::uint32_t>(byteCount);
}

bool Loader24bits::IsByteValid(std::uint32_t index) const noexcept
{
	return index < m_validByteCount;
}
}
