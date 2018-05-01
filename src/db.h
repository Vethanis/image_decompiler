#pragma once

#include <ctime>
#include <mutex>
#include "ints.h"
#include "array.h"

namespace DB
{
    struct ID;
    struct Run;
    struct Slide;
    struct Image;

    struct ID
    {
        u8 m_bytes[256] = {0};

        bool IsNull() const { for(u8 i : m_bytes) { if(i) return false; } return true; }
        s32 Compare(const ID& other) const;
        bool operator==(const ID& other) const { return Compare(other) == 0; }
        bool operator!=(const ID& other) const { return Compare(other) != 0; }
        bool operator< (const ID& other) const { return Compare(other) <  0; }
    };

    enum eTable : u32
    {
        eRun = 0,
        eSlide,
        eImage
    };
    
    void ComputeID(const Run& run, DB::ID& id);
    void ComputeID(const Slide& slide, DB::ID& id);
    void ComputeID(const Image& image, DB::ID& id);

    struct Run
    {
        ID id;
        ID source_image_id;
        ID brush_image_id;
        char filename[64] = {0};
        char brushname[64] = {0};
        char username[64] = {0};
        time_t time_begun = time_t(0);
        s32 frames_per_primitive = 0;
        s32 seconds_between_screenshots = 0;
        s32 max_primitives = 0;
        f32 primitive_alpha = 0.0f;

        void Finalize() { ComputeID(*this, id); }
    };

    struct Slide
    {
        ID id;
        ID run_id;
        ID image_id;
        f64 difference = 0.0f;
        time_t time_recorded = time_t(0);
        s32 num_primitives = 0;
        s32 sequence = 0;

        void Finalize() { ComputeID(*this, id); }
    };

    struct Image
    {
        ID id;
        s32 width=0;
        s32 height=0;
        u8* image = nullptr;

        void Finalize() { ComputeID(*this, id); }
    };

    struct Record
    {
        ID id;
        eTable table=eRun;
        size_t begin=0;

        bool operator <(const Record& other) const { return id <  other.id; }
        bool operator==(const Record& other) const { return id == other.id; }
    };

    struct Context
    {
        Vector<Record> m_metadata;
        const char* m_filename = nullptr;
        FILE* m_pFile = nullptr;
        std::mutex m_lock;
        bool m_sorted = false;
    };

    typedef std::lock_guard<std::mutex> Guard;

    void Open(Context& ctx, const char* filename);
    void Close(Context& ctx);
    bool Read(Context& ctx, const Record& record, void* out);
    inline bool Read(Context& ctx, Run& item)
    {
        Record record;
        record.id = item.id;
        record.table = eRun;
        return Read(ctx, record, &item);
    }
    inline bool Read(Context& ctx, Slide& item)
    {
        Record record;
        record.id = item.id;
        record.table = eSlide;
        return Read(ctx, record, &item);
    }
    inline bool Read(Context& ctx, Image& item)
    {
        Record record;
        record.id = item.id;
        record.table = eImage;
        return Read(ctx, record, &item);
    }
    bool Write(Context& ctx, const void* item, Record& record);
    inline bool Write(Context& ctx, const Run& item)
    {
        Record record;
        record.id = item.id;
        record.table = eRun;
        return Write(ctx, &item, record);
    }
    inline bool Write(Context& ctx, const Slide& item)
    {
        Record record;
        record.id = item.id;
        record.table = eSlide;
        return Write(ctx, &item, record);
    }
    inline bool Write(Context& ctx, const Image& item)
    {
        Record record;
        record.id = item.id;
        record.table = eImage;
        return Write(ctx, &item, record);
    }
    const Record* Find(Context& ctx, const Record& search);
};
