#pragma once 

#include "store.h"
#include "ints.h"

struct NameStore
{
    struct TextBlock
    {
        enum : u32
        {
            Capacity = 256,
        };
        char m_text[Capacity];
        TextBlock(){m_text[0] = 0;};
        TextBlock(const char* val)
        {
            u32 i = 0;
            for(; i < Capacity - 1 && val[i]; ++i)
            {
                m_text[i] = val[i];
            }
            m_text[i] = 0;
        }
    };

    Store<TextBlock, 256> m_store;

    const char* get(u32 name)
    {
        TextBlock* res = m_store.get(name);
        return res ? res->m_text : nullptr;
    }
    const char* operator[](u32 name){ return get(name); }
    void insert(u32 name, const char* val)
    {
        if(m_store.get(name))
        {
            return;
        }

        m_store.insert(name, TextBlock(val));
    }
};

extern NameStore g_NameStore;