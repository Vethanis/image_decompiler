#ifndef UBO_H
#define UBO_H

class UBO{
    unsigned id;
public:
    void init(const void* ptr, unsigned size, const char* name, const unsigned* programs, int num_progs);
    void deinit();
    void upload(const void* ptr, unsigned size);
};

#endif
