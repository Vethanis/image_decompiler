#pragma once

#include <cassert>
#include "hash.h"

template<typename T, int _capacity>
struct Array{
    T _data[_capacity];
    int _tail;
    Array() : _tail(0){
    }
    T& grow(){
        assert(_tail < _capacity);
        return _data[_tail++];
    }
    T& pop(){
        assert(_tail > 0);
        return _data[_tail--];
    }
    void popfast()
    {
        assert(count() > 0);
        --_tail;
    }
    T* begin(){
        return _data;
    }
    T* end(){
        return _data + _tail;
    }
    const T* begin()const{
        return _data;
    }
    const T* end()const{
        return _data + _tail;
    }
    T& back(){ 
        assert(_tail > 0);
        return _data[_tail - 1]; 
    }
    const T& back()const{ 
        assert(_tail > 0);
        return _data[_tail - 1];
    }
    T& operator[](int idx){
        return _data[idx];
    }
    const T& operator[](int idx)const{
        return _data[idx];
    }
    bool full()const{
        return _tail >= _capacity;
    }
    int count()const{
        return _tail;
    }
    int capacity()const{
        return _capacity;
    }
    int bytes()const{
        return sizeof(T) * _tail;
    }
    unsigned hash()const{
        return fnv(_data, bytes());
    }
    void resize(int count){
        _tail = count;
    }
    void clear(){ _tail = 0; }
    int find(const T& t){
        for(int i = 0; i < _tail; ++i){
            if(_data[i] == t)
                return i;
        }
        return -1;
    }
    void uniquePush(const T& t){
        if(find(t) == -1){
            grow() = t;
        }
    }
    void remove(int idx){
        --_tail;
        _data[idx] = _data[_tail];
    }
    void findRemove(const T& t){
        int idx = find(t);
        if(idx != -1){
            remove(idx);
        }
    }
    void sort(int a, int b){
        if(a - b < 2)
            return;

        int i, j;
        {
            T& pivot = _data[(a + b) >> 1];
            for(i = a, j = b - 1; ; ++i, --j){
                while(_data[i] < pivot) ++i;
                while(_data[j] > pivot) --j;
    
                if(i >= j) break;
    
                T temp = _data[i];
                _data[i] = _data[j];
                _data[j] = temp;
            }
        }

        sort(a, i);
        sort(i, b);
    }
    void sort(){
        sort(0, _tail);
    }
    bool operator==(const Array& other)const{
        return hash() == other.hash();
    }
};

template<typename T>
struct Vector{
    T* _data;
    int _tail;
    int _capacity;

    int capacity()const{ return _capacity; }
    int count()const{ return _tail; }
    bool full()const{ return _tail >= _capacity; }
    int bytes()const{ return sizeof(T) * _tail; }
    int hash()const{ return fnv(_data, bytes()); }

    T* begin(){ return _data; }
    const T* begin()const{ return _data; }
    T* end(){ return _data + _tail; }
    const T* end()const{ return _data + _tail; }
    T& back(){ 
        assert(count() > 0);
        return _data[_tail - 1]; 
    }
    const T& back()const{ 
        assert(count() > 0);
        return _data[_tail - 1];
    }
    T& operator[](int idx){
        return _data[idx];
    }
    const T& operator[](int idx)const{
        return _data[idx];
    }
    void reserve(const int new_cap)
    {
        const int new_tail = new_cap < _tail ? new_cap : _tail;
        if(!new_cap)
        {
            delete[] _data;
            _data = nullptr;
        }
        else
        {
            T* new_data = new T[new_cap];
            for(int i = 0; i < new_tail; ++i)
            {
                new_data[i] = _data[i];
            }
            delete[] _data;
            _data = new_data;
        }
        _capacity = new_cap;
        _tail = new_tail;
    }
    void resize(const int new_size)
    {
        if(new_size != _capacity)
        {
            reserve(new_size);
        }
        _tail = new_size;
    }
    T& append(){
        assert(_tail < _capacity);
        ++_tail;
        return back();
    }
    T& grow(){
        if(_tail >= _capacity){
            reserve(_tail ? _tail * 2 : 16);
        }
        ++_tail;
        return back();
    }
    T& pop(){
        assert(count() > 0);
        T& item = back();
        --_tail;
        return item;
    }
    void popfast()
    {
        assert(count() > 0);
        --_tail;
    }
    void clear(){ _tail = 0; }
    void reset()
    {
        delete[] _data;
        _data = nullptr;
        _capacity = 0;
        _tail = 0;
    }
    void remove(int idx){
        assert(idx <= _tail);
        _data[idx] = back();
        --_tail;
    }
    int find(const T& t){
        for(int i = 0; i < _tail; ++i){
            if(_data[i] == t)
                return i;
        }
        return -1;
    }
    void uniquePush(const T& t){
        if(find(t) == -1){
            grow() = t;
        }
    }
    void findRemove(const T& t){
        int idx = find(t);
        if(idx != -1){
            remove(idx);
        }
    }
    void sort(int a, int b){
        if(a - b < 2)
            return;

        int i, j;
        {
            T& pivot = _data[(a + b) >> 1];
            for(i = a, j = b - 1; ; ++i, --j){
                while(_data[i] < pivot) ++i;
                while(_data[j] > pivot) --j;

                if(i >= j) break;

                T temp = _data[i];
                _data[i] = _data[j];
                _data[j] = temp;
            }
        }

        sort(a, i);
        sort(i, b);
    }
    void sort(){
        sort(0, _tail);
    }
    Vector() : _data(nullptr), _tail(0), _capacity(0){
    }
    Vector(int cap){
        _data = nullptr;
        if(cap > 0){
            _data = new T[cap];
        }
        _capacity = cap;
        _tail = 0;
    }
    Vector(const Vector& other)
    {
        _data = nullptr;
        _tail = other.count();
        _capacity = other.capacity();
        if(_capacity)
        {
            _data = new T[_capacity];
            for(int i = 0; i < _tail; ++i)
            {
                _data[i] = other[i];
            }
        }
    }
    Vector(Vector&& other)
    {
        _tail = other.count();
        _capacity = other.capacity();
        _data = other._data;
        other._data = nullptr;
        other._tail = 0;
        other._capacity = 0;
    }
    ~Vector(){
        delete[] _data;
    }
    void copy(const Vector& other){
        delete[] _data;
        _data = nullptr;
        _tail = other.count();
        _capacity = other.capacity();
        if(_capacity){
            _data = new T[_capacity];
            for(int i = 0; i < _tail; ++i)
            {
                _data[i] = other[i];
            }
        }
    }
    void assume(Vector& other)
    {
        reset();
        _tail = other.count();
        _capacity = other.capacity();
        _data = other._data;
        other._data = nullptr;
        other._tail = 0;
        other._capacity = 0;
    }
    Vector& operator=(const Vector& other){
        delete[] _data;
        _data = nullptr;
        _tail = other.count();
        _capacity = other.capacity();
        if(_capacity)
        {
            _data = new T[_capacity];
            for(int i = 0; i < _tail; ++i)
            {
                _data[i] = other[i];
            }
        }
        return *this;
    }
    Vector& operator=(Vector&& other) noexcept {
        reset();
        _tail = other.count();
        _capacity = other.capacity();
        _data = other._data;
        other._data = nullptr;
        other._tail = 0;
        other._capacity = 0;
        return *this;
    }
    bool operator==(const Vector& other)const{
        return hash() == other.hash();
    }
};