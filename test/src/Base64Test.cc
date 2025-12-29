#include <gtest/gtest.h>

#include <Base64Encoder.hpp>
#include <array>

using namespace Phobos;

TEST(Base64Test, Load24Bits1Test) {
  std::array<std::uint8_t, 1U> data{3U};

  Encoder24Bits encoder{};

  encoder.LoadData(std::data(data), std::size(data));

  EXPECT_EQ(encoder.IsByteValid(0U), true) << "The first byte is false.";
  EXPECT_EQ(encoder.IsByteValid(1U), false) << "The second byte is true.";
  EXPECT_EQ(encoder.IsByteValid(2U), false) << "The third byte is true.";

  EXPECT_EQ(encoder.EncodeStrWithCheck(), "Aw==") << "Wrong encoded string.";
}

TEST(Base64Test, Load24Bits2Test) {
  std::array<std::uint8_t, 2U> data{2U, 3U};

  Encoder24Bits encoder{};

  encoder.LoadData(std::data(data), std::size(data));

  EXPECT_EQ(encoder.IsByteValid(0U), true) << "The first byte is false.";
  EXPECT_EQ(encoder.IsByteValid(1U), true) << "The second byte is false.";
  EXPECT_EQ(encoder.IsByteValid(2U), false) << "The third byte is true.";

  EXPECT_EQ(encoder.EncodeStrWithCheck(), "AgM=") << "Wrong encoded string.";
}

TEST(Base64Test, Load24Bits3Test) {
  std::array<std::uint8_t, 3U> data{2U, 3U, 3U};

  Encoder24Bits encoder{};

  encoder.LoadData(std::data(data), std::size(data));

  EXPECT_EQ(encoder.IsByteValid(0U), true) << "The first byte is false.";
  EXPECT_EQ(encoder.IsByteValid(1U), true) << "The second byte is false.";
  EXPECT_EQ(encoder.IsByteValid(2U), true) << "The third byte is false.";

  EXPECT_EQ(encoder.EncodeStr(), "AgMD") << "Wrong encoded string.";
}

TEST(Base64Test, Load24Bits4Test) {
  Encoder24Bits encoder{};

  EXPECT_EQ(encoder.IsByteValid(0U), false) << "The first byte is true.";
  EXPECT_EQ(encoder.IsByteValid(1U), false) << "The second byte is true.";
  EXPECT_EQ(encoder.IsByteValid(2U), false) << "The third byte is true.";

  EXPECT_EQ(encoder.EncodeStrWithCheck(), "====") << "Wrong encoded string.";
}

TEST(Base64Test, Load16BitsTest) {
  {
    std::array<std::uint16_t, 1U> data{
      // The number basically represents the encoded string.
      // NOLINTNEXTLINE (cppcoreguidelines-avoid-magic-numbers)
      0xfff9U};

    Encoder16Bits encoder{};

    encoder.LoadData(std::data(data), std::size(data));

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "//k=") << "Wrong encoded string.";
  }

  {
    std::array<std::uint16_t, 2U> data{2U, 3U};

    Encoder16Bits encoder{};

    size_t loadedElements = encoder.LoadData(std::data(data), std::size(data));

    EXPECT_EQ(encoder.EncodeStr(), "AAIA") << "Wrong encoded string.";
    EXPECT_EQ(loadedElements, 2U) << "Didn't load 2 elements.";

    loadedElements = encoder.LoadData(nullptr, 0U);

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "Aw==") << "Wrong encoded string.";
    EXPECT_EQ(loadedElements, 0U) << "Loaded elements.";
  }

  {
    std::array<std::uint16_t, 3U> data{
      // The numbers basically represents the encoded string.
      // NOLINTNEXTLINE(*-magic-numbers)
      2U, 3U, 7U};

    Encoder16Bits encoder{};

    std::uint16_t const *dataHandleU16 = std::data(data);

    size_t loadedElements = encoder.LoadData(dataHandleU16, 2U);

    EXPECT_EQ(encoder.EncodeStr(), "AAIA") << "Wrong encoded string.";
    EXPECT_EQ(loadedElements, 2U) << "Didn't load 2 elements.";

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    dataHandleU16 += loadedElements;

    loadedElements = encoder.LoadData(dataHandleU16, 1U);

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "AwAH") << "Wrong encoded string.";
    EXPECT_EQ(loadedElements, 1U) << "Didn't load 1 element.";
  }
}

TEST(Base64Test, Load32BitsTest) {
  {
    // The number basically represents the encoded string.
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint32_t, 1U> data{0xfffff9U};

    Encoder32Bits encoder{};

    bool loaded = encoder.LoadData(data[0]);

    EXPECT_EQ(encoder.EncodeStr(), "AP//") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(0U, 0U);

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "+Q==") << "Wrong encoded string.";
    EXPECT_EQ(loaded, false) << "Loaded the new value.";
  }

  {
    std::array<std::uint32_t, 2U> data{2U, 3U};

    Encoder32Bits encoder{};

    bool loaded = encoder.LoadData(data[0]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[1]);

    EXPECT_EQ(encoder.EncodeStr(), "AgAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(0U, 0U);

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "AAM=") << "Wrong encoded string.";
    EXPECT_EQ(loaded, false) << "Loaded the new value.";
  }

  {
    // The numbers basically represents the encoded string.
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint32_t, 5U> data{2U, 3U, 7U, 99U, 5U};

    Encoder32Bits encoder{};

    bool loaded = encoder.LoadData(data[0]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[1]);

    EXPECT_EQ(encoder.EncodeStr(), "AgAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[2]);

    EXPECT_EQ(encoder.EncodeStr(), "AAMA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[3]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAH") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[4]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, false) << "Loaded the new value.";

    loaded = encoder.LoadData(data[4]);

    EXPECT_EQ(encoder.EncodeStr(), "YwAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(0U, 0U);

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "AAU=") << "Wrong encoded string.";
    EXPECT_EQ(loaded, false) << "Loaded the new value.";
  }
}

TEST(Base64Test, Load64BitsTest) {
  {
    // The number basically represents the encoded string.
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint64_t, 1U> data{0xfffffffff9LLU};

    Encoder64Bits encoder{};

    bool loaded = encoder.LoadData(data[0]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAA////") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(0U, 0U);

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "//k=") << "Wrong encoded string.";
    EXPECT_EQ(loaded, false) << "Loaded the new value.";
  }

  {
    std::array<std::uint64_t, 2U> data{2U, 3U};

    Encoder64Bits encoder{};

    bool loaded = encoder.LoadData(data[0]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAAAAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[1]);

    EXPECT_EQ(encoder.EncodeStr(), "AAIAAAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(0U, 0U);

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "AAAAAw==")
      << "Wrong encoded string.";
    EXPECT_EQ(loaded, false) << "Loaded the new value.";
  }

  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint64_t, 5U> data{2U, 3U, 7U, 99U, 5U};

    Encoder64Bits encoder{};

    bool loaded = encoder.LoadData(data[0]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAAAAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[1]);

    EXPECT_EQ(encoder.EncodeStr(), "AAIAAAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[2]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAAAwAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[3]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAAAAAH") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(data[4]);

    EXPECT_EQ(encoder.EncodeStr(), "AAAAAAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, false) << "Loaded the new value.";

    loaded = encoder.LoadData(data[4]);

    EXPECT_EQ(encoder.EncodeStr(), "AGMAAAAA") << "Wrong encoded string.";
    EXPECT_EQ(loaded, true) << "Didn't load the new value.";

    loaded = encoder.LoadData(0U, 0U);

    EXPECT_EQ(encoder.EncodeStrWithCheck(), "AAAABQ==")
      << "Wrong encoded string.";
    EXPECT_EQ(loaded, false) << "Loaded the new value.";
  }
}

TEST(Base64Test, EncodeBase64Test1) {
  std::array<std::uint8_t, 3U> data{2U, 3U, 3U};

  {
    std::string encodedData =
      EncodeBase64Str(std::data(data), 2U, sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AgM=") << "Wrong encoded string.";
  }

  {
    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AgMD") << "Wrong encoded string.";
  }

  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint8_t, 9U> data1{9U, 7U, 5U, 3U, 1U, 6U, 2U, 4U, 8U};

    std::string encodedData = EncodeBase64Str(
      std::data(data1), std::size(data1), sizeof(decltype(data1)::value_type));

    EXPECT_EQ(encodedData, "CQcFAwEGAgQI") << "Wrong encoded string.";
  }
}

TEST(Base64Test, EncodeBase64Test2) {
  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint16_t, 1U> data{0xfff9U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "//k=") << "Wrong encoded string.";
  }

  {
    std::array<std::uint16_t, 2U> data{2U, 3U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAIAAw==") << "Wrong encoded string.";
  }

  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint16_t, 3U> data{2U, 3U, 7U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAIAAwAH") << "Wrong encoded string.";
  }

  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint16_t, 9U> data{9U, 7U, 5U, 3U, 1U, 6U, 2U, 4U, 8U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAkABwAFAAMAAQAGAAIABAAI")
      << "Wrong encoded string.";
  }
}

TEST(Base64Test, EncodeBase64Test3) {
  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint32_t, 1U> data{0xfffff9U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AP//+Q==") << "Wrong encoded string.";
  }

  {
    std::array<std::uint32_t, 2U> data{2U, 3U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAAAAgAAAAM=") << "Wrong encoded string.";
  }

  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint32_t, 3U> data{2U, 3U, 7U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAAAAgAAAAMAAAAH") << "Wrong encoded string.";
  }

  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint32_t, 9U> data{9U, 7U, 5U, 3U, 1U, 6U, 2U, 4U, 8U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAAACQAAAAcAAAAFAAAAAwAAAAEAAAAGAAAAAgAAAAQAAAAI")
      << "Wrong encoded string.";
  }
}

TEST(Base64Test, EncodeBase64Test4) {
  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint64_t, 1U> data{0xfffffffff9LLU};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAAA//////k=") << "Wrong encoded string.";
  }

  {
    std::array<std::uint64_t, 2U> data{2U, 3U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAAAAAAAAAIAAAAAAAAAAw==")
      << "Wrong encoded string.";
  }

  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint64_t, 3U> data{2U, 3U, 7U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAAAAAAAAAIAAAAAAAAAAwAAAAAAAAAH")
      << "Wrong encoded string.";
  }

  {
    // NOLINTNEXTLINE(*-magic-numbers)
    std::array<std::uint64_t, 9U> data{9U, 7U, 5U, 3U, 1U, 6U, 2U, 4U, 8U};

    std::string encodedData = EncodeBase64Str(
      std::data(data), std::size(data), sizeof(decltype(data)::value_type));

    EXPECT_EQ(encodedData, "AAAAAAAAAAkAAAAAAAAABwAAAAAAAAAFAAAAAAAAAAMAAAAAAAA"
                           "AAQAAAAAAAAAGAAAAAAAAAAIAAAAAAAAABAAAAAAAAAAI")
      << "Wrong encoded string.";
  }
}
