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

	EXPECT_EQ(encoder.EncodeStrWithCheck(), "Aw==") << "Wrong encoded string.";
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

	EXPECT_EQ(encoder.EncodeStrWithCheck(), "AgM=") << "Wrong encoded string.";
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

	EXPECT_EQ(encoder.EncodeStr(), "AgMD") << "Wrong encoded string.";
}

TEST(Base64Test, Load24Bits4Test)
{
	std::array<std::uint32_t, 1u> data
	{
		0xfffff9u
	};

	Encoder24bits encoder{};

	encoder.LoadData(std::data(data), 3u);

	EXPECT_EQ(encoder.IsByteValid(0u), true) << "The first byte is false.";
	EXPECT_EQ(encoder.IsByteValid(1u), true) << "The second byte is false.";

	auto encodededData = EncodeBase64(
		std::data(data), std::size(data), sizeof(decltype(data)::value_type)
	);

	/*
	EXPECT_EQ(encodededData[0], 'A') << "The first character isn't A.";
	EXPECT_EQ(encodededData[1], 'P') << "The second character isn't P.";
	EXPECT_EQ(encodededData[2], '/') << "The third character isn't /.";
	EXPECT_EQ(encodededData[3], '/') << "The fourth character isn't /.";
	EXPECT_EQ(encodededData[4], '+') << "The fifth character isn't +.";
	EXPECT_EQ(encodededData[5], 'Q') << "The sixth character isn't Q.";
	EXPECT_EQ(encodededData[6], '=') << "The seventh character isn't =.";
	EXPECT_EQ(encodededData[7], '=') << "The eighth character isn't =.";
	*/
}

TEST(Base64Test, EncodingTest)
{
	std::array<std::int32_t, 3u> data
	{
		2, 3, 3
	};

	//auto encodededData = Encode(std::data(data), 3u * 4u);

	//std::string testStr = "AAAAAgAAAAMAAAAD";
}
