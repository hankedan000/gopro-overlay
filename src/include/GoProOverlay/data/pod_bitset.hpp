#pragma once

#include <stddef.h>// for size_t
#include <stdint.h>

#define TYPE_WIDTH_IN_BITS(TYPENAME) (sizeof(TYPENAME) * 8)

#define POD_BITSET_GET_N_BIT_UNITS(POD_BITSET_T) (sizeof(typename POD_BITSET_T::unit_array_t) / sizeof(typename POD_BITSET_T::unit_t))
#define POD_BITSET_GET_WIDTH_IN_BITS(POD_BITSET_T) TYPE_WIDTH_IN_BITS(typename POD_BITSET_T::unit_array_t)
// computes the bit's index within pod_bitset::unit_array_t
#define POD_BITSET_GET_BITS_UNIT_IDX(POD_BITSET_T, BIT) (BIT / TYPE_WIDTH_IN_BITS(typename BITSET_T::unit_t))
// computes the bit's index within pod_bitset::unit_t
#define POD_BITSET_GET_BITS_IDX_IN_UNIT(POD_BITSET_T, BIT) (BIT % TYPE_WIDTH_IN_BITS(typename BITSET_T::unit_t))

template<typename UNIT_T, size_t N_BIT_UNITS>
struct pod_bitset
{
    using unit_t = UNIT_T;
    using unit_array_t = UNIT_T[N_BIT_UNITS];

    unit_array_t bit_units;
};

template<class BITSET_T>
void
bitset_clear(
    BITSET_T &bitset)
{
    for (size_t i=0; i<POD_BITSET_GET_N_BIT_UNITS(BITSET_T); i++)
    {
        bitset.bit_units[i] = 0;
    }
}

template<class BITSET_T>
void
bitset_set_bit(
    BITSET_T &bitset,
    size_t bit)
{
    const size_t unit_idx = POD_BITSET_GET_BITS_UNIT_IDX(BITSET_T,bit);
    if (unit_idx < POD_BITSET_GET_N_BIT_UNITS(BITSET_T))
    {
        bitset.bit_units[unit_idx] |= (typename BITSET_T::unit_t)(1) << POD_BITSET_GET_BITS_IDX_IN_UNIT(BITSET_T,bit);
    }
}

template<class BITSET_T>
uint8_t
bitset_get_bit(
    const BITSET_T &bitset,
    size_t bit)
{
    uint8_t bit_value = 0;
    const size_t unit_idx = POD_BITSET_GET_BITS_UNIT_IDX(BITSET_T,bit);
    if (unit_idx < POD_BITSET_GET_N_BIT_UNITS(BITSET_T))
    {
        bit_value = bitset.bit_units[unit_idx] >> POD_BITSET_GET_BITS_IDX_IN_UNIT(BITSET_T,bit);
        bit_value &= 1;
    }
    return bit_value;
}

template<class BITSET_T>
bool
bitset_is_set(
    const BITSET_T &bitset,
    size_t bit)
{
    const size_t unit_idx = POD_BITSET_GET_BITS_UNIT_IDX(BITSET_T,bit);
    if (unit_idx < POD_BITSET_GET_N_BIT_UNITS(BITSET_T))
    {
        return bitset.bit_units[unit_idx] & ((typename BITSET_T::unit_t)(1) << POD_BITSET_GET_BITS_IDX_IN_UNIT(BITSET_T,bit));
    }
    return false;
}