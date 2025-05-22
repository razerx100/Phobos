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

	Encoder24Bits encoder{};

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

	Encoder24Bits encoder{};

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

	Encoder24Bits encoder{};

	encoder.LoadData(std::data(data), std::size(data));

	EXPECT_EQ(encoder.IsByteValid(0u), true) << "The first byte is false.";
	EXPECT_EQ(encoder.IsByteValid(1u), true) << "The second byte is false.";
	EXPECT_EQ(encoder.IsByteValid(2u), true) << "The third byte is false.";

	EXPECT_EQ(encoder.EncodeStr(), "AgMD") << "Wrong encoded string.";
}

TEST(Base64Test, Load24Bits4Test)
{
	Encoder24Bits encoder{};

	EXPECT_EQ(encoder.IsByteValid(0u), false) << "The first byte is true.";
	EXPECT_EQ(encoder.IsByteValid(1u), false) << "The second byte is true.";
	EXPECT_EQ(encoder.IsByteValid(2u), false) << "The third byte is true.";

	EXPECT_EQ(encoder.EncodeStrWithCheck(), "====") << "Wrong encoded string.";
}

TEST(Base64Test, Load16BitsTest)
{
	{
		std::array<std::uint16_t, 1u> data
		{
			0xfff9u
		};

		Encoder16Bits encoder{};

		encoder.LoadData(std::data(data), std::size(data));

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "//k=") << "Wrong encoded string.";
	}

	{
		std::array<std::uint16_t, 2u> data
		{
			2u, 3u
		};

		Encoder16Bits encoder{};

		size_t loadedElements = encoder.LoadData(std::data(data), std::size(data));

		EXPECT_EQ(encoder.EncodeStr(), "AAIA") << "Wrong encoded string.";
		EXPECT_EQ(loadedElements, 2u) << "Didn't load 2 elements.";

		loadedElements = encoder.LoadData(nullptr, 0u);

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "Aw==") << "Wrong encoded string.";
		EXPECT_EQ(loadedElements, 0u) << "Loaded elements.";
	}

	{
		std::array<std::uint16_t, 3u> data
		{
			2u, 3u, 7u
		};

		Encoder16Bits encoder{};

		std::uint16_t const* dataHandleU16 = std::data(data);

		size_t loadedElements = encoder.LoadData(dataHandleU16, 2u);

		EXPECT_EQ(encoder.EncodeStr(), "AAIA") << "Wrong encoded string.";
		EXPECT_EQ(loadedElements, 2u) << "Didn't load 2 elements.";

		dataHandleU16 += loadedElements;

		loadedElements = encoder.LoadData(dataHandleU16, 1u);

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "AwAH") << "Wrong encoded string.";
		EXPECT_EQ(loadedElements, 1u) << "Didn't load 1 element.";
	}
}

TEST(Base64Test, Load32BitsTest)
{
	{
		std::array<std::uint32_t, 1u> data
		{
			0xfffff9u
		};

		Encoder32Bits encoder{};

		bool loaded = encoder.LoadData(data[0]);

		EXPECT_EQ(encoder.EncodeStr(), "AP//") << "Wrong encoded string.";
		EXPECT_EQ(loaded, true) << "Didn't load the new value.";

		loaded = encoder.LoadData(0u, 0u);

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "+Q==") << "Wrong encoded string.";
		EXPECT_EQ(loaded, false) << "Loaded the new value.";
	}

	{
		std::array<std::uint32_t, 2u> data
		{
			2u, 3u
		};

		Encoder32Bits encoder{};

		bool loaded = encoder.LoadData(data[0]);

		EXPECT_EQ(encoder.EncodeStr(), "AAAA") << "Wrong encoded string.";
		EXPECT_EQ(loaded, true) << "Didn't load the new value.";

		loaded = encoder.LoadData(data[1]);

		EXPECT_EQ(encoder.EncodeStr(), "AgAA") << "Wrong encoded string.";
		EXPECT_EQ(loaded, true) << "Didn't load the new value.";

		loaded = encoder.LoadData(0u, 0u);

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "AAM=") << "Wrong encoded string.";
		EXPECT_EQ(loaded, false) << "Loaded the new value.";
	}

	{
		std::array<std::uint32_t, 5u> data
		{
			2u, 3u, 7u, 99u, 5u
		};

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

		loaded = encoder.LoadData(0u, 0u);

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "AAU=") << "Wrong encoded string.";
		EXPECT_EQ(loaded, false) << "Loaded the new value.";
	}
}

TEST(Base64Test, Load64BitsTest)
{
	{
		std::array<std::uint64_t, 1u> data
		{
			0xfffffffff9llu
		};

		Encoder64Bits encoder{};

		bool loaded = encoder.LoadData(data[0]);

		EXPECT_EQ(encoder.EncodeStr(), "AAAA////") << "Wrong encoded string.";
		EXPECT_EQ(loaded, true) << "Didn't load the new value.";

		loaded = encoder.LoadData(0u, 0u);

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "//k=") << "Wrong encoded string.";
		EXPECT_EQ(loaded, false) << "Loaded the new value.";
	}

	{
		std::array<std::uint64_t, 2u> data
		{
			2u, 3u
		};

		Encoder64Bits encoder{};

		bool loaded = encoder.LoadData(data[0]);

		EXPECT_EQ(encoder.EncodeStr(), "AAAAAAAA") << "Wrong encoded string.";
		EXPECT_EQ(loaded, true) << "Didn't load the new value.";

		loaded = encoder.LoadData(data[1]);

		EXPECT_EQ(encoder.EncodeStr(), "AAIAAAAA") << "Wrong encoded string.";
		EXPECT_EQ(loaded, true) << "Didn't load the new value.";

		loaded = encoder.LoadData(0u, 0u);

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "AAAAAw==") << "Wrong encoded string.";
		EXPECT_EQ(loaded, false) << "Loaded the new value.";
	}

	{
		std::array<std::uint64_t, 5u> data
		{
			2u, 3u, 7u, 99u, 5u
		};

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

		loaded = encoder.LoadData(0u, 0u);

		EXPECT_EQ(encoder.EncodeStrWithCheck(), "AAAABQ==") << "Wrong encoded string.";
		EXPECT_EQ(loaded, false) << "Loaded the new value.";
	}
}

TEST(Base64Test, EncodeBase64Test1)
{
	std::array<std::uint8_t, 3u> data
	{
		2u, 3u, 3u
	};

	{
		std::string encodedData = EncodeBase64Str(
			std::data(data), 2u, sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AgM=") << "Wrong encoded string.";
	}

	{
		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AgMD") << "Wrong encoded string.";
	}

	std::array<std::uint8_t, 9u> data1
	{
		9u, 7u, 5u, 3u, 1u, 6u, 2u, 4u, 8u
	};

	{
		{
			std::string encodedData = EncodeBase64Str(
				std::data(data1), std::size(data1), sizeof(decltype(data1)::value_type)
			);

			EXPECT_EQ(encodedData, "CQcFAwEGAgQI") << "Wrong encoded string.";
		}
	}
}

TEST(Base64Test, EncodeBase64Test2)
{
	{
		std::array<std::uint16_t, 1u> data
		{
			0xfff9u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "//k=") << "Wrong encoded string.";
	}

	{
		std::array<std::uint16_t, 2u> data
		{
			2u, 3u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAIAAw==") << "Wrong encoded string.";
	}

	{
		std::array<std::uint16_t, 3u> data
		{
			2u, 3u, 7u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAIAAwAH") << "Wrong encoded string.";
	}

	{
		std::array<std::uint16_t, 9u> data
		{
			9u, 7u, 5u, 3u, 1u, 6u, 2u, 4u, 8u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAkABwAFAAMAAQAGAAIABAAI") << "Wrong encoded string.";
	}
}

TEST(Base64Test, EncodeBase64Test3)
{
	{
		std::array<std::uint32_t, 1u> data
		{
			0xfffff9u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AP//+Q==") << "Wrong encoded string.";
	}

	{
		std::array<std::uint32_t, 2u> data
		{
			2u, 3u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAAAAgAAAAM=") << "Wrong encoded string.";
	}

	{
		std::array<std::uint32_t, 3u> data
		{
			2u, 3u, 7u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAAAAgAAAAMAAAAH") << "Wrong encoded string.";
	}

	{
		std::array<std::uint32_t, 9u> data
		{
			9u, 7u, 5u, 3u, 1u, 6u, 2u, 4u, 8u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAAACQAAAAcAAAAFAAAAAwAAAAEAAAAGAAAAAgAAAAQAAAAI")
			<< "Wrong encoded string.";
	}
}

TEST(Base64Test, EncodeBase64Test4)
{
	{
		std::array<std::uint64_t, 1u> data
		{
			0xfffffffff9llu
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAAA//////k=") << "Wrong encoded string.";
	}

	{
		std::array<std::uint64_t, 2u> data
		{
			2u, 3u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAAAAAAAAAIAAAAAAAAAAw==") << "Wrong encoded string.";
	}

	{
		std::array<std::uint64_t, 3u> data
		{
			2u, 3u, 7u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData, "AAAAAAAAAAIAAAAAAAAAAwAAAAAAAAAH") << "Wrong encoded string.";
	}

	{
		std::array<std::uint64_t, 9u> data
		{
			9u, 7u, 5u, 3u, 1u, 6u, 2u, 4u, 8u
		};

		std::string encodedData = EncodeBase64Str(
			std::data(data), std::size(data), sizeof(decltype(data)::value_type)
		);

		EXPECT_EQ(encodedData,
			"AAAAAAAAAAkAAAAAAAAABwAAAAAAAAAFAAAAAAAAAAMAAAAAAAAAAQAAAAAAAAAGAAAAAAAAAAIAAAAAAAAABAAAAAAAAAAI"
		) << "Wrong encoded string.";
	}
}
