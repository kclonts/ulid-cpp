
#include <gtest/gtest.h>
#include <chrono>
#include <ctime>
#include <thread>

#include "ulid.h"

namespace {
	static const std::time_t ts_unix = 1484581420;
	static const auto ts = std::chrono::system_clock::from_time_t(ts_unix);
	static const auto encoding = std::string(ulid::Encoding.data(), ulid::Encoding.size());
}

TEST(basic, 1) {
	ulid::ULID ulid = ulid::Create(std::chrono::system_clock::now(), []() { return 4; });
	std::string str = ulid::Marshal(ulid);

	ASSERT_EQ(26, str.size());
	for (char c : str) {
		ASSERT_NE(std::string::npos, encoding.find(c));
	}
}

TEST(Create, 1) {
	ulid::ULID ulid1 = 0;
	ulid::Encode(ts, []() { return 4; }, ulid1);

	auto ulid2 = ulid::Create(ts, []() { return 4; });

	ASSERT_EQ(0, ulid::CompareULIDs(ulid1, ulid2));
}

TEST(EncodeTimeNow, 1) {
	ulid::ULID ulid = 0;
	ulid::EncodeTimeNow(ulid);
	ulid::EncodeEntropy([]() { return 4; }, ulid);
	std::string str = ulid::Marshal(ulid);

	ASSERT_EQ(26, str.size());
	for (char c : str) {
		ASSERT_NE(std::string::npos, encoding.find(c));
	}
}

TEST(EncodeTimeSystemClockNow, 1) {
	ulid::ULID ulid = 0;
	ulid::EncodeTimeSystemClockNow(ulid);
	ulid::EncodeEntropy([]() { return 4; }, ulid);
	std::string str = ulid::Marshal(ulid);

	ASSERT_EQ(26, str.size());
	for (char c : str) {
		ASSERT_NE(std::string::npos, encoding.find(c));
	}
}

TEST(EncodeEntropyRand, 1) {
	ulid::ULID ulid = 0;
	ulid::EncodeTimeNow(ulid);
	ulid::EncodeEntropyRand(ulid);
	std::string str = ulid::Marshal(ulid);

	ASSERT_EQ(26, str.size());
	for (char c : str) {
		ASSERT_NE(std::string::npos, encoding.find(c));
	}
}

TEST(EncodeEntropyRand, 2) {
	time_t timestamp = 1000000;
	auto duration = std::chrono::seconds(timestamp);
	auto nsduration = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
	auto msduration = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

	auto ts = std::chrono::system_clock::from_time_t(timestamp);

	ulid::ULID ulid1 = 0;
	ulid::EncodeTime(ts, ulid1);

	std::srand(nsduration.count());
	ulid::EncodeEntropyRand(ulid1);

	ulid::ULID ulid2 = 0;
	ulid::EncodeTime(ts, ulid2);

	std::srand(nsduration.count());
	ulid::EncodeEntropyRand(ulid2);

	ASSERT_EQ(0, ulid::CompareULIDs(ulid1, ulid2));
}

TEST(EncodeEntropyMt19937, 1) {
	ulid::ULID ulid = 0;
	ulid::EncodeTimeNow(ulid);

	std::mt19937 generator(4);
	ulid::EncodeEntropyMt19937(generator, ulid);

	std::string str = ulid::Marshal(ulid);
	ASSERT_EQ(26, str.size());
	for (char c : str) {
		ASSERT_NE(std::string::npos, encoding.find(c));
	}
}

TEST(EncodeNowRand, 1) {
	ulid::ULID ulid = 0;
	ulid::EncodeNowRand(ulid);
	std::string str = ulid::Marshal(ulid);

	ASSERT_EQ(26, str.size());
	for (char c : str) {
		ASSERT_NE(std::string::npos, encoding.find(c));
	}
}

TEST(CreateNowRand, 1) {
	ulid::ULID ulid = ulid::CreateNowRand();
	std::string str = ulid::Marshal(ulid);

	ASSERT_EQ(26, str.size());
	for (char c : str) {
		ASSERT_NE(std::string::npos, encoding.find(c));
	}
}

TEST(MarshalBinary, 1) {
	ulid::ULID ulid = ulid::Create(ts, []() { return 4; });
	std::vector<uint8_t> b = ulid::MarshalBinary(ulid);

	for (int i = 15 ; i >= 0 ; i--) {
		ASSERT_EQ(static_cast<uint8_t>(ulid), b[i]);
		ulid >>= 8;
	}
}

TEST(Unmarshal, 1) {
	ulid::ULID ulid = ulid::Unmarshal("0001C7STHC0G2081040G208104");

	const auto ms = 1484581420;
	ulid::ULID ulid_expected = ulid::Create(std::chrono::system_clock::time_point(std::chrono::milliseconds(ms)), []() { return 4; });

	ASSERT_EQ(0, ulid::CompareULIDs(ulid_expected, ulid));
}

TEST(UnmarshalBinary, 1) {
	ulid::ULID ulid_expected = ulid::Create(ts, []() { return 4; });
	std::vector<uint8_t> b = ulid::MarshalBinary(ulid_expected);
	ulid::ULID ulid = ulid::UnmarshalBinary(b);
	ASSERT_EQ(0, ulid::CompareULIDs(ulid_expected, ulid));
}

TEST(Time, 1) {
	ulid::ULID ulid = ulid::Create(ts, []() { return 4; });
	ASSERT_EQ(std::chrono::system_clock::from_time_t(1484581420), ulid::Time(ulid));
}

TEST(AlizainCompatibility, 1) {
	const auto ms = 1469918176385;

	ulid::ULID ulid_got = 0;
	ulid::EncodeTime(std::chrono::system_clock::time_point(std::chrono::milliseconds(ms)), ulid_got);

	ulid::ULID ulid_want = ulid::Unmarshal("01ARYZ6S410000000000000000");
	ASSERT_EQ(0, ulid::CompareULIDs(ulid_want, ulid_got));
}

TEST(LexicographicalOrder, 1) {
	ulid::ULID ulid1 = ulid::CreateNowRand();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	ulid::ULID ulid2 = ulid::CreateNowRand();

	EXPECT_EQ(-1, ulid::CompareULIDs(ulid1, ulid2));
	EXPECT_EQ(1, ulid::CompareULIDs(ulid2, ulid1));
}
