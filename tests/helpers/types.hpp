#pragma once

#include <cstdint>
#include <tuple>

using IntegralTypes = std::tuple<
    int8_t,  uint8_t,
    int16_t, uint16_t,
    int32_t, uint32_t,
    int64_t, uint64_t
>;

using FloatingPointTypes = std::tuple<
    float,
    double,
    long double
>;

using AllNumberTypes = std::tuple<
    int8_t,  uint8_t,
    int16_t, uint16_t,
    int32_t, uint32_t,
    int64_t, uint64_t,
    float,
    double,
    long double
>;

using AllNonStringAndPathTypes = std::tuple<
    int8_t,  uint8_t,
    int16_t, uint16_t,
    int32_t, uint32_t,
    int64_t, uint64_t,
    float, double, long double,
    bool, char
>;