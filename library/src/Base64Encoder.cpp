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

static constexpr std::array s_6bitsOffsetMap
{
	23u, 17u, 11u, 5u
};

// Encoder 24 bits
void Encoder24Bits::LoadData(void const* dataHandle, size_t byteCount) noexcept
{
	std::uint32_t data = 0u;

	auto dataHandleU8 = static_cast<std::uint8_t const*>(dataHandle);

	if (byteCount == 3u)
	{
		memcpy(&data, dataHandleU8, 1u);

		data <<= 8u;

		memcpy(&data, dataHandleU8 + 1u, 1u);

		data <<= 8u;

		memcpy(&data, dataHandleU8 + 2u, 1u);
	}
	else if (byteCount == 1u)
	{
		memcpy(&data, dataHandle, 1u);

		data <<= 16u;
	}
	else if (byteCount == 2u)
	{
		memcpy(&data, dataHandleU8, 1u);

		data <<= 8u;

		memcpy(&data, dataHandleU8 + 1u, 1u);

		data <<= 8u;
	}

	m_data = data;

	m_validByteCount = static_cast<std::uint32_t>(byteCount);
}

bool Encoder24Bits::IsByteValid(size_t index) const noexcept
{
	return index < m_validByteCount;
}

bool Encoder24Bits::AreAllBytesValid() const noexcept
{
	return m_validByteCount == 3u;
}

size_t Encoder24Bits::Get6BitValue(size_t index) const noexcept
{
	auto bitOffset = static_cast<std::int64_t>(s_6bitsOffsetMap[index]);

	std::int64_t endBit = bitOffset - 6;

	size_t outputValue = 0u;

	for (; bitOffset > endBit; --bitOffset)
	{
		outputValue <<= 1u;

		outputValue |= static_cast<size_t>(m_data.test(bitOffset));
	}

	return outputValue;
}

char Encoder24Bits::Encode6bits(size_t index) const noexcept
{
	return s_characterMap[Get6BitValue(index)];
}

char Encoder24Bits::Encode6bitsWithCheck(size_t index) const noexcept
{
	char encodedChar = '=';

	const size_t byteIndex = index ? index - 1u : 0u;

	if (IsByteValid(byteIndex))
		encodedChar = Encode6bits(index);

	return encodedChar;
}

std::array<char, 4u> Encoder24Bits::Encode() const noexcept
{
	return
	{
		Encode6bits(0u),
		Encode6bits(1u),
		Encode6bits(2u),
		Encode6bits(3u)
	};
}

std::array<char, 4u> Encoder24Bits::EncodeWithCheck() const noexcept
{
	return
	{
		Encode6bitsWithCheck(0u),
		Encode6bitsWithCheck(1u),
		Encode6bitsWithCheck(2u),
		Encode6bitsWithCheck(3u)
	};
}

std::string Encoder24Bits::EncodeStr() const noexcept
{
	return std::string
	{
		Encode6bits(0u),
		Encode6bits(1u),
		Encode6bits(2u),
		Encode6bits(3u)
	};
}

std::string Encoder24Bits::EncodeStrWithCheck() const noexcept
{
	return std::string
	{
		Encode6bitsWithCheck(0u),
		Encode6bitsWithCheck(1u),
		Encode6bitsWithCheck(2u),
		Encode6bitsWithCheck(3u)
	};
}

// Encoder 16bits
size_t Encoder16Bits::LoadData(void const* dataHandle, size_t elementCount) noexcept
{
	size_t elementsLoaded = 0u;

	auto dataHandleU16 = static_cast<std::uint16_t const*>(dataHandle);

	constexpr bool isLittleEndian = std::endian::native == std::endian::little;

	m_validByteCount = 0u;

	if (m_hasRemainingValue)
	{
		m_first = m_second;

		++m_validByteCount;

		if (elementCount >= 1u)
		{
			m_second = *dataHandleU16;

			if constexpr (isLittleEndian)
				m_second = std::byteswap(m_second);

			m_validByteCount += 2u;

			elementsLoaded = 1u;
		}

		m_hasRemainingValue = false;
	}
	else
	{
		if (elementCount >= 1)
		{
			m_first = *dataHandleU16;

			if constexpr (isLittleEndian)
				m_first = std::byteswap(m_first);

			m_validByteCount += 2u;

			++elementsLoaded;
		}

		if (elementCount == 2u)
		{
			m_second = *(dataHandleU16 + 1u);

			if constexpr (isLittleEndian)
				m_second = std::byteswap(m_second);

			++m_validByteCount;

			m_hasRemainingValue = true;

			++elementsLoaded;
		}
	}

	return elementsLoaded;
}

Encoder24Bits Encoder16Bits::LoadEncoder24bits() const noexcept
{
	Encoder24Bits encoder{};

	// If there is a remaining value, it will be on the last byte of the second value, so load
	// the first 24 bits. Or even if there are no remaining values but the the valid byte count
	// is 2u, that would be on the first value, so load that.
	if (m_hasRemainingValue || m_validByteCount == 2u)
		encoder.LoadData(&m_first, m_validByteCount);
	else
		// If there is only one valid byte, it will be on the second byte, as we shouldn't load
		// just an 8bit value, and on 16bits data, valid byte can only be 1 from the leftover
		// 8bit from another 16bit data.
		encoder.LoadData(reinterpret_cast<std::uint8_t const*>(&m_first) + 1u, m_validByteCount);

	return encoder;
}

std::array<char, 4u> Encoder16Bits::Encode() const noexcept
{
	Encoder24Bits encoder = LoadEncoder24bits();

	std::array<char, 4u> output{};

	if (encoder.AreAllBytesValid())
		output = encoder.Encode();
	else
		output = encoder.EncodeWithCheck();

	return output;
}

std::string Encoder16Bits::EncodeStr() const noexcept
{
	Encoder24Bits encoder = LoadEncoder24bits();

	std::string output{};

	if (encoder.AreAllBytesValid())
		output = encoder.EncodeStr();
	else
		output = encoder.EncodeStrWithCheck();

	return output;
}

// Encoder 32 Bits
Encoder24Bits Encoder32Bits::LoadEncoder24bits() const noexcept
{
	Encoder24Bits encoder{};

	encoder.LoadData(
		&m_storedValue, std::min(static_cast<std::uint32_t>(m_validByteCount), 3u)
	);

	return encoder;
}

std::array<char, 4u> Encoder32Bits::Encode() const noexcept
{
	Encoder24Bits encoder = LoadEncoder24bits();

	std::array<char, 4u> output{};

	if (encoder.AreAllBytesValid())
		output = encoder.Encode();
	else
		output = encoder.EncodeWithCheck();

	return output;
}

std::string Encoder32Bits::EncodeStr() const noexcept
{
	Encoder24Bits encoder = LoadEncoder24bits();

	std::string output{};

	if (encoder.AreAllBytesValid())
		output = encoder.EncodeStr();
	else
		output = encoder.EncodeStrWithCheck();

	return output;
}

// Encoder 64 Bits
std::array<Encoder24Bits, 2u> Encoder64Bits::LoadEncoder24bits() const noexcept
{
	return {};
}

std::array<char, 8u> Encoder64Bits::Encode() const noexcept
{
	return {};
}

std::string Encoder64Bits::EncodeStr() const noexcept
{
	return {};
}

template<bool checkLastBytes>
static void Encode24Bits(
	Encoder24Bits& encoder, std::vector<std::uint8_t>& encodedData, size_t& encodedCharIndex
) noexcept {
	// Only need to check for validity on the last few bits, as that might not be 24bits.
	// Which I will do outside of the loop.
	if constexpr (checkLastBytes)
		for (size_t index = 0u; index < 4u; ++index)
			encodedData[encodedCharIndex] = encoder.Encode6bitsWithCheck(index);
	else
		for (size_t index = 0u; index < 4u; ++index)
			encodedData[encodedCharIndex] = encoder.Encode6bits(index);

	++encodedCharIndex;
}

template<bool isLittleEndian, size_t primitiveSize>
static void EncodeBase64(
	void const* dataHandle, std::vector<std::uint8_t>& encodedData,
	size_t bits24Count, size_t remainder24BitCount
) noexcept {
	size_t elementOffset    = 0u;
	size_t encodedCharIndex = 0u;

	if constexpr(primitiveSize == 1u)
	{
		auto dataHandleU8 = static_cast<std::uint8_t const*>(dataHandle);

		const size_t byteCount = bits24Count * 24u / 8u;

		for (size_t index = 0u; index < byteCount; ++index)
		{
			Encoder24Bits encoder{};

			encoder.LoadData(dataHandleU8 + elementOffset, 3u);

			Encode24Bits<false>(encoder, encodedData, encodedCharIndex);

			++elementOffset;
		}
	}
	else if constexpr (primitiveSize == 2u)
	{
		struct
		{
			std::uint16_t first;
			std::uint16_t second;
		} tempValue { 0u, 0u };

		auto dataHandleU16 = static_cast<std::uint16_t const*>(dataHandle);

		const size_t byte2Count = bits24Count * 24u / 16u;

		if (byte2Count)
		{
			tempValue.second = *dataHandleU16;

			if constexpr (isLittleEndian)
				tempValue.second = std::byteswap(tempValue.second);
		}

		for (size_t index = 0u; index < byte2Count; ++index)
		{
			++elementOffset;

			Encoder24Bits encoder{};

			// Since we must include half of the 16bits, we will have to access the 4th byte
			// to swap, but it should be fine and there should be 8bits of valid remainder
			// after the end.
			tempValue.first  = tempValue.second;
			tempValue.second = *(dataHandleU16 + elementOffset);

			if constexpr (isLittleEndian)
				tempValue.second = std::byteswap(tempValue.second);

			encoder.LoadData(&tempValue, 3u);

			Encode24Bits<false>(encoder, encodedData, encodedCharIndex);
		}
	}
	else if constexpr (primitiveSize == 4u)
	{
		auto dataHandleU32 = static_cast<std::uint32_t const*>(dataHandle);

		std::bitset<64> tempValue = 0u;
		size_t tempBitCount       = 0u;

		for (size_t index = 0u; index < bits24Count; ++index)
		{
			std::bitset<64> processedValue = 0u;
			std::bitset<64> currentValue   = *(dataHandleU32 + elementOffset);

			if constexpr (isLittleEndian)
				currentValue = std::byteswap(currentValue.to_ullong());

			// If there are any temp bits left, load them up first.

			size_t bitsToProcess = 24u;

			if (tempBitCount)
			{
				processedValue |= tempValue;
				processedValue <<= tempBitCount;

				bitsToProcess -= tempBitCount;
			}

			{
				const size_t currentBitStartIndex = std::size(currentValue) - 1u;

				for (size_t cIndex = 0u; cIndex < bitsToProcess; ++cIndex)
				{
					processedValue[cIndex] = currentValue[currentBitStartIndex];

					currentValue <<= 1u;
				}

				constexpr size_t primitiveBitCount = primitiveSize * 8u;

				tempBitCount = primitiveBitCount - bitsToProcess;

				for (size_t cIndex = 0u; cIndex < tempBitCount; ++cIndex)
				{
					tempValue[cIndex] = currentValue[currentBitStartIndex];

					currentValue <<= 1u;
				}

				// Need to do some sort of reset when the temp bit count will reach 24u.
			}

			Encoder24Bits encoder{};

			//encoder.LoadData(&processedValue.to_ullong(), 3u);

			Encode24Bits<false>(encoder, encodedData, encodedCharIndex);

			++elementOffset;
		}
	}

	if (remainder24BitCount)
	{
		const size_t byteRemainder = remainder24BitCount / 8u;

		if constexpr(primitiveSize == 1u)
		{
			auto dataHandleU8 = static_cast<std::uint8_t const*>(dataHandle);

			Encoder24Bits encoder{};

			encoder.LoadData(dataHandleU8 + elementOffset, byteRemainder);

			Encode24Bits<true>(encoder, encodedData, encodedCharIndex);
		}
		else if constexpr (primitiveSize == 2u)
		{
			auto dataHandleU16 = static_cast<std::uint16_t const*>(dataHandle);

			Encoder24Bits encoder{};

			const std::uint16_t tempData = *(dataHandleU16 + elementOffset);

			encoder.LoadData(
				reinterpret_cast<std::uint8_t const*>(&tempData) + primitiveSize - byteRemainder,
				byteRemainder
			);

			Encode24Bits<true>(encoder, encodedData, encodedCharIndex);
		}
	}
}

std::vector<std::uint8_t> EncodeBase64(
	void const* dataHandle, size_t elementCount, size_t primitiveSize
) noexcept {
	const size_t sizeInBytes     = elementCount * primitiveSize;
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

	// Not constexpr so can check on the runtime.
	const bool isLittleEndian = std::endian::native == std::endian::little;

	if (isLittleEndian)
	{
		if (primitiveSize == 1u)
			EncodeBase64<true, 1u>(dataHandle, encodedData, bits24Count, bits24Remainder);
		else if (primitiveSize == 2u)
			EncodeBase64<true, 2u>(dataHandle, encodedData, bits24Count, bits24Remainder);
		else if (primitiveSize == 4u)
			EncodeBase64<true, 4u>(dataHandle, encodedData, bits24Count, bits24Remainder);
		else if (primitiveSize == 8u)
			EncodeBase64<true, 8u>(dataHandle, encodedData, bits24Count, bits24Remainder);
	}
	else
	{
		if (primitiveSize == 1u)
			EncodeBase64<false, 1u>(dataHandle, encodedData, bits24Count, bits24Remainder);
		else if (primitiveSize == 2u)
			EncodeBase64<false, 2u>(dataHandle, encodedData, bits24Count, bits24Remainder);
		else if (primitiveSize == 4u)
			EncodeBase64<false, 4u>(dataHandle, encodedData, bits24Count, bits24Remainder);
		else if (primitiveSize == 8u)
			EncodeBase64<false, 8u>(dataHandle, encodedData, bits24Count, bits24Remainder);
	}

	return encodedData;
}
}
