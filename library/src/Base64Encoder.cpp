#include <array>
#include <Base64Encoder.hpp>

namespace Phobos
{
static constexpr std::array s_characterMap
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
	't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '+', '/'
};

void Encoder24bits::LoadData(void const* dataHandle, size_t byteCount) noexcept
{
	std::uint32_t data = 0u;

	auto dataU8 = static_cast<std::uint8_t const*>(dataHandle);

	if (byteCount == 3u)
	{
		data = *(dataU8);

		data <<= 8u;

		data |= *(dataU8 + 1u);

		data <<= 8u;

		data |= *(dataU8 + 2u);
	}
	else if (byteCount == 1u)
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

	m_data = data;

	m_validByteCount = static_cast<std::uint32_t>(byteCount);
}

bool Encoder24bits::IsByteValid(size_t index) const noexcept
{
	return index < m_validByteCount;
}

size_t Encoder24bits::Get6BitValue(size_t index) const noexcept
{
	auto bitOffset = static_cast<std::int64_t>(23u - (index * 6u));

	std::int64_t endBit = bitOffset - 6;

	size_t outputValue = 0u;

	for (; bitOffset > endBit; --bitOffset)
	{
		outputValue <<= 1u;

		outputValue |= static_cast<size_t>(m_data.test(bitOffset));
	}

	return outputValue;
}

char Encoder24bits::Encode6bits(size_t index) const noexcept
{
	return s_characterMap[Get6BitValue(index)];
}

char Encoder24bits::Encode6bitsWithCheck(size_t index) const noexcept
{
	char encodedChar = '=';

	const size_t byteIndex = index ? index - 1u : 0u;

	if (IsByteValid(byteIndex))
		encodedChar = Encode6bits(index);

	return encodedChar;
}

std::vector<std::uint8_t> Encode(void const* dataHandle, size_t sizeInBytes) noexcept
{
	const size_t bitCount        = sizeInBytes * 8u;
	const size_t bits24Count     = bitCount / 24u;
	const size_t bits24Remainder = bitCount % 24u;

	size_t output24BitsCount = bits24Count;

	// If the size in bits isn't an exponent of 24, then we need to add padding and make it so.
	// So, there will be an extra 24bits if there is a remainder.
	if (bits24Remainder)
		++output24BitsCount;

	// Each 24bits will be divided into four 6 bits, each represented with a character.
	const size_t encodedCharCount = output24BitsCount * 4u;

	std::vector<std::uint8_t> encodedData(encodedCharCount, 0u);

	size_t dataOffset       = 0u;
	size_t encodedCharIndex = 0u;
	auto dataHandleU8       = static_cast<std::uint8_t const*>(dataHandle);

	for (size_t index = 0u; index < bits24Count; ++index)
	{
		Encoder24bits encoder{};

		// We are gonna process 24bits or 3bytes at a time.
		encoder.LoadData(dataHandleU8 + dataOffset, 3u);

		// Only need to check for validity on the last few bits, as that might not be 24bits.
		// Which I will do outside of the loop.
		encodedData[encodedCharIndex] = encoder.Encode6bits(0u);
		++encodedCharIndex;

		encodedData[encodedCharIndex] = encoder.Encode6bits(1u);
		++encodedCharIndex;

		encodedData[encodedCharIndex] = encoder.Encode6bits(2u);
		++encodedCharIndex;

		encodedData[encodedCharIndex] = encoder.Encode6bits(3u);
		++encodedCharIndex;

		dataOffset += 24u;
	}

	// Check the last few bits separately as there might not be 24bits left.
	if (bits24Remainder)
	{
		Encoder24bits encoder{};

		const size_t byteRemainder = bits24Remainder / 8u;

		encoder.LoadData(dataHandleU8 + dataOffset, byteRemainder);

		encodedData[encodedCharIndex] = encoder.Encode6bitsWithCheck(0u);
		++encodedCharIndex;

		encodedData[encodedCharIndex] = encoder.Encode6bitsWithCheck(1u);
		++encodedCharIndex;

		encodedData[encodedCharIndex] = encoder.Encode6bitsWithCheck(2u);
		++encodedCharIndex;

		encodedData[encodedCharIndex] = encoder.Encode6bitsWithCheck(3u);
	}

	return encodedData;
}
}
