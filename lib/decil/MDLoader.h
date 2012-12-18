//
//  MDLoader.h
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_MDLOADER_H_
#define SILK_LIB_DECIL_MDLOADER_H_

#include "silk/Support/raw_istream.h"
#include <string>

namespace silk
{
    class ErrorHandler;
    namespace decil
    {
        class PEFileReader;
        
        struct MDGUID
        {
            uint32_t offset;
            char guid[16];
        };
        
        struct MDString
        {
            uint32_t offset;
            std::u16string string;
            operator std::u16string () const { return string; }
        };
        
        struct MDBlob
        {
            uint32_t offset;
            uint32_t size;
            std::streampos data_offset;
            PEFileReader *file;
            raw_istream to_istream() const;
            MDBlob();
        };
        
        class MDLoader;
        class MDTokenBase
        {
        public:
            virtual void Load(MDLoader &) = 0;
            virtual unsigned id() const = 0;
            unsigned idx() const { return idx_; }
            MDTokenBase();
            virtual ~MDTokenBase();
            static const int kTagListSentinal = -1;
            operator unsigned() const { return idx(); }
        protected:
            void LoadSimpleToken(MDLoader &loader);
            int LoadCodedToken(MDLoader &loader, const int *tags, size_t tag_length, unsigned tag_bits);
            int LoadCodedToken(unsigned, const int *tags, size_t tag_length, unsigned tag_bits);
            uint32_t idx_;
        };
        
        template <class T>
        class MDSimpleToken : public MDTokenBase
        {
        public:
            virtual void Load(MDLoader &loader)
            { LoadSimpleToken(loader); }
            virtual unsigned id() const
            { return T::id(); }
        };
        
        template <class T>
        class MDCodedToken : public MDTokenBase
        {
        public:
            virtual unsigned id() const { return tbl_id_; }
            
            virtual void Load(MDLoader &loader)
            { tbl_id_ = LoadCodedToken(loader, T::tags, T::tag_length, T::tag_bits); }
            void LoadFromInt(unsigned v)
            { tbl_id_ = LoadCodedToken(v, T::tags, T::tag_length, T::tag_bits); }
            
        private:
            uint32_t tbl_id_;
        };
        
        /* Token used in instructions, ECMA-335, Part III, Sec 1.9 */
        class MDToken : public MDTokenBase
        {
        public:
            virtual void Load(MDLoader &loader);
            void Load(raw_istream &is);
            virtual unsigned id() const { return id_; }
        private:
            unsigned id_;
        };

        struct MetadataTableHeader;
        class MDTableBase;
        
        class MDLoader
        {
        public:
            MDLoader(PEFileReader *file, const raw_istream &is, ErrorHandler &eh);
            
            MDLoader &Load(MDGUID*);
            MDLoader &Load(MDString*);
            MDLoader &Load(MDBlob*);
            MDLoader &Load(MDTokenBase*);
            MDLoader &LoadUserString(MDString*, const MDToken &token);
            
            const MetadataTableHeader & mdt_header() const;
            raw_istream &stream() { return is_; }
            MDTableBase *table(unsigned id) const;
            
        private:
            uint32_t LoadInt(unsigned size);
            static uint32_t ReadBlobOrUserStringSize(raw_istream &is);
            
            PEFileReader *file_;
            raw_istream is_;
            ErrorHandler &eh_;
        };
    }
    
    raw_istream &operator>>(raw_istream &is, decil::MDToken &val);
}

#endif
