#include <Base64Encoder.hpp>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace Phobos {
static constexpr std::array s_characterMap{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

static constexpr std::array s_6bitsOffsetMap{23, 17, 11, 5};

struct MemcpyDetails {
  std::uint32_t offset1;
  std::uint32_t size1;
  std::uint32_t offset2;
  std::uint32_t size2;
};

static constexpr std::array s_memcpyDetails{
  MemcpyDetails{.offset1 = 0U, .size1 = 0U, .offset2 = 0U, .size2 = 0U},
  MemcpyDetails{.offset1 = 1U, .size1 = 1U, .offset2 = 0U, .size2 = 0U},
  MemcpyDetails{.offset1 = 1U, .size1 = 1U, .offset2 = 2U, .size2 = 1U}};

// Encoder 24 bits
void Encoder24Bits::LoadData(void const *dataHandle, size_t byteCount) {
  std::uint32_t data = 0U;

  const auto *dataHandleU8 = static_cast<std::uint8_t const *>(dataHandle);

  const MemcpyDetails memcpyDetails = s_memcpyDetails.at(byteCount - 1U);

  memcpy(&data, dataHandleU8, 1U);

  data <<= bitsInByte;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  memcpy(&data, dataHandleU8 + memcpyDetails.offset1, memcpyDetails.size1);

  data <<= bitsInByte;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  memcpy(&data, dataHandleU8 + memcpyDetails.offset2, memcpyDetails.size2);

  m_data = data;

  m_validByteCount = static_cast<std::uint32_t>(byteCount);
}

bool Encoder24Bits::IsByteValid(size_t index) const noexcept {
  return index < m_validByteCount;
}

bool Encoder24Bits::AreAllBytesValid() const noexcept {
  return m_validByteCount == byteCountBase64;
}

size_t Encoder24Bits::Get6BitValue_(size_t index) const noexcept {
  constexpr auto bitCount = static_cast<std::int64_t>(bitCountCharBase64);

  // Ok, private method.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  std::int64_t bitOffset = s_6bitsOffsetMap[index];

  const std::int64_t endBit = bitOffset - bitCount;

  size_t outputValue = 0U;

  for (; bitOffset > endBit; --bitOffset) {
    outputValue <<= 1U;

    outputValue |= static_cast<size_t>(m_data.test(bitOffset));
  }

  return outputValue;
}

char Encoder24Bits::Encode6bits_(size_t index) const noexcept {
  // Ok, private method.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  return s_characterMap[Get6BitValue_(index)];
}

char Encoder24Bits::Encode6bitsWithCheck_(size_t index) const noexcept {
  char encodedChar = '=';

  // The parameter index is the index of the 6bit segments in the 24bits.
  // We store the data in the multiples of 8bits.
  // Assuming 1 byte is 8bits (usually is).
  // If 3 bytes are stored 8 x 3 = 24 = 6 x 4. 4 full 6bits, so, 0-3 indices are
  // valid . If 2 bytes are stored 8 x 2 = 16 = 6 x 2 + 4, 2 full 6bits and
  // 4bits, 2 empty bits will be added to the end and so, 0-2 indices are valid.
  // If 1 byte is stored 8 x 1 = 8 = 6 x 1 + 2, 1 full 6bits and 2bits, 4 empty
  // bits will be added to the end and so, 0-1 indices are valid. Invalid 6bits
  // are represented with = according to the standard.
  const size_t byteIndex = index > 0U ? index - 1U : 0U;

  if (IsByteValid(byteIndex)) {
    encodedChar = Encode6bits_(index);
  }

  return encodedChar;
}

std::array<char, charCountBase64> Encoder24Bits::Encode() const noexcept {
  return {Encode6bits_(0U), Encode6bits_(1U), Encode6bits_(2U),
          Encode6bits_(3U)};
}

std::array<char, charCountBase64>
Encoder24Bits::EncodeWithCheck() const noexcept {
  return {Encode6bitsWithCheck_(0U), Encode6bitsWithCheck_(1U),
          Encode6bitsWithCheck_(2U), Encode6bitsWithCheck_(3U)};
}

std::string Encoder24Bits::EncodeStr() const noexcept {
  return std::string{Encode6bits_(0U), Encode6bits_(1U), Encode6bits_(2U),
                     Encode6bits_(3U)};
}

std::string Encoder24Bits::EncodeStrWithCheck() const noexcept {
  return std::string{Encode6bitsWithCheck_(0U), Encode6bitsWithCheck_(1U),
                     Encode6bitsWithCheck_(2U), Encode6bitsWithCheck_(3U)};
}

// Encoder 16bits
size_t Encoder16Bits::LoadData(void const *dataHandle,
                               size_t elementCount) noexcept {
  size_t elementsLoaded = 0U;

  const auto *dataHandleU16 = static_cast<std::uint16_t const *>(dataHandle);

  constexpr bool isLittleEndian = std::endian::native == std::endian::little;

  m_validByteCount = 0U;

  if (m_hasRemainingValue) {
    m_first = m_second;

    ++m_validByteCount;

    if (elementCount >= 1U) {
      m_second = *dataHandleU16;

      if constexpr (isLittleEndian) {
        m_second = std::byteswap(m_second);
      }

      m_validByteCount += 2U;

      elementsLoaded = 1U;
    }

    m_hasRemainingValue = false;
  } else {
    if (elementCount >= 1U) {
      m_first = *dataHandleU16;

      if constexpr (isLittleEndian) {
        m_first = std::byteswap(m_first);
      }

      m_validByteCount += 2U;

      ++elementsLoaded;
    }

    if (elementCount == 2U) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      m_second = *(dataHandleU16 + 1U);

      if constexpr (isLittleEndian) {
        m_second = std::byteswap(m_second);
      }

      ++m_validByteCount;

      m_hasRemainingValue = true;

      ++elementsLoaded;
    }
  }

  return elementsLoaded;
}

Encoder24Bits Encoder16Bits::LoadEncoder24bits() const noexcept {
  Encoder24Bits encoder{};

  // If there is a remaining value, it will be on the last byte of the second
  // value, so load the first 24 bits. Or even if there are no remaining values
  // but the the valid byte count is 2u, that would be on the first value, so
  // load that.
  if (m_hasRemainingValue || m_validByteCount == 2U) {
    encoder.LoadData(&m_first, m_validByteCount);
  } else {
    // If there is only one valid byte, it will be on the second byte, as we
    // shouldn't load just an 8bit value, and on 16bits data, valid byte can
    // only be 1 from the leftover 8bit from another 16bit data.

    // NOLINTNEXTLINE(*-bounds-pointer-arithmetic, *-type-reinterpret-cast)
    encoder.LoadData(reinterpret_cast<std::uint8_t const *>(&m_first) + 1U,
                     m_validByteCount);
  }

  return encoder;
}

std::array<char, charCountBase64> Encoder16Bits::Encode() const noexcept {
  return LoadEncoder24bits().Encode();
}

std::array<char, charCountBase64>
Encoder16Bits::EncodeWithCheck() const noexcept {
  return LoadEncoder24bits().EncodeWithCheck();
}

std::string Encoder16Bits::EncodeStr() const noexcept {
  return LoadEncoder24bits().EncodeStr();
}

std::string Encoder16Bits::EncodeStrWithCheck() const noexcept {
  return LoadEncoder24bits().EncodeStrWithCheck();
}

// Encoder 32 Bits
std::array<char, charCountBase64> Encoder32Bits::Encode() const noexcept {
  return LoadEncoder24bits_().Encode();
}

std::array<char, charCountBase64>
Encoder32Bits::EncodeWithCheck() const noexcept {
  return LoadEncoder24bits_().EncodeWithCheck();
}

std::string Encoder32Bits::EncodeStr() const noexcept {
  return LoadEncoder24bits_().EncodeStr();
}

std::string Encoder32Bits::EncodeStrWithCheck() const noexcept {
  return LoadEncoder24bits_().EncodeStrWithCheck();
}

// Encoder 64 Bits
std::array<Encoder24Bits, Encoder64Bits::unitCount>
Encoder64Bits::LoadEncoder48bits() const noexcept {
  const size_t validByteCount = GetValidByteCount();

  std::array<Encoder24Bits, unitCount> encoders{
    LoadEncoder24bits(0U, validByteCount)};

  if (AreLast4CharactersValid()) {
    const size_t remainingValidByteCount = validByteCount - byteCountBase64;

    encoders[1] = LoadEncoder24bits(byteCountBase64, remainingValidByteCount);
  }

  return encoders;
}

std::array<char, Encoder64Bits::charCount>
Encoder64Bits::Encode() const noexcept {
  const auto [encoder1, encoder2] = LoadEncoder48bits();

  std::array<char, charCount> output{'\0', '\0', '\0', '\0',
                                     '\0', '\0', '\0', '\0'};

  {
    const std::array<char, charCountBase64> tempOutput{encoder1.Encode()};

    memcpy(std::data(output), std::data(tempOutput), charCountBase64);
  }

  {
    const std::array<char, charCountBase64> tempOutput{encoder2.Encode()};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy(std::data(output) + charCountBase64, std::data(tempOutput),
           charCountBase64);
  }

  return output;
}

std::array<char, Encoder64Bits::charCount>
Encoder64Bits::EncodeWithCheck() const noexcept {
  const auto [encoder1, encoder2] = LoadEncoder48bits();

  std::array<char, charCount> output{'\0', '\0', '\0', '\0',
                                     '\0', '\0', '\0', '\0'};

  {
    const std::array<char, charCountBase64> tempOutput{
      encoder1.EncodeWithCheck()};

    memcpy(std::data(output), std::data(tempOutput), charCountBase64);
  }

  if (AreLast4CharactersValid()) {
    const std::array<char, charCountBase64> tempOutput{
      encoder2.EncodeWithCheck()};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy(std::data(output) + charCountBase64, std::data(tempOutput),
           charCountBase64);
  }

  return output;
}

std::string Encoder64Bits::EncodeStr() const noexcept {
  const auto [encoder1, encoder2] = LoadEncoder48bits();

  return encoder1.EncodeStr() + encoder2.EncodeStr();
}

std::string Encoder64Bits::EncodeStrWithCheck() const noexcept {
  const auto [encoder1, encoder2] = LoadEncoder48bits();

  std::string output{encoder1.EncodeStrWithCheck()};

  if (AreLast4CharactersValid()) {
    output += encoder2.EncodeStrWithCheck();
  }

  return output;
}

namespace {
template <typename T>
concept UInt32OR64 = Plus24Bits_t<T> && requires(T) {
  std::is_same_v<T, std::uint32_t> || std::is_same_v<T, std::uint64_t>;
};

template <UInt32OR64 T>
struct Encoder32BitsPlus {
  using type = Encoder32Bits;
};

template <>
struct Encoder32BitsPlus<std::uint64_t> {
  using type = Encoder64Bits;
};

template <UInt32OR64 T>
void Encode32BitsPlus(std::vector<char> &encodedData, void const *dataHandleV,
                      size_t elementCount) {
  constexpr bool is64Bits = std::is_same_v<T, std::uint64_t>;
  static constexpr size_t charCount =
    is64Bits ? Encoder64Bits::charCount : charCountBase64;

  auto const *dataHandle = static_cast<T const *>(dataHandleV);

  size_t eIndex = 0U;
  size_t cIndex = 0U;

  typename Encoder32BitsPlus<T>::type encoder{};

  for (; eIndex < elementCount;) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const bool isLoaded = encoder.LoadData(*(dataHandle + eIndex));

    const std::array<char, charCount> encodedChars{encoder.Encode()};

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy(std::data(encodedData) + cIndex, std::data(encodedChars), charCount);

    cIndex += charCount;

    if (isLoaded) {
      ++eIndex;
    }
  }

  {
    encoder.LoadData(0U, 0U);

    const std::array<char, charCount> encoded24Bits{encoder.EncodeWithCheck()};

    size_t remainingByteCount = charCount;

    if constexpr (is64Bits) {
      if (!encoder.AreLast4CharactersValid()) {
        remainingByteCount = charCountBase64;
      }
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits),
           remainingByteCount);
  }
}
} // namespace

std::vector<char> EncodeBase64(void const *dataHandle, size_t elementCount,
                               size_t primitiveSize) noexcept {
  constexpr size_t oneByte = 1U;
  constexpr size_t twoBytes = 2U;
  constexpr size_t fourBytes = 4U;
  constexpr size_t eightBytes = 8U;

  const size_t encodedUnitCount =
    ((elementCount * primitiveSize + 2U) / byteCountBase64) * charCountBase64;

  std::vector<char> encodedData(encodedUnitCount, '\0');

  if (primitiveSize == oneByte) {
    constexpr size_t invalidByteCount = byteCountBase64 - oneByte;

    auto const *dataHandleU8 = static_cast<std::uint8_t const *>(dataHandle);

    size_t eIndex = 0U;
    size_t cIndex = 0U;

    Encoder24Bits encoder{};

    for (; eIndex + invalidByteCount < elementCount;
         eIndex += byteCountBase64) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      encoder.LoadData(dataHandleU8 + eIndex, byteCountBase64);

      const std::array<char, charCountBase64> encoded24Bits{encoder.Encode()};

      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits),
             charCountBase64);

      cIndex += charCountBase64;
    }

    if (eIndex < elementCount) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      encoder.LoadData(dataHandleU8 + eIndex, elementCount - eIndex);

      const std::array<char, charCountBase64> encoded24Bits{
        encoder.EncodeWithCheck()};

      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits),
             charCountBase64);
    }
  } else if (primitiveSize == twoBytes) {
    constexpr size_t invalidByteCount = byteCountBase64 - twoBytes;

    auto const *dataHandleU16 = static_cast<std::uint16_t const *>(dataHandle);

    size_t eIndex = 0U;
    size_t cIndex = 0U;

    Encoder16Bits encoder{};

    for (; eIndex + invalidByteCount < elementCount;) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      const size_t loadedElement = encoder.LoadData(dataHandleU16 + eIndex, 2U);

      const std::array<char, charCountBase64> encoded24Bits = encoder.Encode();

      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits),
             charCountBase64);

      cIndex += charCountBase64;
      eIndex += loadedElement;
    }

    {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      encoder.LoadData(dataHandleU16 + eIndex, elementCount - eIndex);

      const std::array<char, charCountBase64> encoded24Bits{
        encoder.EncodeWithCheck()};

      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits),
             charCountBase64);
    }
  } else if (primitiveSize == fourBytes) {
    Encode32BitsPlus<std::uint32_t>(encodedData, dataHandle, elementCount);
  } else if (primitiveSize == eightBytes) {
    Encode32BitsPlus<std::uint64_t>(encodedData, dataHandle, elementCount);
  }

  return encodedData;
}

std::string EncodeBase64Str(void const *dataHandle, size_t elementCount,
                            size_t primitiveSize) noexcept {
  const std::vector<char> encodedData =
    EncodeBase64(dataHandle, elementCount, primitiveSize);

  return std::string{std::begin(encodedData), std::end(encodedData)};
}
} // namespace Phobos
