//
//  raw_istream.cpp
//  silk
//
//  Created by Haohui Mai on 9/20/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "silk/Support/raw_istream.h"

#include <llvm/Support/MemoryBuffer.h>
#include <cassert>

namespace silk {
    raw_istream::raw_istream()
    : buffer_start_(NULL)
    , buffer_end_(NULL)
    , cursor_(NULL)
    , flags_(0)
    {}
    
    raw_istream::raw_istream(const llvm::MemoryBuffer *buf)
    : buffer_start_(buf->getBufferStart())
    , buffer_end_(buf->getBufferEnd())
    , cursor_(buffer_start_)
    , flags_(0)
    {}
    
    raw_istream::raw_istream(const char *buffer, std::size_t length)
    : buffer_start_(buffer)
    , buffer_end_(buffer + length)
    , cursor_(buffer_start_)
    , flags_(0)
    {}
    
    raw_istream &raw_istream::read(char *s, std::size_t n)
    {
        std::memcpy(s, cursor_, n);
        return skip(n);
    }
    
    raw_istream &raw_istream::skip(std::size_t n)
    {
        assert (cursor_ + n <= buffer_end_);
        cursor_ += n;
        return *this;
    }
    
    raw_istream &raw_istream::align(std::size_t n)
    {
        size_t i = (uintptr_t)cursor_ % n;
        if (i != 0)
            skip(n - i);
        return *this;
    }
    
    raw_istream &raw_istream::seek(std::size_t n)
    {
        if (n >= (unsigned)(buffer_end_ - buffer_start_))
        {
            flags_ |= kFailed;
            return *this;
        }
        cursor_ = buffer_start_ + n;
        return *this;
    }
    
    std::string raw_istream::peek_utf8_null_terminated() const
    {
        size_t l = strnlen(cursor_, remaining_bytes());
        return std::string(cursor_, l);
    }
    
    std::string raw_istream::read_ascii_null_terminated()
    {
        size_t l = strnlen(cursor_, remaining_bytes());
        std::string res(cursor_, l);
        skip(l + 1);
        return res;
    }
    
    int raw_istream::read_compressed_uint32()
    {
        unsigned char b0, b1, b2, b3;
        *this >> b0;
        
        if ((b0 & 0x80) == 0x00)
        {
            return b0;
        }
        else if ((b0 & 0x40) == 0x00)
        {
            *this >> b1;
            return ((b0 & 0x3f) << 8) | b1;
        }
        else if (b0 == 0xFF)
        {
            return -1;
        }
        else
        {
            *this >> b1 >> b2 >> b3;
            return ((b0 & 0x3f) << 24) | (b1 << 16) || (b2 << 8) || b3;
        }
    }
}

