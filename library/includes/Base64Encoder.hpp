#ifndef BASE_64_ENCODER_HPP_
#define BASE_64_ENCODER_HPP_
#include <cstdint>
#include <cmath>
#include <bit>
#include <bitset>
#include <array>
#include <vector>
#include <string>
#include <concepts>
#include <type_traits>

namespace Phobos
{
class Encoder24Bits
{
public:
	Encoder24Bits() : m_data{ 0u }, m_validByteCount{ 0u } {}

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

class Encoder16Bits
{
public:
	Encoder16Bits()
		: m_first{ 0u }, m_second{ 0u }, m_hasRemainingValue{ false }, m_validByteCount{ 0u }
	{}

	// The element count could be either 1 or 2. Returns the number of element loaded.
	// Since 16bits are fewer than 24bits, we can't load 2 elements fully and 8bits
	// would remain. So, on the next turn only one element will be loaded.
	size_t LoadData(void const* dataHandle, size_t elementCount) noexcept;

	[[nodiscard]]
	std::array<char, 4u> Encode() const noexcept;
	[[nodiscard]]
	std::array<char, 4u> EncodeWithCheck() const noexcept;

	[[nodiscard]]
	std::string EncodeStr() const noexcept;
	[[nodiscard]]
	std::string EncodeStrWithCheck() const noexcept;

private:
	[[nodiscard]]
	Encoder24Bits LoadEncoder24bits() const noexcept;

private:
	std::uint16_t m_first;
	std::uint16_t m_second;
	bool          m_hasRemainingValue;
	std::uint8_t  m_validByteCount;
};

template<typename T>
concept Plus24Bits_t = requires(T)
{
	std::is_integral_v<T> && sizeof(T) * 8u > 24u;
};

template<Plus24Bits_t Integral_t>
class Encoder24PlusBits
{
public:
	Encoder24PlusBits()
		: m_storedValue{ 0u }, m_remainingBytes{ 0u }, m_remainingByteCount{ 0u },
		m_validByteCount{ 0u }
	{}

	// Since there are more than 24 bits, there will always be extra bits. And at some point
	// that extra bit count will reach the Integral's bit limit and then the function will load
	// the extra bits fully instead of the argument and return false.
	// We need a way to load the last remaining bits, in such case the element count should be 0u.
	// Element count which is more than 1 would also be treated as 1, as we can't pass more
	// than 1 element.
	bool LoadData(Integral_t currentValue, size_t elementCount = 1u) noexcept
	{
		constexpr size_t integralByteCount = sizeof(Integral_t);

		constexpr bool isLittleEndian = std::endian::native == std::endian::little;

		if constexpr (isLittleEndian)
			currentValue = std::byteswap(currentValue);

		// Load the remaining bits first.
		{
			m_storedValue    = m_remainingBytes;

			m_validByteCount = m_remainingByteCount;
		}

		bool isNewValueLoaded = false;

		// Load the extra bits into stored-value.
		if (elementCount)
		{
			const size_t extraBytesToLoad = integralByteCount - m_remainingByteCount;

			Integral_t tempStoredValue{ m_storedValue };
			Integral_t tempCurrentValue{ currentValue };

			memcpy(
				reinterpret_cast<std::uint8_t*>(&tempStoredValue) + m_remainingByteCount,
				reinterpret_cast<std::uint8_t*>(&tempCurrentValue),
				extraBytesToLoad
			);

			m_storedValue = tempStoredValue;
		}

		if (m_remainingByteCount == integralByteCount || !elementCount)
		{
			m_remainingByteCount = 0u;

			// If the remaining Byte count and the integral byte count are same,
			// then we will have to load the extra bytes from the old remaining bytes.
			// It is not needed for when the element count is zero, but should be
			// fine as we won't load the value if the element count is zero.
			currentValue         = m_remainingBytes;
		}
		else
			isNewValueLoaded = true;

		constexpr size_t remainingByteCountPerIntegral = integralByteCount % 3u;

		// We don't want to add any extra remaining bytes if the element count is zero.
		// Element count more than 1 would be invalid.
		m_remainingByteCount += static_cast<std::uint8_t>(
			elementCount * remainingByteCountPerIntegral
		);

		// We also won't be loading any extra valid bytes if the element count is zero.
		const size_t newlyLoadedValidByteCount
			= elementCount * integralByteCount - m_remainingByteCount;

		// Load the extra bits into remaining-bytes.
		{
			Integral_t tempRemainingValue{ 0u };
			Integral_t tempCurrentValue{ currentValue };

			memcpy(
				reinterpret_cast<std::uint8_t*>(&tempRemainingValue),
				reinterpret_cast<std::uint8_t*>(&tempCurrentValue)
				+ newlyLoadedValidByteCount,
				m_remainingByteCount
			);

			m_remainingBytes = tempRemainingValue;
		}

		m_validByteCount += static_cast<std::uint8_t>(newlyLoadedValidByteCount);

		return isNewValueLoaded;
	}

protected:
	Integral_t   m_storedValue;
	Integral_t   m_remainingBytes;
	std::uint8_t m_remainingByteCount;
	std::uint8_t m_validByteCount;
};

class Encoder32Bits : public Encoder24PlusBits<std::uint32_t>
{
public:
	Encoder32Bits() : Encoder24PlusBits{} {}

	[[nodiscard]]
	std::array<char, 4u> Encode() const noexcept;
	[[nodiscard]]
	std::array<char, 4u> EncodeWithCheck() const noexcept;

	[[nodiscard]]
	std::string EncodeStr() const noexcept;
	[[nodiscard]]
	std::string EncodeStrWithCheck() const noexcept;

private:
	[[nodiscard]]
	Encoder24Bits LoadEncoder24bits() const noexcept;
};

class Encoder64Bits : public Encoder24PlusBits<std::uint64_t>
{
public:
	Encoder64Bits() : Encoder24PlusBits{} {}

	[[nodiscard]]
	std::array<char, 8u> Encode() const noexcept;
	[[nodiscard]]
	std::array<char, 8u> EncodeWithCheck() const noexcept;

	[[nodiscard]]
	std::string EncodeStr() const noexcept;
	[[nodiscard]]
	std::string EncodeStrWithCheck() const noexcept;

private:
	[[nodiscard]]
	std::array<Encoder24Bits, 2u> LoadEncoder24bits() const noexcept;
};

[[nodiscard]]
std::vector<char> EncodeBase64(
	void const* dataHandle, size_t elementCount, size_t primitiveSize
) noexcept;

[[nodiscard]]
std::string EncodeBase64Str(
	void const* dataHandle, size_t elementCount, size_t primitiveSize
) noexcept;
}
#endif
