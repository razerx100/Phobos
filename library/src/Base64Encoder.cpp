#include <Base64Encoder.hpp>
#include <cstdint>

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
  return m_validByteCount == 3U;
}

size_t Encoder24Bits::Get6BitValue_(size_t index) const noexcept {
  constexpr std::int64_t bitCount = 6;

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
  const size_t byteIndex = index > 0 ? index - 1U : 0U;

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
Encoder24Bits Encoder32Bits::LoadEncoder24bits() const noexcept {
  Encoder24Bits encoder{};

  encoder.LoadData(&m_storedValue,
                   std::min(static_cast<std::uint32_t>(m_validByteCount), 3u));

  return encoder;
}

std::array<char, 4u> Encoder32Bits::Encode() const noexcept {
  Encoder24Bits encoder = LoadEncoder24bits();

  return encoder.Encode();
}

std::array<char, 4u> Encoder32Bits::EncodeWithCheck() const noexcept {
  Encoder24Bits encoder = LoadEncoder24bits();

  return encoder.EncodeWithCheck();
}

std::string Encoder32Bits::EncodeStr() const noexcept {
  Encoder24Bits encoder = LoadEncoder24bits();

  return encoder.EncodeStr();
}

std::string Encoder32Bits::EncodeStrWithCheck() const noexcept {
  Encoder24Bits encoder = LoadEncoder24bits();

  return encoder.EncodeStrWithCheck();
}

// Encoder 64 Bits
std::array<Encoder24Bits, 2u>
Encoder64Bits::LoadEncoder24bits() const noexcept {
  std::array<Encoder24Bits, 2u> encoders{};

  Encoder24Bits &encoder1 = encoders[0];
  Encoder24Bits &encoder2 = encoders[1];

  encoder1.LoadData(&m_storedValue,
                    std::min(static_cast<std::uint32_t>(m_validByteCount), 3u));

  if (m_validByteCount > 3u) {
    const auto validByteCount =
        static_cast<std::uint32_t>(m_validByteCount) - 3u;

    encoder2.LoadData(reinterpret_cast<std::uint8_t const *>(&m_storedValue) +
                          3u,
                      std::min(validByteCount, 3u));
  }

  return encoders;
}

std::array<char, 8u> Encoder64Bits::Encode() const noexcept {
  std::array<Encoder24Bits, 2u> encoders = LoadEncoder24bits();

  const Encoder24Bits &encoder1 = encoders[0];
  const Encoder24Bits &encoder2 = encoders[1];

  std::array<char, 8u> output{'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

  {
    std::array<char, 4u> tempOutput{};

    tempOutput = encoder1.Encode();

    memcpy(std::data(output), std::data(tempOutput), 4u);
  }

  {
    std::array<char, 4u> tempOutput{};

    tempOutput = encoder2.Encode();

    memcpy(std::data(output) + 4u, std::data(tempOutput), 4u);
  }

  return output;
}

std::array<char, 8u> Encoder64Bits::EncodeWithCheck() const noexcept {
  std::array<Encoder24Bits, 2u> encoders = LoadEncoder24bits();

  const Encoder24Bits &encoder1 = encoders[0];
  const Encoder24Bits &encoder2 = encoders[1];

  std::array<char, 8u> output{'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

  {
    std::array<char, 4u> tempOutput{};

    tempOutput = encoder1.EncodeWithCheck();

    memcpy(std::data(output), std::data(tempOutput), 4u);
  }

  if (AreLast4CharactersValid()) {
    std::array<char, 4u> tempOutput{};

    tempOutput = encoder2.EncodeWithCheck();

    memcpy(std::data(output) + 4u, std::data(tempOutput), 4u);
  }

  return output;
}

std::string Encoder64Bits::EncodeStr() const noexcept {
  std::array<Encoder24Bits, 2u> encoders = LoadEncoder24bits();

  const Encoder24Bits &encoder1 = encoders[0];
  const Encoder24Bits &encoder2 = encoders[1];

  std::string output{};

  output += encoder1.EncodeStr();

  output += encoder2.EncodeStr();

  return output;
}

std::string Encoder64Bits::EncodeStrWithCheck() const noexcept {
  std::array<Encoder24Bits, 2u> encoders = LoadEncoder24bits();

  const Encoder24Bits &encoder1 = encoders[0];
  const Encoder24Bits &encoder2 = encoders[1];

  std::string output{};

  output += encoder1.EncodeStrWithCheck();

  if (AreLast4CharactersValid())
    output += encoder2.EncodeStrWithCheck();

  return output;
}

std::vector<char> EncodeBase64(void const *dataHandle, size_t elementCount,
                               size_t primitiveSize) noexcept {
  const size_t encodedCharacterCount =
      (elementCount * primitiveSize + 2u) / 3u * 4u;

  std::vector<char> encodedData(encodedCharacterCount, '\0');

  if (primitiveSize == 1u) {
    auto dataHandleU8 = static_cast<std::uint8_t const *>(dataHandle);

    size_t eIndex = 0u;
    size_t cIndex = 0u;

    Encoder24Bits encoder{};

    for (; eIndex + 2u < elementCount; eIndex += 3u) {
      encoder.LoadData(dataHandleU8 + eIndex, 3u);

      std::array<char, 4u> encoded24Bits = encoder.Encode();

      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 4u);

      cIndex += 4u;
    }

    if (eIndex < elementCount) {
      encoder.LoadData(dataHandleU8 + eIndex, elementCount - eIndex);

      std::array<char, 4u> encoded24Bits = encoder.EncodeWithCheck();

      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 4u);
    }
  } else if (primitiveSize == 2u) {
    auto dataHandleU16 = static_cast<std::uint16_t const *>(dataHandle);

    size_t eIndex = 0u;
    size_t cIndex = 0u;

    Encoder16Bits encoder{};

    for (; eIndex + 1u < elementCount;) {
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
  } else if (primitiveSize == 4u) {
    auto dataHandleU32 = static_cast<std::uint32_t const *>(dataHandle);

    size_t eIndex = 0u;
    size_t cIndex = 0u;

    Encoder32Bits encoder{};

    for (; eIndex < elementCount;) {
      const bool isLoaded = encoder.LoadData(dataHandleU32[eIndex]);

      std::array<char, 4u> encoded24Bits = encoder.Encode();

      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 4u);

      cIndex += 4u;

      if (isLoaded)
        ++eIndex;
    }

    {
      encoder.LoadData(0u, 0u);

      std::array<char, 4u> encoded24Bits = encoder.EncodeWithCheck();

      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 4u);
    }
  } else if (primitiveSize == 8u) {
    auto dataHandleU64 = static_cast<std::uint64_t const *>(dataHandle);

    size_t eIndex = 0u;
    size_t cIndex = 0u;

    Encoder64Bits encoder{};

    for (; eIndex < elementCount;) {
      const bool isLoaded = encoder.LoadData(dataHandleU64[eIndex]);

      std::array<char, 8u> encoded24Bits = encoder.Encode();

      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits), 8u);

      cIndex += 8u;

      if (isLoaded)
        ++eIndex;
    }

    {
      encoder.LoadData(0u, 0u);

      std::array<char, 8u> encoded24Bits = encoder.EncodeWithCheck();

      memcpy(std::data(encodedData) + cIndex, std::data(encoded24Bits),
             encoder.AreLast4CharactersValid() ? 8u : 4u);
    }
  }

  return encodedData;
}

std::string EncodeBase64Str(void const *dataHandle, size_t elementCount,
                            size_t primitiveSize) noexcept {
  std::vector<char> encodedData =
      EncodeBase64(dataHandle, elementCount, primitiveSize);

  return std::string{std::begin(encodedData), std::end(encodedData)};
}
} // namespace Phobos
