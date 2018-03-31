#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <cstdio>
#include <cassert>
#include <cstring>

struct HashString{
    unsigned m_hash;
    const char* str() const;
    bool valid()const{ return m_hash != 0; }
    HashString(){m_hash = 0;}
    HashString(unsigned hash){m_hash = hash;}
    HashString(const char* str);
    HashString operator = (HashString o) { m_hash = o.m_hash; return *this; }
    HashString operator = (const char* str) { *this = HashString(str); return *this; }
    HashString operator = (unsigned hash) { m_hash = hash; return *this; }
    operator unsigned () const { return m_hash; }
    operator const char* () const { return str(); }
    bool operator==(HashString other)const{ return m_hash == other.m_hash; }

    template<typename T>
    operator T* () const;

    void serialize(FILE* pFile){
        const char* s = str();
        assert(s);
        const unsigned len = (unsigned)strlen(s);
        fwrite(&len, sizeof(unsigned), 1, pFile);
        fwrite(s, 1, len, pFile);
    }
    void load(FILE* pFile){
        unsigned len = 0;
        fread(&len, sizeof(unsigned), 1, pFile);
        char* buf = new char[len + 1];
        fread(buf, 1, len, pFile);
        buf[len] = 0;
        *this = HashString(buf);
    }
};