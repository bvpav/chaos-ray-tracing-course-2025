#pragma once

#include <cstdint>

namespace crt {

// https://www.cs.hmc.edu/tr/hmc-cs-2014-0905.pdf
//   (massive thanks to this Reddit comment by Christopher Wellons:
//    https://www.reddit.com/r/C_Programming/comments/zbypu1/comment/iyu9a51/)
struct PCG32 {
    uint64_t state, inc;

    constexpr uint32_t operator()() {
        uint64_t old = state;
        state = old * UINT64_C(6364136223846793005) + inc;
        uint32_t xorshifted = ((old >> UINT32_C(18)) ^ old) >> UINT32_C(27);
        uint32_t rot = old >> UINT32_C(59);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    constexpr float uniform() {
        union { uint32_t i; float f; } u;
        // Shift 23 random bits into the mantissa.
        // Make the exponent 127 (a.k.a. 2^0), so it's 1.0.
        u.i = 0x3f800000u | (operator()() >> 9);
        return u.f - 1.0f;
    }
};

inline constexpr PCG32 make_pcg(const uint32_t raster_x, const uint32_t raster_y) noexcept {
    // Pack coordinates into a 64-bit value
    uint64_t seed = (uint64_t(raster_x) << 32) | raster_y;

    // Initialize the generator
    PCG32 rng;
    // Apply PCG seeding rule
    rng.state = 0;
    rng.inc = (seed << 1) | 1; // `inc` has to be odd
    (void)rng(); // Mix the state by advancing
    rng.state += seed;
    (void)rng();
    return rng;
}

}