//
//  PEFileReader.cpp
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "PEFileReader.h"
#include "MDLoader.h"
#include "MetadataTable.h"

namespace silk
{
    using namespace llvm;
    
    namespace decil
    {
        static const int   kDosSignature = 0x5A4D; // MZ
        static const int   kPESignatureOffsetLocation = 0x3C;
        static const int   kPESignature = 0x00004550;    // PE00
        static const int   kBasicPEHeaderSize = kPESignatureOffsetLocation;
        static const short kPEMagic32 = 0x10b;
        static const short kPEMagic64 = 0x20b;
        static const int   kSizeofOptionalHeaderStandardFields32 = 28;
        static const int   kSizeofOptionalHeaderNTAdditionalFields32 = 68;
        
        static uint64_t GetBitMask(unsigned index)
        {
            return 1ULL << index;
        }
        
        PEFileReader::PEFileReader(IHost *host, OwningPtr<MemoryBuffer> &file)
        : host_(host)
        , file_(file.take())
        , state_(ReaderState::kInitialized)
        {
            if (ReadPEFileLevelData() || ReadCORModuleLevelData() || ReadMetadataLevelData())
                return;
        }
        
        int PEFileReader::ReadPEFileLevelData()
        {
            raw_istream is(file_.get());
            ErrorHandler &eh = host_->error_handler();
            
            ReadPEHeader(is, eh);
            if (eh.has_error())
                return -1;
            
            ReadCOFFOptionalHeader(is, eh);
            if (eh.has_error())
                return -1;
            
            is.read(optional_pe_directory_entries_);
            
            ReadPESectionHeaders(is, eh);
            if (eh.has_error())
                return -1;
            
            state_ = ReaderState::kPEFile;
            return 0;
        }
        
        int PEFileReader::ReadCORModuleLevelData()
        {
            const PEDirectoryEntry &clr_header_entry = optional_pe_directory_entries_[kCLRRuntimeHeader];
            ErrorHandler &eh = host_->error_handler();
            
            raw_istream hs = DirectoryToIStream(clr_header_entry, eh);
            if (eh.has_error())
                return -1;
            
            hs.read(cor20_header_);
            
            raw_istream ms = DirectoryToIStream(cor20_header_.MetaDataDirectory, eh);
            if (eh.has_error())
                return -1;
            
            ReadMetadataHeader(ms, eh);
            if (eh.has_error())
                return -1;
            
            state_ = ReaderState::kCORModule;
            return 0;
        }
        
        int PEFileReader::ReadMetadataLevelData()
        {
            InitializeMetadataTables();
            raw_istream & is = md_streams_[kCompressedMetadataTableStream];
            ErrorHandler &eh = host_->error_handler();
            
            is.read(mdt_header_);
            
            for (unsigned i = 0; i < kMetadataTableCount; ++i) {
                if (!(GetBitMask(i) & mdt_header_.PresentTables))
                    continue;
                
                if (md_tables_.count(i) == 0) {
                    eh.Error("Unimplemented CIL table %d", i);
                    continue;
                }
                
                uint32_t row_count;
                is >> row_count;
                md_tables_[i]->set_row(row_count);
                md_tables_[i]->set_sorted(GetBitMask(i) & mdt_header_.SortedTables);
            }
            
            MDLoader loader(this, is, eh);
            for (auto &it : md_tables_)
            {
                it.second->Load(loader);
            }
            
            if (eh.has_error())
                return -1;
            
            state_ = ReaderState::kMetadata;
            return 0;
        }
        
        void PEFileReader::ReadPEHeader(raw_istream &is, ErrorHandler &eh)
        {
            unsigned short dos_signature;
            is >> dos_signature;
            
            if (dos_signature != kDosSignature)
            {
                eh.Error("PE/DOS signature mismatch.");
                return;
            }
            
            if(is.seek(kPESignatureOffsetLocation).fail())
            {
                eh.Error("PE file is too short.");
                return;
            }
            
            uint32_t pe_header_offset;
            is >> pe_header_offset;
            if (is.seek(pe_header_offset).fail())
            {
                eh.Error("PE file is too short.");
                return;
            }
            
            uint32_t nt_signature;
            is >> nt_signature;
            
            if (nt_signature != kPESignature)
            {
                eh.Error("PE/NT signature mismatch.");
                return;
            }
            
            if (is.remaining_bytes() <= sizeof(COFFHeader))
            {
                eh.Error("PE file is too short.");
                return;
            }
            
            is.read(coff_header_);
        }
        
        void PEFileReader::ReadCOFFOptionalHeader(raw_istream &is, ErrorHandler &eh)
        {
            unsigned short type;
            is >> type;
            
            if (type == kPEMagic64)
            {
                if (is.remaining_bytes() <= sizeof(COFFOptionalHeader))
                {
                    eh.Error("PE file is too short.");
                    return;
                }
                is.read(coff_optional_header_);
                
            }
            else if (type == kPEMagic32)
            {
                if (is.remaining_bytes() <=
                    kSizeofOptionalHeaderNTAdditionalFields32 + kSizeofOptionalHeaderNTAdditionalFields32)
                {
                    eh.Error("PE file is too short.");
                    return;
                }
                
                uint32_t tmp;
                
                COFFOptionalHeader & h = coff_optional_header_;
                
                is >> h.MajorLinkerVersion
                >> h.MinorLinkerVersion >> h.SizeOfCode
                >> h.SizeOfInitializedData
                >> h.SizeOfUninitializedData
                >> h.AddressOfEntryPoint
                >> h.BaseOfCode
                >> h.BaseOfData;
                
                
                is >> tmp;
                h.ImageBase = tmp;
                is >> h.SectionAlignment >> h.FileAlignment
                >> h.MajorOperatingSystemVersion
                >> h.MinorOperatingSystemVersion
                >> h.MajorImageVersion
                >> h.MinorImageVersion
                >> h.MajorSubsystemVersion
                >> h.MinorSubsystemVersion
                >> h.Win32VersionValue
                >> h.SizeOfImage
                >> h.SizeOfHeaders
                >> h.CheckSum
                >> h.Subsystem
                >> h.DllCharacteristics;
                
                is >> tmp;
                h.SizeOfStackReserve = tmp;
                is >> tmp;
                h.SizeOfStackCommit = tmp;
                is >> tmp;
                h.SizeOfHeapReserve = tmp;
                is >> tmp;
                h.SizeOfHeapCommit = tmp;
                is >> h.LoaderFlags >> h.NumberOfRvaAndSizes;
                
            }
            else
            {
                eh.Error("Invalid COFF optional header magic.");
            }
        }
        
        void PEFileReader::ReadPESectionHeaders(raw_istream &is, ErrorHandler &)
        {
            pe_section_headers_.resize(coff_header_.SectionNum);
            for (unsigned i = 0; i < coff_header_.SectionNum; ++i)
                is.read(pe_section_headers_[i]);
        }
        
        void PEFileReader::ReadMetadataHeader(raw_istream &is, ErrorHandler &)
        {
            const static struct StreamInfo
            {
                unsigned idx;
                const char *name;
            } si[] = {
                { kStringStream, "#Strings" },
                { kBlobStream, "#Blob" },
                { kGUIDStream, "#GUID" },
                { kUserStringStream, "#US" },
                { kCompressedMetadataTableStream, "#~" },
                { kUncompressedMetadataTableStream, "#-" },
            };
            
            struct MetadataHeader & h = metadata_header_;
            is >> h.Signature >> h.MajorVersion >> h.MinorVersion >> h.ExtraData >> h.VersionStringSize;
            
            h.VersionString = is.peek_utf8_null_terminated();
            is.skip(h.VersionStringSize);
            is >> h.Flags >> h.NumberOfStreams;
            
            for (size_t i = 0; i < h.NumberOfStreams; ++i)
            {
                MetadataStreamHeader hs;
                is >> hs.Offset >> hs.Size;
                hs.Name = is.read_ascii_null_terminated();
                is.align(4);
                
                for (auto &s : si)
                {
                    if (hs.Name == s.name)
                        md_streams_[s.idx] = raw_istream(is.start() + hs.Offset, hs.Size);
                }
            }
        }
        
        raw_istream PEFileReader::DirectoryToIStream(const PEDirectoryEntry &e, ErrorHandler &eh) const
        {
            for (auto &it : pe_section_headers_)
            {
                if (it.VirtualAddress <= e.RelativeVirtualAddress
                    && e.RelativeVirtualAddress < it.VirtualAddress + it.VirtualSize)
                {
                    // FIXME: Padding might not be zeroed
                    int offset = e.RelativeVirtualAddress - it.VirtualAddress;
                    return raw_istream(file_->getBufferStart() + it.PointerToRawData + offset, e.Size);
                }
            }
            
            eh.Error("Malformed metadata directory entry.");
            return raw_istream();
        }
        
        raw_istream PEFileReader::RVAToIStream(uint32_t rva) const
        {
            for (auto &e : pe_section_headers_)
            {
                if (e.VirtualAddress <= rva && rva <= e.VirtualAddress + e.VirtualSize)
                {
                    auto off = rva - e.VirtualAddress + e.PointerToRawData;
                    return raw_istream(file_->getBufferStart() + off, file_->getBufferSize() - off);
                }
                                       
            }
            return raw_istream();
        }
        
        raw_istream &PEFileReader::md_stream(int idx)
        {
            assert(idx < kMetadataStreamCount);
            return md_streams_[idx];
        }
        
        MDTableBase *PEFileReader::table(unsigned idx) const
        {
            auto it = md_tables_.find(idx);
            return it == md_tables_.end() ? NULL :it->second;
        }
        
        bool PEFileReader::is_assembly() const
        {
            return GetMDTable<AssemblyDefinition>().size() == 1;
        }
        
        AssemblyIdentity PEFileReader::GetAssemblyId() const
        {
            assert(is_assembly());
            auto r = GetMDTable<AssemblyDefinition>().get(1);
            return AssemblyIdentity(r.Name, r.Culture, r.MajorVersion, r.MinorVersion, r.RevisionNumber, r.BuildNumber);
        }
        
        MethodIL *PEFileReader::GetMethodIL(size_t idx) const
        {
            auto &tbl = GetMDTable<MethodDef>();
            if (idx <= 0 || idx > tbl.size())
                return nullptr;
            
            auto &row = tbl.get(idx);
            if (row.RVA == 0)
                return nullptr;
            
            raw_istream is = RVAToIStream(row.RVA);
            uint8_t b0, b1;
            is >> b0;
            
            if ((b0 & MethodIL::kFatFormat) == MethodIL::kTinyFormat)
            {
                auto size = b0 >> MethodIL::kILTinyFormatSizeShift;
                auto m = new MethodIL();
                m->LocalVariablesInited = true;
                m->MaxStack = 8;
                m->EncodedILMemoryBlock = raw_istream(is.pos(), size);
                return m;
            }
            
            assert ((b0 & MethodIL::kFatFormat) == MethodIL::kFatFormat);
            
            is >> b1;
            assert ((b1 >> MethodIL::kILFatFormatHeaderSizeShift) == MethodIL::kILFatFormatHeaderSize);
            
            auto m = new MethodIL();
            uint32_t code_size;

            m->LocalVariablesInited = (b0 & MethodIL::kInitLocals) == MethodIL::kInitLocals;
            is >> m->MaxStack >> code_size >> m->LocalSignatureToken;
            m->EncodedILMemoryBlock = raw_istream(is.pos(), code_size);
            
            return m;
        }
        
        void PEFileReader::InitializeMetadataTables()
        {
            CreateTable<ModuleDefinition>();
            CreateTable<TypeRef>();
            CreateTable<FieldDef>();
            CreateTable<ConstantDefinition>();
            CreateTable<CustomAttribute>();
            CreateTable<DeclSecurity>();
            CreateTable<ClassLayout>();
            CreateTable<StandAloneSignature>();
            CreateTable<PropertyMap>();
            CreateTable<PropertyDefinition>();
            CreateTable<MethodSemantics>();
            CreateTable<TypeDefinition>();
            CreateTable<MethodDef>();
            CreateTable<ParamDef>();
            CreateTable<InterfaceImplementation>();
            CreateTable<MemberReference>();
            CreateTable<MethodImplementation>();
            CreateTable<ModuleReference>();
            CreateTable<TypeSpecification>();
            CreateTable<ImplementationMap>();
            CreateTable<FieldRVA>();
            CreateTable<AssemblyDefinition>();
            CreateTable<AssemblyRef>();
            CreateTable<NestedClass>();
            CreateTable<GenericParameter>();
        }
    }
}
