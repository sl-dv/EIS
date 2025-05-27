#ifndef TABUL_NONSTATIC_HPP
#define TABUL_NONSTATIC_HPP

#include <array>
#include <cstdint>
#include <random>

class TabHash {
public:
    using SimpleTable = std::array<std::array<uint32_t, 256>, 4>;
    using TwistedTable = std::array<std::array<uint64_t, 256>, 4>;

    // Constructor initializes the tables with random values
    TabHash() {
        Reset();
    }

    // Hash functions
    uint32_t Simple(uint32_t x) const {
        const auto& H = simpleTable;
        uint32_t h = 0;
        for (int i = 0; i < 4; i++) {
            uint8_t c = static_cast<uint8_t>(x);
            h ^= H[i][c];
            x >>= 8;
        }
        return h;
    }

    uint32_t Twisted(uint32_t x) const {
        const auto& H = twistedTable;
        uint64_t h = 0;
        int i;
        for (i = 0; i < 3; i++) {
            uint8_t c = static_cast<uint8_t>(x);
            h ^= H[i][c];
            x >>= 8;
        }
        uint8_t c = static_cast<uint8_t>(x ^ h);
        h ^= H[i][c];
        h >>= 32;
        return static_cast<uint32_t>(h);
    }

    // Reset the tables with new random values
    void Reset() {
        simpleTable = SetupSimple();
        twistedTable = SetupTwisted();
    }

private:
    SimpleTable simpleTable;
    TwistedTable twistedTable;

    SimpleTable SetupSimple() {
        SimpleTable table;
        std::mt19937 rng(std::random_device{}());
        for (auto& row : table)
            for (auto& cell : row)
                cell = static_cast<uint32_t>(rng());
        return table;
    }

    TwistedTable SetupTwisted() {
        TwistedTable table;
        std::mt19937_64 rng(std::random_device{}());
        for (auto& row : table)
            for (auto& cell : row)
                cell = rng();
        return table;
    }
};

#endif