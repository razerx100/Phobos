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

TEST(Base64Test, Load16BitsTest)
{
	{
		std::array<std::uint16_t, 1u> data
		{
			0xfff9u
		};

		Encoder16bits encoder{};

		encoder.LoadData(std::data(data), std::size(data));

		EXPECT_EQ(encoder.EncodeStr(), "//k=") << "Wrong encoded string.";
	}

	{
		std::array<std::uint16_t, 2u> data
		{
			2u, 3u
		};

		Encoder16bits encoder{};

		size_t loadedElements = encoder.LoadData(std::data(data), std::size(data));

		EXPECT_EQ(encoder.EncodeStr(), "AAIA") << "Wrong encoded string.";
		EXPECT_EQ(loadedElements, 2u) << "Didn't load 2 elements.";

		loadedElements = encoder.LoadData(nullptr, 0u);

		EXPECT_EQ(encoder.EncodeStr(), "Aw==") << "Wrong encoded string.";
		EXPECT_EQ(loadedElements, 0u) << "Loaded elements.";
	}

	{
		std::array<std::uint16_t, 3u> data
		{
			2u, 3u, 7u
		};

		Encoder16bits encoder{};

		std::uint16_t const* dataHandleU16 = std::data(data);

		size_t loadedElements = encoder.LoadData(dataHandleU16, 2u);

		EXPECT_EQ(encoder.EncodeStr(), "AAIA") << "Wrong encoded string.";
		EXPECT_EQ(loadedElements, 2u) << "Didn't load 2 elements.";

		dataHandleU16 += loadedElements;

		loadedElements = encoder.LoadData(dataHandleU16, 1u);

		EXPECT_EQ(encoder.EncodeStr(), "AwAH") << "Wrong encoded string.";
		EXPECT_EQ(loadedElements, 1u) << "Didn't load 1 element.";
	}
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
