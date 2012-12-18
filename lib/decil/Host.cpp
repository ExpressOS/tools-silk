//
//  Host.cpp
//  silk
//
//  Created by Haohui Mai on 11/25/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "Host.h"
#include "PEFileReader.h"
#include "MetadataTable.h"
#include "PEFileToObjectModel.h"
#include "PlatformTypes.h"

#include "silk/Support/Util.h"

#include <llvm/Support/FileSystem.h>

namespace silk
{
    namespace decil
    {
        using namespace llvm;
        
        IHost::~IHost() {}
        Host::~Host() {}

        IHost *CreateDefaultHost()
        {
            return new Host();
        }

        Host::Host()
        : platform_type_(new PlatformType(this))
        {}
        
        IPlatformType *Host::platform_type()
        { return platform_type_; }
        
        IAssembly *Host::LoadAssemblyFromPath(const std::string &path)
        {
            OwningPtr<MemoryBuffer> buffer;
            MemoryBuffer::getFileOrSTDIN(path, buffer);

            if (!buffer)
            {
                error_handler_.Error("Cannot find assembly `%s'.", path.c_str());
                return nullptr;
            }
            
            PEFileReader *pe_file = new PEFileReader(this, buffer);
            
            if (pe_file->state() < PEFileReader::ReaderState::kMetadata
                || !pe_file->is_assembly())
                return nullptr;
            
            auto id = pe_file->GetAssemblyId();
            auto lookup_assembly = LookupAssembly(id);
            if (lookup_assembly)
                return lookup_assembly;
            
            auto pe_model = new PEFileToObjectModel(this, pe_file);
            auto assembly = pe_model->containing_assembly();
            assemblies_[id] = assembly;
            loaded_assemblies_.push_back(assembly);
            // object_model->
            //            PEFileToObjectModel peFileToObjectModel = new PEFileToObjectModel(this, peFileReader, moduleIdentity, null, this.metadataReaderHost.PointerSize);
            //            this.LoadedModule(peFileToObjectModel.Module);
            //            Assembly/*?*/ assembly = peFileToObjectModel.Module as Assembly;
            //            if (assembly != null) {
            //                this.OpenMemberModules(binaryDocument, assembly);
            //            }
            //            return peFileToObjectModel.Module;
            //
            //printf("state: %d\n", pe_file->state());
            return assembly;

        }
        IAssembly *Host::LoadAssembly(const std::string &file)
        {
            for (auto &path : class_paths_)
            {
                std::string full_path_name = path + "/" + file;
                if (sys::fs::exists(full_path_name))
                    return LoadAssemblyFromPath(full_path_name);
            }
            return nullptr;
        }
        
        IAssembly *Host::LoadAssembly(const AssemblyIdentity &id)
        {
            std::string u8name = ToUTF8String(id.name());
            for (auto &path : class_paths_)
            {
                std::string full_path_name = path + "/" + u8name + ".dll";
                if (sys::fs::exists(full_path_name))
                    return LoadAssemblyFromPath(full_path_name);
            }
            return nullptr;
        }
        
        IAssembly *Host::get_assembly(int idx)
        { return loaded_assemblies_.at(idx); }
        
        size_t Host::assembly_size() const
        { return loaded_assemblies_.size(); }
        
        void Host::AddClassPath(const std::string &path)
        {
            class_paths_.push_back(path);
        }
        
        IAssembly *Host::LookupAssembly(const AssemblyIdentity &id) const
        {
            auto it = assemblies_.find(id);
            return it == assemblies_.end() ? nullptr : it->second;
        }
    }
}

