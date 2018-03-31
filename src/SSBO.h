#pragma once

class SSBO{
    unsigned id;
public:
    void init(unsigned binding);
    void deinit();
    void upload(const void* src, unsigned bytes);
    void download(void* dest, unsigned bytes);
};

