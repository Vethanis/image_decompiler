#pragma once 

#include <cassert>

template<typename T, const unsigned cap>
class Store
{
    static constexpr unsigned invalid_val = 0xffffff;
    
    T data[cap];
    unsigned names[cap];
    unsigned count;
    unsigned capacity;

    unsigned mask(unsigned key){
        return key & (cap - 1);
    }
    unsigned probe_distance(unsigned pos, unsigned key){
        return mask(pos + cap - mask(key));
    }
    unsigned index_of(unsigned key){
        unsigned pos = mask(key);
        unsigned dist = 0;
        while(true){
            if(names[pos] == key){
                return pos;
            }
            else if(names[pos] == 0 || dist > probe_distance(pos, names[pos])){
                return invalid_val;
            }
            pos = mask(pos + 1);
            ++dist;
        }
        return invalid_val;
    }
public:
    Store(){
        count = 0;
        capacity = cap;
        for(unsigned i = 0; i < capacity; ++i)
        {
            names[i] = 0;
        }
    }

    class iterator
    {
        Store* m_pStore;
        unsigned m_idx;
        void iterate(){
            for(; m_idx < m_pStore->capacity; ++m_idx){
                unsigned key = m_pStore->names[m_idx];
                if(key)
                    break;
            }
        }
    public:
        iterator(Store* pStore, unsigned idx = 0) : m_pStore(pStore), m_idx(idx){
            iterate();
        }
        bool operator != (const iterator& o)const{
            return m_idx != o.m_idx;
        }
        T& operator*(){
            return m_pStore->data[m_idx];
        }
        iterator& operator++(){
            ++m_idx;
            iterate();
            return *this;
        }
    };

    iterator begin(){ return iterator(this); }
    iterator end(){ return iterator(this, capacity); }

    void insert(unsigned key, const T& _val){
        assert(count < cap);
        unsigned pos = mask(key);
        unsigned dist = 0;
        T val = _val;
        while(true){
            if(names[pos] == 0){
                names[pos] = key;
                data[pos] = val;
                count++;
                return;
            }

            unsigned existing_dist = probe_distance(pos, names[pos]);
            if(existing_dist < dist){
                {
                    const unsigned tname = key;
                    key = names[pos];
                    names[pos] = tname;
                }
                {
                    const T tval = val;
                    val = data[pos];
                    data[pos] = tval;
                }

                dist = existing_dist;
            }

            pos = mask(pos + 1);
            ++dist;
        }
    }
    T* get(unsigned key){
        unsigned loc = index_of(key);
        if(loc == invalid_val){
            return nullptr;
        }
        return data + loc;
    }
    T* operator[](unsigned key){
        return get(key);
    }
    void remove(unsigned key){
        unsigned loc = index_of(key);
        if(loc != invalid_val)
        {
            count--;
            names[loc] = 0;
            data[loc] = T();

            unsigned nextLoc = mask(loc + 1);
            unsigned trueLoc = mask(names[nextLoc]);
            while(names[nextLoc] && trueLoc < nextLoc)
            {
                names[loc] = names[nextLoc];
                names[nextLoc] = 0;
                data[loc] = data[nextLoc];
                data[nextLoc] = T();

                loc = nextLoc;
                nextLoc = mask(loc + 1);
                trueLoc = mask(names[nextLoc]);
            }
        }
    }
    bool full(){ return cap == count; }
    void clear()
    {
        for(unsigned i = 0; i < count; ++i)
        {
            names[i] = 0;
        }
        for(unsigned i = 0; i < count; ++i)
        {
            data[i] = T();
        }
    }
};