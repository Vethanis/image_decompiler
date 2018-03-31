#include "hashstring.h"
#include "namestore.h"
#include "hash.h"

HashString::HashString(const char* str){
    m_hash = fnv(str);
    g_NameStore.insert(m_hash, str);
}

const char* HashString::str() const {
    return g_NameStore[m_hash];
}