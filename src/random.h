// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_RANDOM_H
#define BITCOIN_RANDOM_H

#include <crypto/chacha20.h>
#include <crypto/common.h>
#include <uint256.h>

#include <cstdint>
#include <limits>

/**
 * Generate random data via the internal PRNG.
 *
 * These functions are designed to be fast (sub microsecond), but do not
 * necessarily meaningfully add entropy to the PRNG state.
 *
 * Thread-safe.
 */
void GetRandBytes(uint8_t *buf, int num);
uint64_t GetRand(uint64_t nMax);
int GetRandInt(int nMax);
uint256 GetRandHash();

/**
 * Gather entropy from various sources, feed it into the internal PRNG, and
 * generate random data using it.
 *
 * This function will cause failure whenever the OS RNG fails.
 *
 * Thread-safe.
 */
void GetStrongRandBytes(uint8_t *buf, int num);

/**
 * Sleep for 1ms, gather entropy from various sources, and feed them to the PRNG
 * state.
 *
 * Thread-safe.
 */
void RandAddSeedSleep();

/**
 * Fast randomness source. This is seeded once with secure random data, but
 * is completely deterministic and does not gather more entropy after that.
 *
 * This class is not thread-safe.
 */
class FastRandomContext {
private:
    bool requires_seed;
    ChaCha20 rng;

    uint8_t bytebuf[64];
    int bytebuf_size;

    uint64_t bitbuf;
    int bitbuf_size;

    void RandomSeed();

    void FillByteBuffer() {
        if (requires_seed) {
            RandomSeed();
        }
        rng.Output(bytebuf, sizeof(bytebuf));
        bytebuf_size = sizeof(bytebuf);
    }

    void FillBitBuffer() {
        bitbuf = rand64();
        bitbuf_size = 64;
    }

public:
    explicit FastRandomContext(bool fDeterministic = false);

    /** Initialize with explicit seed (only for testing) */
    explicit FastRandomContext(const uint256 &seed);

    // Do not permit copying a FastRandomContext (move it, or create a new one
    // to get reseeded).
    FastRandomContext(const FastRandomContext &) = delete;
    FastRandomContext(FastRandomContext &&) = delete;
    FastRandomContext &operator=(const FastRandomContext &) = delete;

    /**
     * Move a FastRandomContext. If the original one is used again, it will be
     * reseeded.
     */
    FastRandomContext &operator=(FastRandomContext &&from) noexcept;

    /** Generate a random 64-bit integer. */
    uint64_t rand64() {
        if (bytebuf_size < 8) {
            FillByteBuffer();
        }
        uint64_t ret = ReadLE64(bytebuf + 64 - bytebuf_size);
        bytebuf_size -= 8;
        return ret;
    }

    /** Generate a random (bits)-bit integer. */
    uint64_t randbits(int bits) {
        if (bits == 0) {
            return 0;
        } else if (bits > 32) {
            return rand64() >> (64 - bits);
        } else {
            if (bitbuf_size < bits) {
                FillBitBuffer();
            }
            uint64_t ret = bitbuf & (~uint64_t(0) >> (64 - bits));
            bitbuf >>= bits;
            bitbuf_size -= bits;
            return ret;
        }
    }

    /** Generate a random integer in the range [0..range). */
    uint64_t randrange(uint64_t range) {
        --range;
        int bits = CountBits(range);
        while (true) {
            uint64_t ret = randbits(bits);
            if (ret <= range) {
                return ret;
            }
        }
    }

    /** Generate random bytes. */
    std::vector<uint8_t> randbytes(size_t len);

    /** Generate a random 32-bit integer. */
    uint32_t rand32() { return randbits(32); }

    /** generate a random uint256. */
    uint256 rand256();

    /** Generate a random boolean. */
    bool randbool() { return randbits(1); }

    // Compatibility with the C++11 UniformRandomBitGenerator concept
    typedef uint64_t result_type;
    static constexpr uint64_t min() { return 0; }
    static constexpr uint64_t max() {
        return std::numeric_limits<uint64_t>::max();
    }
    inline uint64_t operator()() { return rand64(); }
};

/**
 * More efficient than using std::shuffle on a FastRandomContext.
 *
 * This is more efficient as std::shuffle will consume entropy in groups of
 * 64 bits at the time and throw away most.
 *
 * This also works around a bug in libstdc++ std::shuffle that may cause
 * type::operator=(type&&) to be invoked on itself, which the library's
 * debug mode detects and panics on. This is a known issue, see
 * https://stackoverflow.com/questions/22915325/avoiding-self-assignment-in-stdshuffle
 */
template <typename I, typename R> void Shuffle(I first, I last, R &&rng) {
    while (first != last) {
        size_t j = rng.randrange(last - first);
        if (j) {
            using std::swap;
            swap(*first, *(first + j));
        }
        ++first;
    }
}

/**
 * Number of random bytes returned by GetOSRand.
 * When changing this constant make sure to change all call sites, and make
 * sure that the underlying OS APIs for all platforms support the number.
 * (many cap out at 256 bytes).
 */
static const ssize_t NUM_OS_RANDOM_BYTES = 32;

/**
 * Get 32 bytes of system entropy. Do not use this in application code: use
 * GetStrongRandBytes instead.
 */
void GetOSRand(uint8_t *ent32);

/**
 * Check that OS randomness is available and returning the requested number of
 * bytes.
 */
bool Random_SanityCheck();

/**
 * Initialize global RNG state and log any CPU features that are used.
 *
 * Calling this function is optional. RNG state will be initialized when first
 * needed if it is not called.
 */
void RandomInit();

#endif // BITCOIN_RANDOM_H
