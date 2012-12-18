#ifndef SILK_SUPPORT_RAW_ISTREAM_H_
#define SILK_SUPPORT_RAW_ISTREAM_H_

#include <string>
#include <stdint.h>

namespace llvm {
    class MemoryBuffer;
}

namespace silk {
    
    class raw_istream {
    public:
        raw_istream();
        raw_istream(const llvm::MemoryBuffer *);
        raw_istream(const char *buffer, std::size_t length);
        raw_istream &read(char *s, std::size_t n);
        
        template <class T>
        raw_istream &operator>>(T &v)
        {
            return read<T>(v);
        }
        
        template <class T>
        raw_istream &read(T &v)
        {
            read(reinterpret_cast<char*>(&v), sizeof(T));
            return *this;
        }

        template <class T>
        T peek() const { return *reinterpret_cast<const T*>(cursor_); }
        
        const char *start() const { return buffer_start_; }
        const char *end()   const { return buffer_end_; }
        const char *pos()   const { return cursor_; }
        
        size_t size() const
            { return buffer_end_ - buffer_start_; }
        size_t remaining_bytes() const
            { return buffer_end_ - cursor_; }
        std::streampos tellg() const
            { return cursor_ - buffer_start_; }
        
        bool fail() const { return flags_ & kFailed; }
        bool eof() const  { return cursor_ == buffer_end_; }
        
        raw_istream& skip(std::size_t n);
        raw_istream& align(std::size_t n);
        raw_istream& seek(std::size_t n);
        
        std::string peek_utf8_null_terminated() const;
        std::string read_ascii_null_terminated();
        int read_compressed_uint32();
        
    private:
        enum
        {
            kFailed = 1,
        };
        
        const char *buffer_start_;
        const char *buffer_end_;
        const char *cursor_;
        unsigned flags_;
        //  raw_istream(const raw_istream &);
        //raw_istream & operator=(const raw_istream &);
    };
    
}
#endif
