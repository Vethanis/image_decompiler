#pragma once 

#include "ints.h"

constexpr u32 fnv_ob_32 = 2166136261;
constexpr u32 fnv_prime_32 = 16777619;
constexpr u64 fnv_ob_64 = 14695981039346656037;
constexpr u64 fnv_prime_64 = 1099511628211;

inline u32 fnv(const char* name)
{
    const u8* data = (const u8*)name;
    u32 val = fnv_ob_32;
    while(*data)
    {
        val ^= *data++;
        val *= fnv_prime_32;
    }
    val |= val==0u;
    return val;
}

inline u32 fnv(const void* p, const u32 len)
{
    const u8* data = (const u8*)p;
    u32 val = fnv_ob_32;
    for(u32 i = 0; i < len; i++)
    {
        val ^= data[i];
        val *= fnv_prime_32;
    }
    val |= val==0u;
    return val;
}

inline u64 fnv64(const char* name)
{
    const u8* data = (const u8*)name;
    u64 val = fnv_ob_64;
    while(*data)
    {
        val ^= *data++;
        val *= fnv_prime_64;
    }
    val |= val==0u;
    return val;
}

inline u64 fnv64(const void* p, const u64 len)
{
    const u8* data = (const u8*)p;
    u64 val = fnv_ob_64;
    for(u64 i = 0; i < len; i++)
    {
        val ^= data[i];
        val *= fnv_prime_64;
    }
    val |= val==0u;
    return val;
}