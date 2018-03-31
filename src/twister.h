#pragma once

#include "ints.h"
#include "assert.h"
#include "hash.h"

template<typename T, const u32 t_capacity, typename C = u16>
class TwArray
{
private:
    T m_data[t_capacity];
    C m_twists[t_capacity];
    C m_tail;
public:
    TwArray()
    {
        m_tail = 0;
    }
    C count() const
    {
        return m_tail;
    }
    C bytes() const
    {
        return sizeof(T) * m_tail;
    }
    u32 hash() const
    {
        return fnv(m_data, bytes());
    }
    bool full() const
    {
        return m_tail == t_capacity;
    }
    bool empty() const
    {
        return m_tail == 0;
    }
    bool valid(C idx) const
    {
        return idx < m_tail && m_twists[idx] < m_tail;
    }
    const T* begin() const
    {
        return m_data;
    }
    T* begin()
    {
        return m_data;
    }
    const T* end() const
    {
        return m_data + m_tail;
    }
    T* end()
    {
        return m_data + m_tail;
    }
    const T& back() const
    {
        assert(count());
        return m_data[m_tail - 1];
    }
    T& back()
    {
        assert(count());
        return m_data[m_tail - 1];
    }
    const T& operator[](C idx) const
    {
        const C pos = m_twists[idx];
        assert(pos < t_capacity);
        return m_data[pos];
    }
    T& operator[](C idx)
    {
        const C pos = m_twists[idx];
        assert(pos < t_capacity);
        return m_data[pos];
    }
    C find(const T& item) const
    {
        for(C i = 0; i < m_tail; ++i)
        {
            const C pos = m_twists[i];
            if(pos < m_tail && m_data[pos] == item)
            {
                return i;
            }
        }
        return (C)-1;
    }
    void clear()
    {
        m_tail = 0;
    }
    C insert(const T& item)
    {
        assert(!full());
        const C pos = m_tail;
        ++m_tail;
        m_data[pos] = item;
        m_twists[pos] = pos;
        return pos;
    }
    C create()
    {
        assert(!full());
        const C pos = m_tail;
        ++m_tail;
        m_twists[pos] = pos;
        return pos;
    }
    void remove(C idx)
    {
        assert(idx < t_capacity);
        assert(count());
        const C pos = m_twists[idx];
        const C tail = m_tail - 1;
        m_data[pos] = m_data[tail];
        m_twists[idx] = (C)-1;
        m_twists[tail] = pos;
    }
    bool findRemove(const T& item)
    {
        assert(!empty());
        for(C i = 0; i < m_tail; ++i)
        {
            if(m_data[i] == item)
            {
                const C tail = m_tail - 1;
                m_data[i] = m_data[tail];
                m_twists[i] = (C)-1;
                m_twists[tail] = i;
                return true;
            }
        }
        return false;
    }
};