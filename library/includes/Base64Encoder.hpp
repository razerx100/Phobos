#ifndef BASE_64_ENCODER_HPP_
#define BASE_64_ENCODER_HPP_
#include <array>
#include <bit>
#include <bitset>
#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

namespace Phobos {
inline constexpr size_t bitsInByte = 8U;
inline constexpr size_t charCountBase64 = 4U;
inline constexpr size_t bitCountCharBase64 = 6U;
inline constexpr size_t bitCountBase64 = 24U; // 6 x 4 = 24bits.
inline constexpr size_t byteCountBase64 = 3U;

class Encoder24Bits {
public:
  // Won't account for endianness. So, for any primitive larger than a byte,
  // the correct bit sized encoder should be used instead. Also
  // only loads 24bits/3 bytes.
  void LoadData(void const *dataHandle, size_t byteCount);

  [[nodiscard]]
  bool IsByteValid(size_t index) const noexcept;
  [[nodiscard]]
  bool AreAllBytesValid() const noexcept;

  [[nodiscard]]
  std::array<char, charCountBase64> Encode() const noexcept;
  [[nodiscard]]
  std::array<char, charCountBase64> EncodeWithCheck() const noexcept;
  [[nodiscard]]
  std::string EncodeStr() const noexcept;
  [[nodiscard]]
  std::string EncodeStrWithCheck() const noexcept;

  [[nodiscard]]
  const std::bitset<bitCountBase64> &GetData() const noexcept {
    return m_data;
  }

private:
  [[nodiscard]]
  char Encode6bits_(size_t index) const noexcept;
  [[nodiscard]]
  char Encode6bitsWithCheck_(size_t index) const noexcept;

  [[nodiscard]]
  size_t Get6BitValue_(size_t index) const noexcept;

private:
  std::bitset<bitCountBase64> m_data;
  std::uint32_t m_validByteCount{};
};

class Encoder16Bits {
public:
  // The element count could be either 1 or 2. Returns the number of element
  // loaded. Since 16bits are fewer than 24bits, we can't load 2 elements fully
  // and 8bits would remain. So, on the next turn only one element will be
  // loaded.
  size_t LoadData(void const *dataHandle, size_t elementCount) noexcept;

  [[nodiscard]]
  std::array<char, charCountBase64> Encode() const noexcept;
  [[nodiscard]]
  std::array<char, charCountBase64> EncodeWithCheck() const noexcept;

  [[nodiscard]]
  std::string EncodeStr() const noexcept;
  [[nodiscard]]
  std::string EncodeStrWithCheck() const noexcept;

private:
  [[nodiscard]]
  Encoder24Bits LoadEncoder24bits() const noexcept;

private:
  std::uint16_t m_first{0U};
  std::uint16_t m_second{0U};
  bool m_hasRemainingValue{false};
  std::uint8_t m_validByteCount{0U};
};

template <typename T>
concept Plus24Bits_t = requires(T) {
  std::is_integral_v<T> && sizeof(T) * bitsInByte > bitCountBase64;
};

template <Plus24Bits_t Integral_t>
class Encoder24PlusBits {
public:
  // Some point the extra bit count will reach the Integral's bit limit and
  // then the function will load the extra bits fully instead of the argument
  // and return false. We need a way to load the last remaining bits, in such
  // case the element count should be 0u. Element count which is more than 1
  // would also be treated as 1, as we can't pass more than 1 element.
  bool LoadData(Integral_t currentValue, size_t elementCount = 1U) noexcept {
    constexpr size_t integralByteCount = sizeof(Integral_t);

    constexpr bool isLittleEndian = std::endian::native == std::endian::little;

    if constexpr (isLittleEndian) {
      currentValue = std::byteswap(currentValue);
    }

    // Load the remaining bits first.
    m_storedValue = m_remainingBytes;

    m_validByteCount = m_remainingByteCount;

    bool isNewValueLoaded = false;

    // Load the extra bits into stored-value.
    if (elementCount > 0U) {
      const size_t extraBytesToLoad = integralByteCount - m_remainingByteCount;

      Integral_t tempStoredValue{m_storedValue};
      Integral_t tempCurrentValue{currentValue};

      // NOLINTNEXTLINE(*-bounds-pointer-arithmetic, *-type-reinterpret-cast)
      memcpy(reinterpret_cast<std::uint8_t *>(&tempStoredValue) +
               m_remainingByteCount,
             // NOLINTNEXTLINE(*-type-reinterpret-cast)
             reinterpret_cast<std::uint8_t *>(&tempCurrentValue),
             extraBytesToLoad);

      m_storedValue = tempStoredValue;
    }

    if (m_remainingByteCount == integralByteCount || elementCount == 0U) {
      m_remainingByteCount = 0U;

      // If the remaining Byte count and the integral byte count are same,
      // then we will have to load the extra bytes from the old remaining bytes.
      // It is not needed for when the element count is zero, but should be
      // fine as we won't load the value if the element count is zero.
      currentValue = m_remainingBytes;
    } else {
      isNewValueLoaded = true;
    }

    constexpr size_t remainingByteCountPerIntegral = integralByteCount % 3U;

    // We don't want to add any extra remaining bytes if the element count is
    // zero. Element count more than 1 would be invalid.
    m_remainingByteCount +=
      static_cast<std::uint8_t>(elementCount * remainingByteCountPerIntegral);

    // We also won't be loading any extra valid bytes if the element count is
    // zero.
    const size_t newlyLoadedValidByteCount =
      // NOLINTNEXTLINE (readability-math-missing-parentheses)
      elementCount * integralByteCount - m_remainingByteCount;

    // Load the extra bits into remaining-bytes.
    Integral_t tempRemainingValue{0U};
    Integral_t tempCurrentValue{currentValue};

    // NOLINTBEGIN(*-bounds-pointer-arithmetic, *-type-reinterpret-cast)
    memcpy(reinterpret_cast<std::uint8_t *>(&tempRemainingValue),
           reinterpret_cast<std::uint8_t *>(&tempCurrentValue) +
             newlyLoadedValidByteCount,
           m_remainingByteCount);
    // NOLINTEND(*-bounds-pointer-arithmetic, *-type-reinterpret-cast)

    m_remainingBytes = tempRemainingValue;

    m_validByteCount += static_cast<std::uint8_t>(newlyLoadedValidByteCount);

    return isNewValueLoaded;
  }

protected:
  [[nodiscard]]
  Encoder24Bits LoadEncoder24bits() const noexcept {
    Encoder24Bits encoder{};

    encoder.LoadData(
      &m_storedValue,
      std::min(static_cast<size_t>(m_validByteCount), byteCountBase64));

    return encoder;
  }

protected:
  Integral_t m_storedValue;
  Integral_t m_remainingBytes;
  std::uint8_t m_remainingByteCount;
  std::uint8_t m_validByteCount;
};

class Encoder32Bits : public Encoder24PlusBits<std::uint32_t> {
public:
  [[nodiscard]]
  std::array<char, charCountBase64> Encode() const noexcept;
  [[nodiscard]]
  std::array<char, charCountBase64> EncodeWithCheck() const noexcept;

  [[nodiscard]]
  std::string EncodeStr() const noexcept;
  [[nodiscard]]
  std::string EncodeStrWithCheck() const noexcept;
};

class Encoder64Bits : public Encoder24PlusBits<std::uint64_t> {
public:
  static constexpr size_t unitCount = 2U;
  static constexpr size_t charCount = charCountBase64 * unitCount;

  [[nodiscard]]
  std::array<char, charCount> Encode() const noexcept;
  [[nodiscard]]
  std::array<char, charCount> EncodeWithCheck() const noexcept;

  [[nodiscard]]
  std::string EncodeStr() const noexcept;
  [[nodiscard]]
  std::string EncodeStrWithCheck() const noexcept;

  [[nodiscard]]
  bool AreLast4CharactersValid() const noexcept {
    return m_validByteCount > byteCountBase64;
  }

private:
  [[nodiscard]]
  std::array<Encoder24Bits, unitCount> LoadEncoder48bits() const noexcept;
};

[[nodiscard]]
std::vector<char> EncodeBase64(void const *dataHandle, size_t elementCount,
                               size_t primitiveSize) noexcept;

[[nodiscard]]
std::string EncodeBase64Str(void const *dataHandle, size_t elementCount,
                            size_t primitiveSize) noexcept;
} // namespace Phobos
#endif
