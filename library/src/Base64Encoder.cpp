#include <Base64Encoder.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace Phobos {
namespace {
constexpr std::array s_characterMap{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

constexpr std::array s_6bitOffsetMap{23, 17, 11, 5};
constexpr std::array s_validBitToByteMap{ 0U, 1U, 1U, 2U, 3U };

struct MemcpyDetails {
  std::uint32_t offset1;
  std::uint32_t size1;
  std::uint32_t offset2;
  std::uint32_t size2;
};

constexpr std::array s_memcpyDetails{
  MemcpyDetails{.offset1 = 0U, .size1 = 0U, .offset2 = 0U, .size2 = 0U},
  MemcpyDetails{.offset1 = 1U, .size1 = 1U, .offset2 = 0U, .size2 = 0U},
  MemcpyDetails{.offset1 = 1U, .size1 = 1U, .offset2 = 2U, .size2 = 1U}};

consteval std::uint8_t operator""_u8(unsigned long long value) noexcept {
  return static_cast<std::uint8_t>(value);
}

constexpr auto u8Max = std::numeric_limits<std::uint8_t>::max();

constexpr std::uint8_t s_equalCharValue = 0_u8;

// clang-format off
// Start at 43 or '+'.
constexpr std::array s_bitMap{
  62_u8, u8Max, u8Max, u8Max, 63_u8, 52_u8, 53_u8, 54_u8, 55_u8, 56_u8,
  57_u8, 58_u8, 59_u8, 60_u8, 61_u8, u8Max, u8Max, u8Max, s_equalCharValue,
  u8Max,u8Max, u8Max, 0_u8,  1_u8,  2_u8,  3_u8,  4_u8,  5_u8, 6_u8,
  7_u8,8_u8, 9_u8, 10_u8, 11_u8, 12_u8, 13_u8, 14_u8, 15_u8, 16_u8,
  17_u8,18_u8, 19_u8, 20_u8, 21_u8, 22_u8, 23_u8, 24_u8, 25_u8, u8Max,
  u8Max,u8Max, u8Max, u8Max, u8Max, 26_u8, 27_u8, 28_u8, 29_u8, 30_u8,
  31_u8,32_u8, 33_u8, 34_u8, 35_u8, 36_u8, 37_u8, 38_u8, 39_u8, 40_u8,
  41_u8,42_u8, 43_u8, 44_u8, 45_u8, 46_u8, 47_u8, 48_u8, 49_u8, 50_u8,
  51_u8};
// clang-format on
} // namespace

// Encoder 24 bits
void Encoder24Bits::LoadData(std::uint8_t const *dataHandle, size_t byteCount) {
  assert(byteCount <= byteCountBase64 && "Can't load more than 24bits/3bytes.");

  std::uint32_t data = 0U;

  const MemcpyDetails memcpyDetails = s_memcpyDetails.at(byteCount - 1U);

  memcpy(&data, dataHandle, 1U);

  data <<= bitsInByte;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  memcpy(&data, dataHandle + memcpyDetails.offset1, memcpyDetails.size1);

  data <<= bitsInByte;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  memcpy(&data, dataHandle + memcpyDetails.offset2, memcpyDetails.size2);

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
  std::int64_t bitOffset = s_6bitOffsetMap[index];

  const std::int64_t endBit = bitOffset - bitCount;

  size_t outputValue = 0U;

  for (; bitOffset > endBit; --bitOffset) {
    outputValue <<= 1U;

    outputValue |= static_cast<size_t>(m_data[bitOffset]);
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

bool Encoder24Bits::IsValidRange_(char character) noexcept {
  return character >= '+' && character <= 'z';
}

size_t Encoder24Bits::GetValidCharCount_(char const *encodedStr) noexcept {
  size_t count = 0U;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  while (*(encodedStr + count) != '=')
  {
    ++count;
  }

  return count;
}

void Encoder24Bits::Set6BitValue_(size_t index, std::uint8_t value) noexcept {
  constexpr auto bitCount = static_cast<std::int64_t>(bitCountCharBase64);

  // Ok, private method.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  std::int64_t bitOffset = s_6bitOffsetMap[index];

  const std::int64_t endBit = bitOffset - bitCount;

  std::bitset<bitCountCharBase64> valueBitSet{value};

  for (std::int64_t valueIndex = bitCount - 1; bitOffset > endBit;
       --bitOffset, --valueIndex) {
    m_data[bitOffset] = valueBitSet[valueIndex];
  }
}

void Encoder24Bits::LoadAndDecode_(char const *encodedStr) {
  // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  char const char1 = *(encodedStr + 0U);
  char const char2 = *(encodedStr + 1U);
  char const char3 = *(encodedStr + 2U);
  char const char4 = *(encodedStr + 3U);

  if (!IsValidRange_(char1) || !IsValidRange_(char2) || !IsValidRange_(char3) ||
      !IsValidRange_(char4)) {
    throw std::runtime_error{"Invalid encoded string."};
  }

  const size_t index1 = char1 - '+';
  const size_t index2 = char2 - '+';
  const size_t index3 = char3 - '+';
  const size_t index4 = char4 - '+';
  // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  // NOLINTBEGIN(cppcoreguidelines-pro-bounds-constant-array-index)
  const std::uint8_t sixBitOne = s_bitMap[index1];
  const std::uint8_t sixBitTwo = s_bitMap[index2];
  const std::uint8_t sixBitThree = s_bitMap[index3];
  const std::uint8_t sixBitFour = s_bitMap[index4];
  // NOLINTEND(cppcoreguidelines-pro-bounds-constant-array-index)

  if (sixBitOne == u8Max || sixBitTwo == u8Max || sixBitThree == u8Max ||
      sixBitFour == u8Max) {
    throw std::runtime_error{"Invalid encoded string."};
  }

  Set6BitValue_(0U, sixBitOne);
  Set6BitValue_(1U, sixBitTwo);
  Set6BitValue_(2U, sixBitThree);
  Set6BitValue_(3U, sixBitFour);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
  m_validByteCount = s_validBitToByteMap[GetValidCharCount_(encodedStr)];
}

std::array<std::uint8_t, byteCountBase64>
Encoder24Bits::GetDecodedData() const noexcept {
  unsigned long decodedBitValue = m_data.to_ulong();

  std::array<std::uint8_t, byteCountBase64> decodedValue{};

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  memcpy(std::data(decodedValue) + 2U, &decodedBitValue, 1U);
  decodedBitValue >>= bitsInByte;

  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  memcpy(std::data(decodedValue) + 1U, &decodedBitValue, 1U);
  decodedBitValue >>= bitsInByte;

  memcpy(std::data(decodedValue), &decodedBitValue, 1U);

  return decodedValue;
}

// Encoder 16bits
size_t Encoder16Bits::LoadData(std::uint16_t const *dataHandle,
                               size_t elementCount) {
  assert(elementCount <= 2U && "Can't load more than 2 16bit elements.");

  size_t elementsLoaded = 0U;

  constexpr bool isLittleEndian = std::endian::native == std::endian::little;

  m_validByteCount = 0U;

  if (m_hasRemainingValue) {
    m_first = m_second;

    ++m_validByteCount;

    if (elementCount >= 1U) {
      m_second = *dataHandle;

      if constexpr (isLittleEndian) {
        m_second = std::byteswap(m_second);
      }

      m_validByteCount += 2U;

      elementsLoaded = 1U;
    }

    m_hasRemainingValue = false;
  } else {
    if (elementCount >= 1U) {
      m_first = *dataHandle;

      if constexpr (isLittleEndian) {
        m_first = std::byteswap(m_first);
      }

      m_validByteCount += 2U;

      ++elementsLoaded;
    }

    if (elementCount == 2U) {
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      m_second = *(dataHandle + 1U);

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

Encoder24Bits Encoder16Bits::LoadEncoder24bits() const {
  Encoder24Bits encoder{};

  // If there is a remaining value, it will be on the last byte of the second
  // value, so load the first 24 bits. Or even if there are no remaining values
  // but the the valid byte count is 2u, that would be on the first value, so
  // load that.
  if (m_hasRemainingValue || m_validByteCount == 2U) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    encoder.LoadData(reinterpret_cast<std::uint8_t const *>(&m_first),
                     m_validByteCount);
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

std::array<char, charCountBase64> Encoder16Bits::Encode() const {
  return LoadEncoder24bits().Encode();
}

std::array<char, charCountBase64> Encoder16Bits::EncodeWithCheck() const {
  return LoadEncoder24bits().EncodeWithCheck();
}

std::string Encoder16Bits::EncodeStr() const {
  return LoadEncoder24bits().EncodeStr();
}

std::string Encoder16Bits::EncodeStrWithCheck() const {
  return LoadEncoder24bits().EncodeStrWithCheck();
}

// Encoder 32 Bits
std::array<char, charCountBase64> Encoder32Bits::Encode() const {
  return LoadEncoder24bits_().Encode();
}

std::array<char, charCountBase64> Encoder32Bits::EncodeWithCheck() const {
  return LoadEncoder24bits_().EncodeWithCheck();
}

std::string Encoder32Bits::EncodeStr() const {
  return LoadEncoder24bits_().EncodeStr();
}

std::string Encoder32Bits::EncodeStrWithCheck() const {
  return LoadEncoder24bits_().EncodeStrWithCheck();
}

// Encoder 64 Bits
std::array<Encoder24Bits, Encoder64Bits::unitCount>
Encoder64Bits::LoadEncoder48bits() const {
  const size_t validByteCount = GetValidByteCount();

  std::array<Encoder24Bits, unitCount> encoders{
    LoadEncoder24bits(0U, validByteCount)};

  if (AreLast4CharactersValid()) {
    const size_t remainingValidByteCount = validByteCount - byteCountBase64;

    encoders[1] = LoadEncoder24bits(byteCountBase64, remainingValidByteCount);
  }

  return encoders;
}

std::array<char, Encoder64Bits::charCount> Encoder64Bits::Encode() const {
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
Encoder64Bits::EncodeWithCheck() const {
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

std::string Encoder64Bits::EncodeStr() const {
  const auto [encoder1, encoder2] = LoadEncoder48bits();

  return encoder1.EncodeStr() + encoder2.EncodeStr();
}

std::string Encoder64Bits::EncodeStrWithCheck() const {
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
void Encode32BitsPlus(std::vector<char> &encodedData, T const *dataHandle,
                      size_t elementCount) {
  constexpr bool is64Bits = std::is_same_v<T, std::uint64_t>;
  static constexpr size_t charCount =
    is64Bits ? Encoder64Bits::charCount : charCountBase64;

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
                               size_t primitiveSize) {
  constexpr size_t oneByte = 1U;
  constexpr size_t twoBytes = 2U;
  constexpr size_t fourBytes = 4U;
  constexpr size_t eightBytes = 8U;

  assert((primitiveSize == oneByte || primitiveSize == twoBytes ||
          primitiveSize == fourBytes || primitiveSize == eightBytes) &&
         "Invalid primitive size.");

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
    Encode32BitsPlus(encodedData,
                     static_cast<std::uint32_t const *>(dataHandle),
                     elementCount);
  } else if (primitiveSize == eightBytes) {
    Encode32BitsPlus(encodedData,
                     static_cast<std::uint64_t const *>(dataHandle),
                     elementCount);
  }

  return encodedData;
}

std::string EncodeBase64Str(void const *dataHandle, size_t elementCount,
                            size_t primitiveSize) {
  const std::vector<char> encodedData =
    EncodeBase64(dataHandle, elementCount, primitiveSize);

  return std::string{std::begin(encodedData), std::end(encodedData)};
}
} // namespace Phobos
