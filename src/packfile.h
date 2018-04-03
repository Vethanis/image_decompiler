#pragma once

#include "array.h"
#include "store.h"
#include "circular_queue.h"
#include <cstdio>

template<unsigned t_capacity>
struct Packfile
{
    struct Location
    {
        size_t m_begin=0;
        size_t m_size=0;
    };
    struct Placement 
    {
        void* m_value=nullptr;
        unsigned m_name=0;
    };
    Store<Location, t_capacity> m_locations;
    Store<void*, t_capacity> m_loaded;
    CircularQueue<unsigned, t_capacity> m_requests;
    CircularQueue<Placement, t_capacity> m_placements;

    const char* m_filename=nullptr;
    FILE* m_pFile = nullptr;
    size_t m_beginOffset = 0;
    size_t m_currentOffset = 0;

    void Init(const char* filename)
    {
        m_filename = filename;
        m_pFile = fopen(m_filename, "rb");
        assert(m_pFile);
        // load metadata
        unsigned num_items = 0;
        fread(&num_items, sizeof(unsigned), 1, m_pFile);
        assert(num_items > 0 && num_items <= t_capacity);
        Vector<unsigned> names;
        Vector<Location> locations;
        names.resize(num_items);
        locations.resize(num_items);
        fread(names.begin(), names.bytes(), 1, m_pFile);
        fread(locations.begin(), locations.bytes(), 1, m_pFile);
        for(unsigned i = 0; i < num_items; ++i)
        {
            m_locations.insert(names[i], locations[i]);
        }
        m_beginOffset = ftell(m_pFile);
    }
    void Save()
    {
        fseek(m_pFile, 0, SEEK_SET);
    }
    void Deinit()
    {
        m_requests.clear();
        while(m_placements.count())
        {
            Placement p = m_placements.pop();
            free(p.m_value);
        }
        fclose(m_pFile);
        m_pFile = nullptr;
        for(void* pItem : m_loaded)
        {
            free(pItem);
        }
        m_loaded.clear();
    }
    ~Packfile()
    {
        deinit();
    }
    void Request(unsigned name)
    {
        if(m_requests.full() == false)
        {
            m_requests.push(name);
        }
    }
    void Release(unsigned name)
    {
        void** ppValue = m_loaded[name];
        if(ppValue)
        {
            free(*ppValue);
            m_loaded.remove(name);
        }
    }
    void Add(unsigned name, void* value)
    {
        void** ppValue = m_loaded[name];
        if(ppValue)
        {
            free(ppValue);
            *ppValue = value;
        }
        else
        {
            m_loaded.insert(name, value);
        }
    }
    void* Get(unsigned name)
    {
        void** ppValue = m_loaded[name];
        return ppValue ? *ppValue : nullptr;
    }
    const void* Get(unsigned name) const
    {
        void** ppValue = m_loaded[name];
        return ppValue ? *ppValue : nullptr;
    }
    void FulfillRequests()
    {
        while(m_requests.empty() == false && m_placements.full() == false)
        {
            unsigned name = m_requests.pop();
            if(m_loaded[name])
            {
                continue;
            }
            const Location* pLoc = m_locations[name];
            if(pLoc)
            {
                fseek(m_pFile, m_beginOffset + pLoc->m_begin, SEEK_SET);
                void* dest = malloc(pLoc->m_size);
                fread(dest, pLoc->m_size, 1, m_pFile);
                Placement p;
                p.m_name = name;
                p.m_value = dest;
                m_placements.push(placement);
            }
        }
    }
    void PlacePlacements()
    {
        while(m_placements.empty() == false)
        {
            Placement p = m_placements.pop();
            if(!m_loaded[p.m_name])
            {
                m_loaded.insert(p.m_name, p.m_value);
            }
            else
            {
                free(p.m_value);
            }
        }
    }
};