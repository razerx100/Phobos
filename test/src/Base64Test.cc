#include <gtest/gtest.h>

#include <limits>
#include <Base64Encoder.hpp>
#include <array>

using namespace Phobos;

TEST(Base64Test, Load24Bits1Test)
{
	std::array<std::uint8_t, 1u> data
	{
		3u
	};

	Encoder24bits encoder{};

	encoder.LoadData(std::data(data), std::size(data));

	EXPECT_EQ(encoder.IsByteValid(0u), true) << "The first byte is false.";
	EXPECT_EQ(encoder.IsByteValid(1u), false) << "The second byte is true.";
	EXPECT_EQ(encoder.IsByteValid(2u), false) << "The third byte is true.";

	std::bitset<24> bitData = encoder.GetData();

	EXPECT_EQ(bitData.test(16u), true) << "The sixteenth bit isn't true.";
	EXPECT_EQ(bitData.test(17u), true) << "The seventeenth bit isn't true.";

	EXPECT_EQ(encoder.Encode6bitsWithCheck(0u), 'A') << "The first character isn't A.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(1u), 'w') << "The second character isn't w.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(2u), '=') << "The third character isn't =.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(3u), '=') << "The fourth character isn't =.";
}

TEST(Base64Test, Load24Bits2Test)
{
	std::array<std::uint8_t, 2u> data
	{
		2u, 3u
	};

	Encoder24bits encoder{};

	encoder.LoadData(std::data(data), std::size(data));

	EXPECT_EQ(encoder.IsByteValid(0u), true) << "The first byte is false.";
	EXPECT_EQ(encoder.IsByteValid(1u), true) << "The second byte is false.";
	EXPECT_EQ(encoder.IsByteValid(2u), false) << "The third byte is true.";

	std::bitset<24> bitData = encoder.GetData();

	EXPECT_EQ(bitData.test(8u), true) << "The eighth bit isn't true.";
	EXPECT_EQ(bitData.test(9u), true) << "The nineth bit isn't true.";
	EXPECT_EQ(bitData.test(16u), false) << "The sixteenth bit isn't false.";
	EXPECT_EQ(bitData.test(17u), true) << "The seventeenth bit isn't true.";

	EXPECT_EQ(encoder.Encode6bitsWithCheck(0u), 'A') << "The first character isn't A.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(1u), 'g') << "The second character isn't g.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(2u), 'M') << "The third character isn't M.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(3u), '=') << "The fourth character isn't =.";
}

TEST(Base64Test, Load24Bits3Test)
{
	std::array<std::uint8_t, 3u> data
	{
		2u, 3u, 3u
	};

	Encoder24bits encoder{};

	encoder.LoadData(std::data(data), std::size(data));

	EXPECT_EQ(encoder.IsByteValid(0u), true) << "The first byte is false.";
	EXPECT_EQ(encoder.IsByteValid(1u), true) << "The second byte is false.";
	EXPECT_EQ(encoder.IsByteValid(2u), true) << "The third byte is false.";

	std::bitset<24> bitData = encoder.GetData();

	EXPECT_EQ(bitData.test(0u), true) << "The first bit isn't true.";
	EXPECT_EQ(bitData.test(1u), true) << "The second bit isn't true.";
	EXPECT_EQ(bitData.test(8u), true) << "The eighth bit isn't true.";
	EXPECT_EQ(bitData.test(9u), true) << "The nineth bit isn't true.";
	EXPECT_EQ(bitData.test(16u), false) << "The sixteenth bit isn't false.";
	EXPECT_EQ(bitData.test(17u), true) << "The seventeenth bit isn't true.";

	EXPECT_EQ(encoder.Encode6bitsWithCheck(0u), 'A') << "The first character isn't A.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(1u), 'g') << "The second character isn't g.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(2u), 'M') << "The third character isn't M.";
	EXPECT_EQ(encoder.Encode6bitsWithCheck(3u), 'D') << "The fourth character isn't D.";
}

/*
TEST(Base64Test, EncodingTest)
{
	std::array<std::int32_t, 3u> data
	{
		2, 3, 3
	};

	auto encodededData = Encode(std::data(data), 3u * 4u);

	std::string testStr = "AAAAAgAAAAMAAAAD";
}
*/
