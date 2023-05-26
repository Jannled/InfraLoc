#ifndef INFRALOC_H
#define INFRALOC_H

#include <cstdint>
#include <bitset>         // std::bitset
#include <array>

#include "mymath.hpp"

typedef uint8_t pin_index_t;

class InfraLoc
{
    private:
        pin_index_t data;
        pin_index_t s0;
        pin_index_t s1;
        pin_index_t s2;
        pin_index_t s3;
        std::array<number_t, 12> buffer;

    public:
        InfraLoc(pin_index_t data, pin_index_t s0, pin_index_t s1, pin_index_t s2, pin_index_t s3);

        void switchChannel(uint8_t channel);
};

#endif // INFRALOC_H