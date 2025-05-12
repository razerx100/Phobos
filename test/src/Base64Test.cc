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

	Loader24bits loader{};

	loader.LoadData(std::data(data), std::size(data));

	EXPECT_EQ(loader.IsByteValid(0u), true) << "The first byte is false.";
	EXPECT_EQ(loader.IsByteValid(1u), false) << "The second byte is true.";
	EXPECT_EQ(loader.IsByteValid(2u), false) << "The third byte is true.";

	std::bitset<24> bitData = loader.GetData();

	EXPECT_EQ(bitData.test(16u), true) << "The sixteenth bit isn't true.";
	EXPECT_EQ(bitData.test(17u), true) << "The seventeenth bit isn't true.";
}

TEST(Base64Test, Load24Bits2Test)
{
	std::array<std::uint8_t, 2u> data
	{
		2u, 3u
	};

	Loader24bits loader{};

	loader.LoadData(std::data(data), std::size(data));

	EXPECT_EQ(loader.IsByteValid(0u), true) << "The first byte is false.";
	EXPECT_EQ(loader.IsByteValid(1u), true) << "The second byte is false.";
	EXPECT_EQ(loader.IsByteValid(2u), false) << "The third byte is true.";

	std::bitset<24> bitData = loader.GetData();

	EXPECT_EQ(bitData.test(8u), true) << "The eighth bit isn't true.";
	EXPECT_EQ(bitData.test(9u), true) << "The nineth bit isn't true.";
	EXPECT_EQ(bitData.test(16u), false) << "The sixteenth bit isn't false.";
	EXPECT_EQ(bitData.test(17u), true) << "The seventeenth bit isn't true.";
}

TEST(Base64Test, Load24Bits3Test)
{
	std::array<std::uint8_t, 3u> data
	{
		2u, 3u, 3u
	};

	Loader24bits loader{};

	loader.LoadData(std::data(data), std::size(data));

	EXPECT_EQ(loader.IsByteValid(0u), true) << "The first byte is false.";
	EXPECT_EQ(loader.IsByteValid(1u), true) << "The second byte is false.";
	EXPECT_EQ(loader.IsByteValid(2u), true) << "The third byte is false.";

	std::bitset<24> bitData = loader.GetData();

	EXPECT_EQ(bitData.test(0u), true) << "The first bit isn't true.";
	EXPECT_EQ(bitData.test(1u), true) << "The second bit isn't true.";
	EXPECT_EQ(bitData.test(8u), true) << "The eighth bit isn't true.";
	EXPECT_EQ(bitData.test(9u), true) << "The nineth bit isn't true.";
	EXPECT_EQ(bitData.test(16u), false) << "The sixteenth bit isn't false.";
	EXPECT_EQ(bitData.test(17u), true) << "The seventeenth bit isn't true.";
}

