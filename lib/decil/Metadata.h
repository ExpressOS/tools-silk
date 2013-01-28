//
//  Metadata.h
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_METADATA_H_
#define SILK_LIB_METADATA_H_

#include <vector>
#include <limits>
#include <cassert>
#include <cstdint>

namespace silk
{
    namespace decil
    {
        /* Type of metadata. Table indices */
        enum
        {
            kModuleDefinition = 0x00,
            kTypeReference = 0x01,
            kTypeDefinition = 0x02,
            kField = 0x04,
            kMethodDefinition = 0x06,
            kParameter = 0x08,
            kInterfaceImplementation = 0x09,
            kMemberReference = 0x0A,
            kConstant = 0x0B,
            kCustomAttribute = 0x0C,
            kFieldMarshal = 0x0D,
            kDeclSecurity = 0x0E,
            kClassLayout = 0x0F,
            kFieldLayout = 0x10,
            kStandAloneSignature = 0x11,
            kEventMap = 0x12,
            kEvent = 0x14,
            kPropertyMap = 0x15,
            kProperty = 0x17,
            kMethodSemantics = 0x18,
            kMethodImplementation = 0x19,
            kModuleReference = 0x1A,
            kTypeSpecification = 0x1B,
            kImplementationMap = 0x1C,
            kFieldRVA = 0x1D,
            kAssemblyDefinition = 0x20,
            kAssemblyProcessor = 0x21,
            kAssemblyOperatingSystem = 0x22,
            kAssemblyReference = 0x23,
            kAssemblyReferenceProcessor = 0x24,
            kAssemblyReferenceOperatingSystem = 0x25,
            kFile = 0x26,
            kExportedType = 0x27,
            kManifestResource = 0x28,
            kNestedClass = 0x29,
            kGenericParameter = 0x2A,
            kMethodSpecification = 0x2B,
            kGenericParameterConstraint = 0x2C,
            kMetadataTableCount,
            kInvalidTableReference = 0xffff,
        };
        
        struct MetadataTableHeader
        {
            /* Heap flags */
            enum
            {
                kStrings32Bit = 0x01,
                kGUIDs32Bit   = 0x02,
                kBlobs32Bit   = 0x04,
            };
            
            uint32_t Reserved;
            uint8_t MajorVersion;
            uint8_t MinorVersion;
            uint8_t HeapOffsetSizes;
            uint8_t MoreReserved;
            uint64_t PresentTables;
            uint64_t SortedTables;
            
            unsigned StringIndexSize() const
            { return HeapOffsetSizes & kStrings32Bit ? 4 : 2; }
            unsigned GUIDIndexSize() const
            { return HeapOffsetSizes & kGUIDs32Bit ? 4 : 2; }
            unsigned BlobIndexSize() const
            { return HeapOffsetSizes & kBlobs32Bit ? 4 : 2; }
        };
        
        class MDLoader;
        
        class MDRowBase {
        public:
            unsigned index() const             { return row_index_; }
            void set_row_index(unsigned value) { row_index_ = value; }
            
            virtual void Load(MDLoader &loader) = 0;
            MDRowBase();
            virtual ~MDRowBase();
            
        protected:
            unsigned row_index_;
        };
        
        class MDTableBase {
        public:
            MDTableBase();
            unsigned size() const    { return rows_; }
            void set_row(unsigned r) { rows_ = r; }
            unsigned sorted() const  { return sorted_; }
            void set_sorted(bool v)  { sorted_ = v; }
            bool is_full_id() const  { return size() >= std::numeric_limits<uint16_t>::max(); }
            
            virtual void Load(MDLoader &loader) = 0;
            virtual ~MDTableBase();
        private:
            unsigned rows_;
            bool sorted_;
        };
        
        template<class EntryType>
        class MDTable : public MDTableBase {
        public:
            // All indices are started from 1, so here we
            // reserve another spot here.
            virtual void Load(MDLoader &loader)
            {
                entries_.resize(size() + 1);
                for (unsigned i = 1; i <= size(); ++i)
                {
                    entries_[i].set_row_index(i);
                    entries_[i].Load(loader);
                }
            }
            
            typedef typename std::vector<EntryType>::iterator iterator;
            typedef typename std::vector<EntryType>::const_iterator const_iterator;

            iterator begin()
            { return entries_.size() > 1 ? ++entries_.begin() : entries_.end(); }
            iterator end()
            { return entries_.end(); }
            
            const_iterator begin() const
            { return entries_.size() > 1 ? ++entries_.begin() : entries_.end(); }
            const_iterator end() const
            { return entries_.end(); }
            
            const EntryType &get(std::size_t index) const
            {
                assert (index > 0);
                return entries_.at(index);
            }
            
            EntryType &get(std::size_t index)
            {
                assert (index > 0);
                return entries_.at(index);
            }
            
        private:
            std::vector<EntryType> entries_;
        };
    }
}
#endif

