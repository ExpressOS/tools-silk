//
//  MDLoader.cpp
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "MDLoader.h"
#include "Metadata.h"
#include "PEFileReader.h"

#include <locale>
#include <codecvt>
#include <iostream>

namespace silk
{
    raw_istream &operator>>(raw_istream &is, decil::MDToken &val)
    {
        val.Load(is);
        return is;
    }
    namespace decil
    {
        static uint32_t LowerBitMask(unsigned int i)
        {
            return (1 << i) - 1;
        }
        
        MDTokenBase::MDTokenBase()
        : idx_(0)
        {}
        
        MDTokenBase::~MDTokenBase()
        {}
        
        void MDTokenBase::LoadSimpleToken(MDLoader &loader)
        {
            if (loader.table(id())->is_full_id())
            {
                loader.stream() >> idx_;
            }
            else
            {
                uint16_t v;
                loader.stream() >> v;
                idx_ = v;
            }
        }
        
        int MDTokenBase::LoadCodedToken(MDLoader & loader, const int tags[], size_t tag_length, unsigned tag_bits) {
            std::vector<int> table_ids(tags, tags + tag_length);
            
            bool has_big_table = false;
            for (auto id : table_ids)
            {
                auto table = loader.table(id);
                if (table && table->is_full_id())
                {
                    has_big_table = true;
                    break;
                }
            }
            
            uint32_t v;
            if (has_big_table) {
                loader.stream() >> v;
            } else {
                uint16_t v1;
                loader.stream() >> v1;
                v = v1;
            }
            
            idx_ = v >> tag_bits;
            return table_ids.at(v & LowerBitMask(tag_bits));
        }
        
        int MDTokenBase::LoadCodedToken(unsigned v, const int tags[], size_t tag_length, unsigned tag_bits)
        {
            std::vector<int> table_ids(tags, tags + tag_length);
            idx_ = v >> tag_bits;
            return table_ids.at(v & LowerBitMask(tag_bits));
        }
        
        void MDToken::Load(MDLoader &) {}
        void MDToken::Load(raw_istream & is)
        {
            uint32_t t;
            is >> t;
            id_ = t >> 24;
            idx_ = t & 0xffffff;
        }
        
        MDBlob::MDBlob()
        : offset(0)
        , size(0)
        , data_offset(0)
        , file(nullptr)
        {}
        
        raw_istream MDBlob::to_istream() const
        {
            raw_istream is = file->md_stream(PEFileReader::kBlobStream);
            is.seek(data_offset);
            return raw_istream(is.pos(), size);
        }
        
        MDLoader::MDLoader(PEFileReader *file, const raw_istream &is, ErrorHandler &eh)
        : file_(file)
        , is_(is)
        , eh_(eh)
        {}
        
        MDLoader & MDLoader::Load(MDGUID *v)
        {
            v->offset = LoadInt(mdt_header().GUIDIndexSize());
            raw_istream &is = file_->md_stream(PEFileReader::kGUIDStream);
            memcpy(v->guid, is.start() + v->offset, sizeof(v->guid));
            return *this;
        }
        
        MDLoader & MDLoader::Load(MDString *v)
        {
            v->offset = LoadInt(mdt_header().StringIndexSize());
            raw_istream &is = file_->md_stream(PEFileReader::kStringStream);
            is.seek(v->offset);
            
            auto str = is.peek_utf8_null_terminated();
            if (str.length() != 0)
            {
                std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
                v->string = utf16conv.from_bytes(str);
            }
            else
            {
                v->string.clear();
            }
            
            return *this;
        }
        
        MDLoader & MDLoader::Load(MDBlob *v)
        {
            v->offset = LoadInt(mdt_header().BlobIndexSize());
            raw_istream &is = file_->md_stream(PEFileReader::kBlobStream);
            is.seek(v->offset);
            
            v->size = ReadBlobOrUserStringSize(is);
            v->data_offset = is.tellg();
            v->file = file_;
            
            if (v->offset == 726 && v->size == 32854)
            {
                std::cout << "MDLoader::Load " << v << "\n";
            }
            return *this;
        }
        
        MDLoader & MDLoader::LoadUserString(MDString *str, const MDToken &token)
        {
            str->offset = token.idx();
            raw_istream & is = file_->md_stream(PEFileReader::kUserStringStream);
            is.seek(str->offset);
            
            size_t size = ReadBlobOrUserStringSize(is);
            str->string = std::u16string(reinterpret_cast<const char16_t*>(is.pos()), size / 2);
            
            return *this;
        }
        
        MDLoader & MDLoader::Load(MDTokenBase *v)
        {
            v->Load(*this);
            return *this;
        }
        
        uint32_t MDLoader::ReadBlobOrUserStringSize(raw_istream & is)
        {
            unsigned char b1, b2, b3, b4;
            is >> b1;
            if (b1 < 128)
            {
                return b1;
            }
            else if ((b1 & 0xc0) == 0x80)
            {
                is >> b2;
                return ((b1 & ~0xc0) << 8) | b2;
            }
            else if ((b1 & 0xe0) == 0xc0)
            {
                is >> b2 >> b3 >> b4;
                return ((b1 & ~0xe0) << 24) | (b2 << 16) | (b3 << 8) | b4;
            }
            else
            {
                assert(0 && "Incorrect data");
            }
        }
        
        uint32_t MDLoader::LoadInt(unsigned size)
        {
            if (size == 2)
            {
                uint16_t i;
                stream() >> i;
                return i;
            }
            else
            {
                uint32_t dummy;
                stream() >> dummy;
                return dummy;
            }
        }
        
        const MetadataTableHeader &MDLoader::mdt_header() const
        { return file_->mdt_header(); }
        
        MDTableBase *MDLoader::table(unsigned id) const
        { return file_->table(id); }
    }
}

