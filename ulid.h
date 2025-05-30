
// COPIED from https://github.com/ChrisBove/ulid/blob/master/src/ulid_uint128.hh
// fixes the original related to this issue
// https://github.com/suyash/ulid/issues/13

#ifndef ULID_UINT128_HH
#define ULID_UINT128_HH

// openssl rand is a much better source than std::rand
// which depending on OS and config uses the same seed & sequence from program start
#include <string_view>
#if ULID_ENABLE_OPENSSL && __has_include(<openssl/rand.h>)
  #define ULID_HAS_OPENSSL 1
  #include <openssl/rand.h>
#else
  #define ULID_HAS_OPENSSL 0
  #include <cstdlib>

  // Mock the OpenSSL RAND_bytes function using std::rand when OpenSSL is not available
  inline int RAND_bytes(unsigned char* buf, int num) {
    for (int i = 0; i < num; ++i) {
      buf[i] = static_cast<unsigned char>(std::rand() & 0xFF);
    }
    return 1;
  }
#endif

#if __has_include(<boost/range/algorithm/copy.hpp>) && __has_include(<boost/uuid/uuid.hpp>)
  #define ULID_HAS_BOOST 1
  #include <boost/range/algorithm/copy.hpp>
  #include <boost/uuid/uuid.hpp>
#else
  #define ULID_HAS_BOOST 0
  // Define a fallback or stub for boost::uuids::uuid if needed
  namespace boost {
    namespace uuids {
      struct uuid {
        unsigned char data[16];
        typedef unsigned char value_type;
        typedef unsigned char* iterator;
        typedef const unsigned char* const_iterator;
        iterator begin() { return data; }
        const_iterator begin() const { return data; }
        iterator end() { return data + 16; }
        const_iterator end() const { return data + 16; }
      };
    }  // namespace uuids
  }  // namespace boost
#endif

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <random>
#include <span>
#include <vector>

#if _MSC_VER > 0
typedef uint32_t rand_t;
#else
typedef uint8_t rand_t;
#endif

namespace ulid {

const int STR_SIZE = 26;
const int BIN_SIZE = 16;

/**
 * ULID is a 16 byte Universally Unique Lexicographically Sortable Identifier
 * */
typedef __uint128_t ULID;

/**
 * EncodeTime will encode the time point to the passed ulid
 * */
inline void EncodeTime(std::chrono::time_point<std::chrono::system_clock> time_point, ULID& ulid) {
	auto time_ms			= std::chrono::time_point_cast<std::chrono::milliseconds>(time_point);
	int64_t timestamp = time_ms.time_since_epoch().count();

	// NOLINTBEGIN
	ULID t = static_cast<uint8_t>(timestamp >> 40);

	t <<= 8;
	t |= static_cast<uint8_t>(timestamp >> 32);

	t <<= 8;
	t |= static_cast<uint8_t>(timestamp >> 24);

	t <<= 8;
	t |= static_cast<uint8_t>(timestamp >> 16);

	t <<= 8;
	t |= static_cast<uint8_t>(timestamp >> 8);

	t <<= 8;
	t |= static_cast<uint8_t>(timestamp);

	t <<= 80;

	ULID mask = 1;
	mask <<= 80;
	mask--;

	ulid = t | (ulid & mask);
	// NOLINTEND
}

/**
 * EncodeTimeNow will encode a ULID using the time obtained using
 * std::time(nullptr)
 * */
inline void EncodeTimeNow(ULID& ulid) {
	auto time_now = std::chrono::system_clock::from_time_t(time(nullptr));
	EncodeTime(time_now, ulid);
}

/**
 * EncodeTimeSystemClockNow will encode a ULID using the time obtained using
 * std::chrono::system_clock::now() by taking the timestamp in milliseconds.
 * */
inline void EncodeTimeSystemClockNow(ULID& ulid) {
	EncodeTime(std::chrono::system_clock::now(), ulid);
}

/**
 * EncodeEntropy will encode the last 10 bytes of the passed uint8_t array with
 * the values generated using the passed random number generator.
 * */
inline void EncodeEntropy(const std::function<uint8_t()>& rng, ULID& ulid) {
	// NOLINTBEGIN
	ulid = (ulid >> 80) << 80;

	ULID e = rng();

	e <<= 8;
	e |= rng();

	e <<= 8;
	e |= rng();

	e <<= 8;
	e |= rng();

	e <<= 8;
	e |= rng();

	e <<= 8;
	e |= rng();

	e <<= 8;
	e |= rng();

	e <<= 8;
	e |= rng();

	e <<= 8;
	e |= rng();

	e <<= 8;
	e |= rng();

	ulid |= e;
	// NOLINTEND
}

/**
 * EncodeEntropyRand will encode a ulid using openssl RAND_bytes
 * */
inline void EncodeEntropyRand(ULID& ulid) {
	// NOLINTBEGIN
	ulid = (ulid >> 80) << 80;

	uint8_t buffer[10];

	if (RAND_bytes(buffer, sizeof(buffer)) != 1) {
		throw std::runtime_error("Failed to generate random bytes with OpenSSL");
	}

	ULID e = buffer[0];

	e <<= 8;
	e |= buffer[1];

	e <<= 8;
	e |= buffer[2];

	e <<= 8;
	e |= buffer[3];

	e <<= 8;
	e |= buffer[4];

	e <<= 8;
	e |= buffer[5];

	e <<= 8;
	e |= buffer[6];

	e <<= 8;
	e |= buffer[7];

	e <<= 8;
	e |= buffer[8];

	e <<= 8;
	e |= buffer[9];

	ulid |= e;

	// NOLINTEND
}

static std::uniform_int_distribution<rand_t> Distribution_0_255(0, 255);	// NOLINT

/**
 * EncodeEntropyMt19937 will encode a ulid using std::mt19937
 *
 * It also creates a std::uniform_int_distribution to generate values in [0,
 * 255]
 * */
inline void EncodeEntropyMt19937(std::mt19937& generator, ULID& ulid) {
	// NOLINTBEGIN
	ulid = (ulid >> 80) << 80;

	ULID e = Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	e <<= 8;
	e |= Distribution_0_255(generator);

	ulid |= e;
	// NOLINTEND
}

/**
 * Encode will create an encoded ULID with a timestamp and a generator.
 * */
inline void Encode(std::chrono::time_point<std::chrono::system_clock> timestamp,
									 const std::function<uint8_t()>& rng, ULID& ulid) {
	EncodeTime(timestamp, ulid);
	EncodeEntropy(rng, ulid);
}

/**
 * EncodeNowRand = EncodeTimeNow + EncodeEntropyRand.
 * */
inline void EncodeNowRand(ULID& ulid) {
	EncodeTimeNow(ulid);
	EncodeEntropyRand(ulid);
}

/**
 * Create will create a ULID with a timestamp and a generator.
 * */
inline ULID Create(std::chrono::time_point<std::chrono::system_clock> timestamp,
									 const std::function<uint8_t()>& rng) {
	ULID ulid = 0;
	Encode(timestamp, rng, ulid);
	return ulid;
}

/**
 * CreateNowRand:EncodeNowRand = Create:Encode.
 * */
inline ULID CreateNowRand() {
	ULID ulid = 0;
	EncodeNowRand(ulid);
	return ulid;
}

/**
 * Crockford's Base32
 * */
static const std::span<const char, 33> Encoding = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";

/**
 * MarshalTo will marshal a ULID to the passed character array.
 *
 * Implementation taken directly from oklog/ulid
 * (https://sourcegraph.com/github.com/oklog/ulid@0774f81f6e44af5ce5e91c8d7d76cf710e889ebb/-/blob/ulid.go#L162-190)
 *
 * timestamp:
 * dst[0]: first 3 bits of data[0]
 * dst[1]: last 5 bits of data[0]
 * dst[2]: first 5 bits of data[1]
 * dst[3]: last 3 bits of data[1] + first 2 bits of data[2]
 * dst[4]: bits 3-7 of data[2]
 * dst[5]: last bit of data[2] + first 4 bits of data[3]
 * dst[6]: last 4 bits of data[3] + first bit of data[4]
 * dst[7]: bits 2-6 of data[4]
 * dst[8]: last 2 bits of data[4] + first 3 bits of data[5]
 * dst[9]: last 5 bits of data[5]
 *
 * entropy:
 * follows similarly, except now all components are set to 5 bits.
 * */
inline void MarshalTo(const ULID& ulid, std::span<char, STR_SIZE> dst) {
	// NOLINTBEGIN
	// 10 byte timestamp
	dst[0] = Encoding[(static_cast<uint8_t>(ulid >> 120) & 224) >> 5];
	dst[1] = Encoding[static_cast<uint8_t>(ulid >> 120) & 31];
	dst[2] = Encoding[(static_cast<uint8_t>(ulid >> 112) & 248) >> 3];
	dst[3] = Encoding[((static_cast<uint8_t>(ulid >> 112) & 7) << 2) |
										((static_cast<uint8_t>(ulid >> 104) & 192) >> 6)];
	dst[4] = Encoding[(static_cast<uint8_t>(ulid >> 104) & 62) >> 1];
	dst[5] = Encoding[((static_cast<uint8_t>(ulid >> 104) & 1) << 4) |
										((static_cast<uint8_t>(ulid >> 96) & 240) >> 4)];
	dst[6] = Encoding[((static_cast<uint8_t>(ulid >> 96) & 15) << 1) |
										((static_cast<uint8_t>(ulid >> 88) & 128) >> 7)];
	dst[7] = Encoding[(static_cast<uint8_t>(ulid >> 88) & 124) >> 2];
	dst[8] = Encoding[((static_cast<uint8_t>(ulid >> 88) & 3) << 3) |
										((static_cast<uint8_t>(ulid >> 80) & 224) >> 5)];
	dst[9] = Encoding[static_cast<uint8_t>(ulid >> 80) & 31];

	// 16 bytes of entropy
	dst[10] = Encoding[(static_cast<uint8_t>(ulid >> 72) & 248) >> 3];
	dst[11] = Encoding[((static_cast<uint8_t>(ulid >> 72) & 7) << 2) |
										 ((static_cast<uint8_t>(ulid >> 64) & 192) >> 6)];
	dst[12] = Encoding[(static_cast<uint8_t>(ulid >> 64) & 62) >> 1];
	dst[13] = Encoding[((static_cast<uint8_t>(ulid >> 64) & 1) << 4) |
										 ((static_cast<uint8_t>(ulid >> 56) & 240) >> 4)];
	dst[14] = Encoding[((static_cast<uint8_t>(ulid >> 56) & 15) << 1) |
										 ((static_cast<uint8_t>(ulid >> 48) & 128) >> 7)];
	dst[15] = Encoding[(static_cast<uint8_t>(ulid >> 48) & 124) >> 2];
	dst[16] = Encoding[((static_cast<uint8_t>(ulid >> 48) & 3) << 3) |
										 ((static_cast<uint8_t>(ulid >> 40) & 224) >> 5)];
	dst[17] = Encoding[static_cast<uint8_t>(ulid >> 40) & 31];
	dst[18] = Encoding[(static_cast<uint8_t>(ulid >> 32) & 248) >> 3];
	dst[19] = Encoding[((static_cast<uint8_t>(ulid >> 32) & 7) << 2) |
										 ((static_cast<uint8_t>(ulid >> 24) & 192) >> 6)];
	dst[20] = Encoding[(static_cast<uint8_t>(ulid >> 24) & 62) >> 1];
	dst[21] = Encoding[((static_cast<uint8_t>(ulid >> 24) & 1) << 4) |
										 ((static_cast<uint8_t>(ulid >> 16) & 240) >> 4)];
	dst[22] = Encoding[((static_cast<uint8_t>(ulid >> 16) & 15) << 1) |
										 ((static_cast<uint8_t>(ulid >> 8) & 128) >> 7)];
	dst[23] = Encoding[(static_cast<uint8_t>(ulid >> 8) & 124) >> 2];
	dst[24] = Encoding[((static_cast<uint8_t>(ulid >> 8) & 3) << 3) |
										 (((static_cast<uint8_t>(ulid)) & 224) >> 5)];
	dst[25] = Encoding[(static_cast<uint8_t>(ulid)) & 31];
	// NOLINTEND
}

/**
 * Marshal will marshal a ULID to a std::string.
 * */
inline std::string Marshal(const ULID& ulid) {
	std::array<char, STR_SIZE> data{};
	MarshalTo(ulid, data);
	return std::string(data.data(), STR_SIZE);
}

/**
 * MarshalBinaryTo will Marshal a ULID to the passed byte array
 * */
template <typename T = uint8_t>
inline void MarshalBinaryTo(const ULID& ulid, const std::span<T, BIN_SIZE> dst) {
	// NOLINTBEGIN
	// timestamp
	dst[0] = static_cast<T>(ulid >> 120);
	dst[1] = static_cast<T>(ulid >> 112);
	dst[2] = static_cast<T>(ulid >> 104);
	dst[3] = static_cast<T>(ulid >> 96);
	dst[4] = static_cast<T>(ulid >> 88);
	dst[5] = static_cast<T>(ulid >> 80);

	// entropy
	dst[6]	= static_cast<T>(ulid >> 72);
	dst[7]	= static_cast<T>(ulid >> 64);
	dst[8]	= static_cast<T>(ulid >> 56);
	dst[9]	= static_cast<T>(ulid >> 48);
	dst[10] = static_cast<T>(ulid >> 40);
	dst[11] = static_cast<T>(ulid >> 32);
	dst[12] = static_cast<T>(ulid >> 24);
	dst[13] = static_cast<T>(ulid >> 16);
	dst[14] = static_cast<T>(ulid >> 8);
	dst[15] = static_cast<T>(ulid);
	// NOLINTEND
}

// [[clang::unsafe_buffer_usage]] inline void MarshalBinaryTo(const ULID& ulid, std::array<uint8_t,
// BIN_SIZE> dst) { 	return MarshalBinaryTo(ulid, dst);
// }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunsafe-buffer-usage"
/**
 * MarshalBinary will Marshal a ULID to a byte vector.
 * */
inline std::vector<uint8_t> MarshalBinary(const ULID& ulid) {
	std::vector<uint8_t> dst(BIN_SIZE);
	MarshalBinaryTo(ulid, std::span<uint8_t, BIN_SIZE>(dst));
	return dst;
}

inline boost::uuids::uuid MarshalUuid(const ULID& ulid) {
	boost::uuids::uuid uuid;

	// NOLINTBEGIN
	// Marshal using the same byte ordering as MarshalBinaryTo

	// time
	uuid.data[0] = static_cast<uint8_t>(ulid >> 120);
	uuid.data[1] = static_cast<uint8_t>(ulid >> 112);
	uuid.data[2] = static_cast<uint8_t>(ulid >> 104);
	uuid.data[3] = static_cast<uint8_t>(ulid >> 96);
	uuid.data[4] = static_cast<uint8_t>(ulid >> 88);
	uuid.data[5] = static_cast<uint8_t>(ulid >> 80);

	// entropy
	uuid.data[6]	= static_cast<uint8_t>(ulid >> 72);
	uuid.data[7]	= static_cast<uint8_t>(ulid >> 64);
	uuid.data[8]	= static_cast<uint8_t>(ulid >> 56);
	uuid.data[9]	= static_cast<uint8_t>(ulid >> 48);
	uuid.data[10] = static_cast<uint8_t>(ulid >> 40);
	uuid.data[11] = static_cast<uint8_t>(ulid >> 32);
	uuid.data[12] = static_cast<uint8_t>(ulid >> 24);
	uuid.data[13] = static_cast<uint8_t>(ulid >> 16);
	uuid.data[14] = static_cast<uint8_t>(ulid >> 8);
	uuid.data[15] = static_cast<uint8_t>(ulid);
	// NOLINTEND

	return uuid;
}

/**
 * dec storesdecimal encodings for characters.
 * 0xFF indicates invalid character.
 * 48-57 are digits.
 * 65-90 are capital alphabets.
 * */
static const std::array<uint8_t, 256> dec = {
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		/* 0     1     2     3     4     5     6     7  */
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		/* 8     9                                      */
		0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		/*    10(A) 11(B) 12(C) 13(D) 14(E) 15(F) 16(G) */
		0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
		/*17(H)     18(J) 19(K)       20(M) 21(N)       */
		0x11, 0xFF, 0x12, 0x13, 0xFF, 0x14, 0x15, 0xFF,
		/*22(P)23(Q)24(R) 25(S) 26(T)       27(V) 28(W) */
		0x16, 0x17, 0x18, 0x19, 0x1A, 0xFF, 0x1B, 0x1C,
		/*29(X)30(Y)31(Z)                               */
		0x1D, 0x1E, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#pragma GCC diagnostic pop

/**
 * UnmarshalFrom will unmarshal a ULID from the passed character array.
 * */
inline void UnmarshalFrom(const std::string_view str, ULID& ulid) {
	assert(str.size() == STR_SIZE);

	// NOLINTBEGIN
	// timestamp
	ulid = (dec[int(str[0])] << 5) | dec[int(str[1])];

	ulid <<= 8;
	ulid |= (dec[int(str[2])] << 3) | (dec[int(str[3])] >> 2);

	ulid <<= 8;
	ulid |= (dec[int(str[3])] << 6) | (dec[int(str[4])] << 1) | (dec[int(str[5])] >> 4);

	ulid <<= 8;
	ulid |= (dec[int(str[5])] << 4) | (dec[int(str[6])] >> 1);

	ulid <<= 8;
	ulid |= (dec[int(str[6])] << 7) | (dec[int(str[7])] << 2) | (dec[int(str[8])] >> 3);

	ulid <<= 8;
	ulid |= (dec[int(str[8])] << 5) | dec[int(str[9])];

	// entropy
	ulid <<= 8;
	ulid |= (dec[int(str[10])] << 3) | (dec[int(str[11])] >> 2);

	ulid <<= 8;
	ulid |= (dec[int(str[11])] << 6) | (dec[int(str[12])] << 1) | (dec[int(str[13])] >> 4);

	ulid <<= 8;
	ulid |= (dec[int(str[13])] << 4) | (dec[int(str[14])] >> 1);

	ulid <<= 8;
	ulid |= (dec[int(str[14])] << 7) | (dec[int(str[15])] << 2) | (dec[int(str[16])] >> 3);

	ulid <<= 8;
	ulid |= (dec[int(str[16])] << 5) | dec[int(str[17])];

	ulid <<= 8;
	ulid |= (dec[int(str[18])] << 3) | (dec[int(str[19])] >> 2);

	ulid <<= 8;
	ulid |= (dec[int(str[19])] << 6) | (dec[int(str[20])] << 1) | (dec[int(str[21])] >> 4);

	ulid <<= 8;
	ulid |= (dec[int(str[21])] << 4) | (dec[int(str[22])] >> 1);

	ulid <<= 8;
	ulid |= (dec[int(str[22])] << 7) | (dec[int(str[23])] << 2) | (dec[int(str[24])] >> 3);

	ulid <<= 8;
	ulid |= (dec[int(str[24])] << 5) | dec[int(str[25])];
	// NOLINTEND
}

/**
 * Unmarshal will create a new ULID by unmarshaling the passed string.
 * */
 inline ULID Unmarshal(const std::string_view str) {
	ULID ulid = 0;
	UnmarshalFrom(str, ulid);
	return ulid;
}

/**
 * UnmarshalBinaryFrom will unmarshal a ULID from the passed byte array.
 * */
inline void UnmarshalBinaryFrom(const std::span<uint8_t const, BIN_SIZE> b, ULID& ulid) {
	// NOLINTBEGIN
	// timestamp
	ulid = b[0];

	ulid <<= 8;
	ulid |= b[1];

	ulid <<= 8;
	ulid |= b[2];

	ulid <<= 8;
	ulid |= b[3];

	ulid <<= 8;
	ulid |= b[4];

	ulid <<= 8;
	ulid |= b[5];

	// entropy
	ulid <<= 8;
	ulid |= b[6];

	ulid <<= 8;
	ulid |= b[7];

	ulid <<= 8;
	ulid |= b[8];

	ulid <<= 8;
	ulid |= b[9];

	ulid <<= 8;
	ulid |= b[10];

	ulid <<= 8;
	ulid |= b[11];

	ulid <<= 8;
	ulid |= b[12];

	ulid <<= 8;
	ulid |= b[13];

	ulid <<= 8;
	ulid |= b[14];

	ulid <<= 8;
	ulid |= b[15];
	// NOLINTEND
}

[[clang::unsafe_buffer_usage]] inline void UnmarshalBinaryFrom(
		const std::array<uint8_t, BIN_SIZE> b, ULID& ulid) {
	return UnmarshalBinaryFrom(std::span<uint8_t const, BIN_SIZE>(b), ulid);
}

inline void UnmarshalBinaryFrom(const boost::uuids::uuid& uuid, ULID& ulid) {
	UnmarshalBinaryFrom(std::span<uint8_t const, BIN_SIZE>(uuid.begin(), uuid.end()), ulid);
}

/**
 * Unmarshal will create a new ULID by unmarshaling the passed byte vector.
 * */
inline ULID UnmarshalBinary(const std::span<uint8_t, BIN_SIZE>& b) {
	ULID ulid = 0;
	UnmarshalBinaryFrom(b, ulid);
	return ulid;
}

inline ULID UnmarshalBinary(const std::span<uint8_t>& b) {
	assert(b.size_bytes() == BIN_SIZE);

	ULID ulid = 0;
	UnmarshalBinaryFrom(std::span<uint8_t, BIN_SIZE>(b.data(), BIN_SIZE), ulid);
	return ulid;
}

inline ULID UnmarshalBinary(const boost::uuids::uuid& uuid) {
	ULID ulid = 0;
	UnmarshalBinaryFrom(uuid, ulid);
	return ulid;
}

/**
 * CompareULIDs will compare two ULIDs.
 * returns:
 *     -1 if ulid1 is Lexicographically before ulid2
 *      1 if ulid1 is Lexicographically after ulid2
 *      0 if ulid1 is same as ulid2
 * */
inline int CompareULIDs(const ULID& ulid1, const ULID& ulid2) {
	return -2 * (ulid1 < ulid2) - 1 * (ulid1 == ulid2) + 1;
}

/**
 * Time will extract the timestamp used to generate a ULID
 * */
inline std::chrono::time_point<std::chrono::system_clock> Time(const ULID& ulid) {
	// NOLINTBEGIN
	int64_t ans = 0;

	ans |= static_cast<uint8_t>(ulid >> 120);

	ans <<= 8;
	ans |= static_cast<uint8_t>(ulid >> 112);

	ans <<= 8;
	ans |= static_cast<uint8_t>(ulid >> 104);

	ans <<= 8;
	ans |= static_cast<uint8_t>(ulid >> 96);

	ans <<= 8;
	ans |= static_cast<uint8_t>(ulid >> 88);

	ans <<= 8;
	ans |= static_cast<uint8_t>(ulid >> 80);

	return std::chrono::time_point<std::chrono::system_clock>(std::chrono::milliseconds{ans});
	// NOLINTEND
}

};	// namespace ulid

#endif	// ULID_UINT128_HH
