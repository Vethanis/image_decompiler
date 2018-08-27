#include "db.h"

#include <cassert>
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <algorithm>
#include "sha3.h"

s32 DB::ID::Compare(const ID& other) const 
{
    const s32* A = (const s32*)m_bytes;
    const s32* B = (const s32*)other.m_bytes;
    for(u32 i = 0; i < sizeof(m_bytes) / sizeof(s32); ++i)
    {
        s32 diff = A[i] - B[i];
        if(diff)
        {
            return diff;
        }
    }
    return 0;
}

void DB::Open(DB::Context& ctx, const char* filename)
{
    assert(filename);

    DB::Guard g(ctx.m_lock);

    assert(!ctx.m_pFile);
    assert(!ctx.m_filename);

    ctx.m_filename = filename;
    ctx.m_pFile = fopen(ctx.m_filename, "rb");
    assert(ctx.m_pFile);

    ctx.m_metadata.reset();
    size_t num_records = 0;
    fread(&num_records, sizeof(size_t), 1, ctx.m_pFile);
    ctx.m_metadata.resizeGrow((s32)num_records);
    fread(ctx.m_metadata.begin(), sizeof(DB::Record), num_records, ctx.m_pFile);
    std::sort(ctx.m_metadata.begin(), ctx.m_metadata.end());
    ctx.m_sorted = true;
}

void DB::Close(DB::Context& ctx)
{
    DB::Guard g(ctx.m_lock);

    char filename[256] = {0};
    sprintf(filename, "%s_tmp", ctx.m_filename);
    FILE* pTmp = fopen(filename, "wb");
    assert(pTmp);

    const size_t num_records = ctx.m_metadata.count();
    fwrite(&num_records, sizeof(size_t), 1, pTmp);
    const uint32_t header_begin = ftell(pTmp);
    if(!ctx.m_sorted)
    {
        std::sort(ctx.m_metadata.begin(), ctx.m_metadata.end());
        ctx.m_sorted = true;
    }
    fwrite(ctx.m_metadata.begin(), sizeof(DB::Record), num_records, pTmp);
    const uint32_t header_end = ftell(pTmp);

    Vector<u8> image_memory;
    for(DB::Record& record : ctx.m_metadata)
    {
        fseek(ctx.m_pFile, record.begin, SEEK_SET);
        record.begin = ftell(pTmp);
        switch(record.table)
        {
            case DB::eTable::eRun:
            {
                DB::Run run;
                fread(&run, sizeof(DB::Run), 1, ctx.m_pFile);
                fwrite(&run, sizeof(DB::Run), 1, pTmp);
            }
            break;
            case DB::eTable::eSlide:
            {
                DB::Slide slide;
                fread(&slide, sizeof(DB::Slide), 1, ctx.m_pFile);
                fwrite(&slide, sizeof(DB::Slide), 1, pTmp);
            }
            break;
            case DB::eTable::eImage:
            {
                DB::Image image;
                fread(&image, sizeof(DB::Image), 1, ctx.m_pFile);
                const size_t image_bytes = (size_t)(image.image);
                image_memory.resizeGrow((s32)image_bytes);
                fread(image_memory.begin(), image_bytes, 1, ctx.m_pFile);
                fwrite(&image, sizeof(DB::Image), 1, pTmp);
                fwrite(image_memory.begin(), image_bytes, 1, pTmp);
            }
            break;
            default:
            {
                puts("Bad table metadata found.");
                assert(false);
            }
            break;
        }
    }

    fseek(pTmp, header_begin, SEEK_SET);
    fwrite(ctx.m_metadata.begin(), sizeof(DB::Record), num_records, pTmp);
    fclose(pTmp);
    fclose(ctx.m_pFile);
    {
        ctx.m_pFile = fopen(ctx.m_filename, "wb");
        pTmp = fopen(filename, "rb");

        fseek(pTmp, 0, SEEK_END);
        const size_t file_size = ftell(pTmp);
        fseek(pTmp, 0, SEEK_SET);

        char buffer[256] = {0};
        size_t bytes_left = file_size;
        while(bytes_left > sizeof(buffer))
        {
            fread(buffer, sizeof(buffer), 1, pTmp);
            fwrite(buffer, sizeof(buffer), 1, ctx.m_pFile);
            bytes_left -= sizeof(buffer);
        }
        if(bytes_left > 0)
        {
            fread(buffer, bytes_left, 1, pTmp);
            fwrite(buffer, bytes_left, 1, ctx.m_pFile);
            bytes_left -= bytes_left;
        }

        fclose(ctx.m_pFile);
        fclose(pTmp);
        ctx.m_metadata.reset();
        ctx.m_pFile = nullptr;
        pTmp = nullptr;
        ctx.m_filename = nullptr;
    }
}

void StringHash(const char* str, sha3_ctx* ctx)
{
    size_t len = strlen(str);
    rhash_sha3_update(ctx, (const u8*)str, len);
}

void ByteHash(const void* p, size_t bytes, sha3_ctx* ctx)
{
    rhash_sha3_update(ctx, (const u8*)p, bytes);
}

void DB::ComputeID(const DB::Run& run, DB::ID& id)
{
    sha3_ctx ctx;
    rhash_sha3_256_init(&ctx);
    ByteHash(&run, sizeof(DB::Run), &ctx);
    rhash_sha3_final(&ctx, id.m_bytes);
}

void DB::ComputeID(const DB::Slide& slide, DB::ID& id)
{
    sha3_ctx ctx;
    rhash_sha3_256_init(&ctx);
    ByteHash(&slide.run_id, sizeof(DB::Slide) - sizeof(DB::ID), &ctx);
    rhash_sha3_final(&ctx, id.m_bytes);
}

void DB::ComputeID(const DB::Image& image, DB::ID& id)
{
    sha3_ctx ctx;
    rhash_sha3_256_init(&ctx);
    ByteHash(&image.width, sizeof(s32) * 2, &ctx);
    const size_t image_bytes = image.width * image.height * 4 * sizeof(u8);
    ByteHash(image.image, image_bytes, &ctx);
    rhash_sha3_final(&ctx, id.m_bytes);
}

const DB::Record* DB::Find(DB::Context& ctx, const DB::Record& search)
{
    if(!ctx.m_sorted)
    {
        std::sort(ctx.m_metadata.begin(), ctx.m_metadata.end());
        ctx.m_sorted = true;
    }
    auto it = std::lower_bound(ctx.m_metadata.begin(), ctx.m_metadata.end(), search);
    if(it == ctx.m_metadata.end())
    {
        return nullptr;
    }
    const DB::Record& record = *it;
    if (record.table != search.table || record.id != search.id)
    {
        return nullptr;
    }
    return &record;
}

bool DB::Read(DB::Context& ctx, const DB::Record& search, void* dst)
{
    DB::Guard g(ctx.m_lock);
    assert(ctx.m_pFile);

    const Record* pRecord = Find(ctx, search);
    if(!pRecord)
        return false;

    fseek(ctx.m_pFile, pRecord->begin, SEEK_SET);
    
    switch(pRecord->table)
    {
        case DB::eTable::eRun:
        {
            fread(dst, sizeof(DB::Run), 1, ctx.m_pFile);
        }
        break;
        case DB::eTable::eSlide:
        {
            fread(dst, sizeof(DB::Slide), 1, ctx.m_pFile);
        }
        break;
        case DB::eTable::eImage:
        {
            DB::Image& image = *(DB::Image*)dst;
            fread(&image, sizeof(DB::Image), 1, ctx.m_pFile);
            const size_t image_bytes = (size_t)image.image;
            image.image = new u8[image_bytes];
            fread(image.image, image_bytes, 1, ctx.m_pFile);
        }
        break;
        default:
        {
            puts("Invalid record table found.");
            assert(false);
            return false;
        }
        break;
    }

    return true;
}

bool DB::Write(DB::Context& ctx, const void* item, Record& record)
{
    switch(record.table)
    {
        case DB::eTable::eRun:
        {
            const DB::Run& run = *(const DB::Run*)item;
            ComputeID(run, record.id);
        }
        break;
        case DB::eTable::eSlide:
        {
            const DB::Slide& slide = *(const DB::Slide*)item;
            ComputeID(slide, record.id);
        }
        break;
        case DB::eTable::eImage:
        {
            const DB::Image& image = *(const DB::Image*)item;
            ComputeID(image, record.id);
        }
        break;
        default:
        {
            assert(false);
        }
        return false;
    }
    DB::Guard g(ctx.m_lock);
    assert(ctx.m_pFile);

    const Record* pExisting = Find(ctx, record);
    if(pExisting)
    {
        fseek(ctx.m_pFile, pExisting->begin, SEEK_SET);
    }
    else
    {
        fseek(ctx.m_pFile, 0, SEEK_END);
        record.begin = ftell(ctx.m_pFile);
        ctx.m_metadata.grow() = record;
        ctx.m_sorted = false;
    }

    switch(record.table)
    {
        case eRun:
        {
            const DB::Run* pRun = (const DB::Run*)item;
            fwrite(pRun, sizeof(DB::Run), 1, ctx.m_pFile);
        }
        break;
        case eSlide:
        {
            const DB::Slide* pSlide = (const DB::Slide*)item;
            fwrite(pSlide, sizeof(DB::Slide), 1, ctx.m_pFile);
        }
        break;
        case eImage:
        {
            const DB::Image* pImage = (const DB::Image*)item;
            const size_t img_bytes = pImage->width * pImage->height * 4 * sizeof(u8);
            DB::Image toWrite = *pImage;
            memcpy(&toWrite.image, &img_bytes, sizeof(size_t));
            fwrite(&toWrite, sizeof(DB::Image), 1, ctx.m_pFile);
            fwrite(pImage->image, img_bytes, 1, ctx.m_pFile);
        }
        break;
        default:
        {
            assert(false);
        }
        return false;
    }

    return true;
}