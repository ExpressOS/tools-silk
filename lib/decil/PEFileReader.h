//
//  PEFileReader.h
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_PEFILEREADER_H_
#define SILK_LIB_DECIL_PEFILEREADER_H_

#include "Metadata.h"
#include "MDLoader.h"

#include "silk/decil/Units.h"
#include "silk/decil/IHost.h"
#include "silk/Support/raw_istream.h"

#include <llvm/ADT/OwningPtr.h>
#include <llvm/Support/MemoryBuffer.h>

#include <string>
#include <vector>
#include <map>

namespace silk
{
    namespace decil
    {
        struct COFFHeader
        {
            uint16_t Machine;
            uint16_t SectionNum;
            uint32_t Timestamp;
            uint32_t PointerToSymbolTable;
            uint32_t SymbolsNum;
            uint16_t OptionalHeaderSize;
            uint16_t Characteristics;
        };
        
        struct COFFOptionalHeader
        {
            uint8_t MajorLinkerVersion;
            uint8_t MinorLinkerVersion;
            uint32_t SizeOfCode;
            uint32_t SizeOfInitializedData;
            uint32_t SizeOfUninitializedData;
            uint32_t AddressOfEntryPoint;
            uint32_t BaseOfCode;
            union {
                uint32_t BaseOfData;
                uint64_t ImageBase;
            };
            uint32_t SectionAlignment;
            uint32_t FileAlignment;
            uint16_t MajorOperatingSystemVersion;
            uint16_t MinorOperatingSystemVersion;
            uint16_t MajorImageVersion;
            uint16_t MinorImageVersion;
            uint16_t MajorSubsystemVersion;
            uint16_t MinorSubsystemVersion;
            uint32_t Win32VersionValue;
            uint32_t SizeOfImage;
            uint32_t SizeOfHeaders;
            uint32_t CheckSum;
            uint16_t Subsystem;
            uint16_t DllCharacteristics;
            uint64_t SizeOfStackReserve;
            uint64_t SizeOfStackCommit;
            uint64_t SizeOfHeapReserve;
            uint64_t SizeOfHeapCommit;
            uint32_t LoaderFlags;
            uint32_t NumberOfRvaAndSizes;
        };
        
        struct PEDirectoryEntry
        {
            uint32_t RelativeVirtualAddress;
            uint32_t Size;
        };
        
        struct PESection
        {
            char Name[8];
            uint32_t VirtualSize;
            uint32_t VirtualAddress;
            uint32_t SizeOfRawData;
            uint32_t PointerToRawData;
            uint32_t PointerToRelocations;
            uint32_t PointerToLinenumbers;
            uint16_t NumberOfRelocations;
            uint16_t NumberOfLinenumbers;
            uint16_t Characteristics;
        };
        
        /* CLI specific headers */
        struct COR20Header
        {
            int CountBytes;
            uint16_t MajorRuntimeVersion;
            uint16_t MinorRuntimeVersion;
            PEDirectoryEntry MetaDataDirectory;
            uint32_t COR20Flags;
            uint32_t EntryPointTokenOrRVA;
            PEDirectoryEntry ResourcesDirectory;
            PEDirectoryEntry StrongNameSignatureDirectory;
            PEDirectoryEntry CodeManagerTableDirectory;
            PEDirectoryEntry VtableFixupsDirectory;
            PEDirectoryEntry ExportAddressTableJumpsDirectory;
            PEDirectoryEntry ManagedNativeHeaderDirectory;
        };
        
        struct MetadataHeader
        {
            uint32_t Signature;
            uint16_t MajorVersion;
            uint16_t MinorVersion;
            uint32_t ExtraData;
            uint32_t VersionStringSize;
            std::string VersionString;
            uint16_t Flags;
            uint16_t NumberOfStreams;
        };
        
        struct MetadataStreamHeader
        {
            uint32_t Offset;
            uint32_t Size;
            std::string Name;
        };
        
        /* ECMA-335 Partition II, 25.4 */
        class MethodIL
        {
        public:
            enum
            {
                kTinyFormat = 0x2,
                kFatFormat = 0x3,
                kILFormatMask = kFatFormat,
                kMoreSect = 0x8,
                kInitLocals = 0x10,
            };
            enum
            {
                kSectEHTable = 0x1,
                kSectOptILTable = 0x2,
                kSectFatFormat = 0x40,
                kSectMoreSect = 0x80,
            };
            
            static const int kILTinyFormatSizeShift = 2;
            static const int kILFatFormatHeaderSizeShift = 4;
            static const int kILFatFormatHeaderSize = 0x03;

            bool LocalVariablesInited;
            uint16_t MaxStack;
            MDToken LocalSignatureToken;
            raw_istream EncodedILMemoryBlock;
        };
        
        class PEFileReader
        {
        public:
            PEFileReader(IHost *host, llvm::OwningPtr<llvm::MemoryBuffer> &file);
            
            enum ReaderState
            {
                kInitialized,
                kPEFile,
                kCORModule,
                kMetadata,
            };
            ReaderState state() const { return state_; }
            /* Metadata streams */
            enum
            {
                kStringStream,
                kBlobStream,
                kGUIDStream,
                kUserStringStream,
                kCompressedMetadataTableStream,
                kUncompressedMetadataTableStream,
                kMetadataStreamCount,
            };
            
            raw_istream &md_stream(int idx);
            const MetadataTableHeader &mdt_header() const { return mdt_header_; }

            MDTableBase *table(unsigned id) const;
            template<class T>
            MDTable<T> &GetMDTable() const
            {
                return *(static_cast<MDTable<T>*>(table(T::id())));
            }
            
            bool is_assembly() const;
            AssemblyIdentity GetAssemblyId() const;
            MethodIL *GetMethodIL(size_t idx) const;
            raw_istream RVAToIStream(uint32_t rva) const;

        private:
            enum
            {
                kExportTable,
                kImportTable,
                kResourceTable,
                kExceptionTable,
                kCertificateTable,
                kBaseRelocationTable,
                kDebug,
                kArchitecture,
                kGlobalPtr,
                kTLSTable,
                kLoadConfigTable,
                kBoundImport,
                kIAT,
                kDelayImportDescriptor,
                kCLRRuntimeHeader,
                kReservedHeader,
                kOptionalHeaderDataDirectoryCount,
            };

            
            COFFHeader coff_header_;
            COFFOptionalHeader coff_optional_header_;
            PEDirectoryEntry optional_pe_directory_entries_[kOptionalHeaderDataDirectoryCount];
            std::vector<PESection> pe_section_headers_;
            
            COR20Header cor20_header_;
            MetadataHeader metadata_header_;

            IHost *host_;
            llvm::OwningPtr<llvm::MemoryBuffer> file_;
            ReaderState state_;
            
            int ReadPEFileLevelData();
            int ReadCORModuleLevelData();
            int ReadMetadataLevelData();
            
            void ReadPEHeader(raw_istream &is, ErrorHandler &eh);
            void ReadCOFFHeader(raw_istream &is, ErrorHandler &eh);
            void ReadCOFFOptionalHeader(raw_istream &is, ErrorHandler &eh);
            void ReadPESectionHeaders(raw_istream &is, ErrorHandler &eh);
            
            void ReadMetadataHeader(raw_istream &is, ErrorHandler &eh);
            
            template<class T>
            void CreateTable()
            {
                md_tables_.insert(std::make_pair(T::id(), new MDTable<T>()));
            }
            void InitializeMetadataTables();
            
            raw_istream DirectoryToIStream(const PEDirectoryEntry &e, ErrorHandler &) const;
            
            raw_istream md_streams_[kMetadataStreamCount];
            MetadataTableHeader mdt_header_;
            typedef std::map<unsigned, MDTableBase*> MDTables;
            MDTables md_tables_;
        };
    }
}
#endif
