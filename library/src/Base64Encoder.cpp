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

struct MemcpyDetails
{
	std::uint32_t offset1;
	std::uint32_t size1;
	std::uint32_t offset2;
	std::uint32_t size2;
};

static constexpr std::array s_memcpyDetails
{
	MemcpyDetails{ 0u, 0u, 0u, 0u },
	MemcpyDetails{ 1u, 1u, 0u, 0u },
	MemcpyDetails{ 1u, 1u, 2u, 1u }
};

// Encoder 24 bits
void Encoder24Bits::LoadData(void const* dataHandle, size_t byteCount) noexcept
{
	std::uint32_t data = 0u;

	auto dataHandleU8 = static_cast<std::uint8_t const*>(dataHandle);

	const MemcpyDetails memcpyDetails = s_memcpyDetails[byteCount - 1u];

	memcpy(&data, dataHandleU8, 1u);

	data <<= 8u;

	memcpy(&data, dataHandleU8 + memcpyDetails.offset1, memcpyDetails.size1);

	data <<= 8u;

	memcpy(&data, dataHandleU8 + memcpyDetails.offset2, memcpyDetails.size2);

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

	return encoder.Encode();
}

std::array<char, 4u> Encoder16Bits::EncodeWithCheck() const noexcept
{
	Encoder24Bits encoder = LoadEncoder24bits();

	return encoder.EncodeWithCheck();
}

std::string Encoder16Bits::EncodeStr() const noexcept
{
	Encoder24Bits encoder = LoadEncoder24bits();

	return encoder.EncodeStr();
}

std::string Encoder16Bits::EncodeStrWithCheck() const noexcept
{
	Encoder24Bits encoder = LoadEncoder24bits();

	return encoder.EncodeStrWithCheck();
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
	std::array<Encoder24Bits, 2u> encoders{};

	Encoder24Bits& encoder1 = encoders[0];
	Encoder24Bits& encoder2 = encoders[1];

	encoder1.LoadData(
		&m_storedValue, std::min(static_cast<std::uint32_t>(m_validByteCount), 3u)
	);

	if (m_validByteCount > 3u)
	{
		const auto validByteCount = static_cast<std::uint32_t>(m_validByteCount) - 3u;

		encoder2.LoadData(
			reinterpret_cast<std::uint8_t const*>(&m_storedValue) + 3u,
			std::min(validByteCount, 3u)
		);
	}

	return encoders;
}

std::array<char, 8u> Encoder64Bits::Encode() const noexcept
{
	std::array<Encoder24Bits, 2u> encoders = LoadEncoder24bits();

	const Encoder24Bits& encoder1 = encoders[0];
	const Encoder24Bits& encoder2 = encoders[1];

	std::array<char, 8u> output{ '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0' };

	{
		std::array<char, 4u> output1{};

		if (encoder1.AreAllBytesValid())
			output1 = encoder1.Encode();
		else
			output1 = encoder1.EncodeWithCheck();

		memcpy(std::data(output), std::data(output1), 4u);
	}

	if (m_validByteCount > 3u)
	{
		std::array<char, 4u> output2{};

		if (encoder2.AreAllBytesValid())
			output2 = encoder2.Encode();
		else
			output2 = encoder2.EncodeWithCheck();

		memcpy(std::data(output) + 4u, std::data(output2), 4u);
	}

	return output;
}

std::string Encoder64Bits::EncodeStr() const noexcept
{
	std::array<Encoder24Bits, 2u> encoders = LoadEncoder24bits();

	const Encoder24Bits& encoder1 = encoders[0];
	const Encoder24Bits& encoder2 = encoders[1];

	std::string output{};

	{
		if (encoder1.AreAllBytesValid())
			output += encoder1.EncodeStr();
		else
			output += encoder1.EncodeStrWithCheck();
	}

	if (m_validByteCount > 3u)
	{
		if (encoder2.AreAllBytesValid())
			output += encoder2.EncodeStr();
		else
			output += encoder2.EncodeStrWithCheck();
	}

	return output;
}

std::vector<char> EncodeBase64(
	void const* dataHandle, size_t elementCount, size_t primitiveSize
) noexcept {
	const size_t encodedCharacterCount = (elementCount * primitiveSize + 2u) / 3u * 4u;

	std::vector<char> encodedData(encodedCharacterCount, '\0');

	if (primitiveSize == 1u)
	{
		auto dataHandleU8 = static_cast<std::uint8_t const*>(dataHandle);

		size_t eIndex = 0u;
		size_t cIndex = 0u;

		Encoder24Bits encoder{};

		for (; eIndex + 2u < elementCount; eIndex += 3u)
		{
			encoder.LoadData(dataHandleU8 + eIndex, 3u);

			std::array<char, 4u> encoded24Bits = encoder.Encode();

			memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 4u);

			cIndex += 4u;
		}

		if (eIndex < elementCount)
		{
			encoder.LoadData(dataHandleU8 + eIndex, elementCount - eIndex);

			std::array<char, 4u> encoded24Bits = encoder.EncodeWithCheck();

			memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 4u);
		}
	}
	else if (primitiveSize == 2u)
	{
		auto dataHandleU16 = static_cast<std::uint16_t const*>(dataHandle);

		size_t eIndex = 0u;
		size_t cIndex = 0u;

		Encoder16Bits encoder{};

		for (; eIndex + 1u < elementCount;)
		{
			const size_t loadedElement = encoder.LoadData(dataHandleU16 + eIndex, 2u);

			std::array<char, 4u> encoded24Bits = encoder.Encode();

			memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 4u);

			cIndex += 4u;
			eIndex += loadedElement;
		}

		{
			encoder.LoadData(dataHandleU16 + eIndex, elementCount - eIndex);

			std::array<char, 4u> encoded24Bits = encoder.EncodeWithCheck();

			memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 4u);
		}
	}

	return encodedData;
}

std::string EncodeBase64Str(
	void const* dataHandle, size_t elementCount, size_t primitiveSize
) noexcept {
	std::vector<char> encodedData = EncodeBase64(dataHandle, elementCount, primitiveSize);

	return std::string{ std::begin(encodedData), std::end(encodedData) };
}
}
