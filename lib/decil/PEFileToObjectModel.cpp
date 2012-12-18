//
//  PEFileToObjectModel.cpp
//  silk
//
//  Created by Haohui Mai on 11/26/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#include "PEFileToObjectModel.h"
#include "PEFileReader.h"
#include "BinaryObjectModel.h"
#include "MetadataTable.h"
#include "SignatureConverter.h"
#include "Host.h"
#include "ILReader.h"

#include "silk/Support/Util.h"

#include <iostream>

namespace silk
{
    namespace decil
    {
        PEFileToObjectModel::PEFileToObjectModel(Host *host, PEFileReader *file)
        : host_(host)
        , file_(file)
        {
            assert(file->is_assembly());
            auto id = file->GetAssemblyId();
            containing_assembly_ = new Assembly(this, id);

            fields_.resize(file_->GetMDTable<FieldDef>().size() + 1);
            methods_.resize(file_->GetMDTable<MethodDef>().size() + 1);
            type_refs_.resize(file_->GetMDTable<TypeRef>().size() + 1);
            member_refs_.resize(file_->GetMDTable<MemberReference>().size() + 1);

            LoadAssemblyReferences();
            LoadTypeDefinitions();
            
//            this.LoadModuleReferences();
//            this.RootModuleNamespace = new RootNamespace(this);
//            this.NamespaceINameHashtable = new Hashtable<Namespace>();
//            this.LoadNamespaces();
//            this.NamespaceReferenceINameHashtable = new DoubleHashtable<NamespaceReference>();
//            this.NamespaceTypeTokenTable = new DoubleHashtable(peFileReader.TypeDefTable.NumberOfRows + peFileReader.ExportedTypeTable.NumberOfRows);
//            this.NestedTypeTokenTable = new DoubleHashtable(peFileReader.NestedClassTable.NumberOfRows + peFileReader.ExportedTypeTable.NumberOfRows);
//            this.PreLoadTypeDefTableLookup();
//            this.ModuleTypeDefArray = new TypeBase/*?*/[peFileReader.TypeDefTable.NumberOfRows + 1];
//            this.ModuleTypeDefLoadState = new LoadState[peFileReader.TypeDefTable.NumberOfRows + 1];
//            this.RedirectedTypeDefArray = new INamedTypeReference/*?*/[peFileReader.TypeDefTable.NumberOfRows + 1];
//            this.ModuleTypeDefLoadState[0] = LoadState.Loaded;
//            
//            this.ExportedTypeArray = new ExportedTypeAliasBase/*?*/[peFileReader.ExportedTypeTable.NumberOfRows + 1];
//            this.ExportedTypeLoadState = new LoadState[peFileReader.ExportedTypeTable.NumberOfRows + 1];
//            this.ExportedTypeLoadState[0] = LoadState.Loaded;
//            
//            this.ModuleGenericParamArray = new GenericParameter[peFileReader.GenericParamTable.NumberOfRows + 1];
//            if (peFileReader.MethodSpecTable.NumberOfRows > 0) {
//                this.ModuleMethodSpecHashtable = new DoubleHashtable<IGenericMethodInstanceReference>(peFileReader.MethodSpecTable.NumberOfRows + 1);
//            }
//            
//            this.ModuleTypeRefReferenceArray = new INamedTypeReference[peFileReader.TypeRefTable.NumberOfRows + 1];
//            this.ModuleTypeRefReferenceLoadState = new LoadState[peFileReader.TypeRefTable.NumberOfRows + 1];
//            this.ModuleTypeRefReferenceLoadState[0] = LoadState.Loaded;
//            if (peFileReader.TypeSpecTable.NumberOfRows > 0) {
//                this.ModuleTypeSpecHashtable = new DoubleHashtable<TypeSpecReference>(peFileReader.TypeSpecTable.NumberOfRows + 1);
//            }
//            
//            this.ModuleFieldArray = new FieldDefinition[peFileReader.FieldTable.NumberOfRows + 1];
//            this.ModuleMethodArray = new IMethodDefinition[peFileReader.MethodTable.NumberOfRows + 1];
//            this.ModuleEventArray = new EventDefinition[peFileReader.EventTable.NumberOfRows + 1];
//            this.ModulePropertyArray = new PropertyDefinition[peFileReader.PropertyTable.NumberOfRows + 1];
//            
//            this.ModuleMemberReferenceArray = new MemberReference/*?*/[peFileReader.MemberRefTable.NumberOfRows + 1];
//            this.UnspecializedMemberReferenceArray = new MemberReference/*?*/[peFileReader.MemberRefTable.NumberOfRows + 1];
//            this.SpecializedFieldHashtable = new DoubleHashtable<ISpecializedFieldReference>();
//            this.SpecializedMethodHashtable = new DoubleHashtable<ISpecializedMethodReference>();
//            
//            this.CustomAttributeArray = new ICustomAttribute/*?*/[peFileReader.CustomAttributeTable.NumberOfRows + 1];
//            
//            this.DeclSecurityArray = new ISecurityAttribute/*?*/[peFileReader.DeclSecurityTable.NumberOfRows + 1];

        }
        
        std::u16string PEFileToObjectModel::MangleParams(const std::vector<ITypeReference *> &params)
        {
            std::u16string res;
            for (auto e : params)
            {
                res += u"." + e->resolved_type()->name();
            }
            return res;
        }
        
        void PEFileToObjectModel::LoadAssemblyReferences()
        {
            auto &tbl = file_->GetMDTable<AssemblyRef>();
            
            assembly_references_.reserve(tbl.size() + 1);
            assembly_references_.push_back(nullptr);
            for (auto &e : tbl)
            {
                auto id = AssemblyIdentity(e.Name, e.Culture, e.MajorVersion, e.MinorVersion,
                                           e.RevisionNumber, e.BuildNumber);
                auto v = new AssemblyReference(host_, id);
                assembly_references_.push_back(v);
            }
        }
        
        void PEFileToObjectModel::LoadTypeDefinitions()
        {
            auto &tbl = file_->GetMDTable<TypeDefinition>();
            named_typedefs_.resize(tbl.size() + 1);
            auto end = tbl.size() + 1;
            // Making sure the iterator is valid
            for (size_t i = 1; i < end; ++i)
            {
                GetTypeDefinitionAtRow(i);
            }
            
            for (size_t i = 1; i < end; ++i)
            {
                auto t = GetTypeDefinitionAtRow(i);
                for (auto it = t->method_begin(), end = t->method_end(); it != end; ++it)
                {
                    auto meth = static_cast<MethodDefinition*>(*it);
                    LoadMethodDefinition(meth, meth->method_def());
                    meth->LoadInstructions();
                }
            }
        }
        
        unsigned PEFileToObjectModel::FindParentOfNestedClassByRowId(size_t idx)
        {
            auto &tbl = file_->GetMDTable<NestedClass>();
            for (auto &e : tbl)
            {
                if (e.NestedClass == idx)
                    return e.EnclosingClass;
            }
            return 0;
        }
        
        void PEFileToObjectModel::LoadTypeBaseMembers(TypeBase *type)
        {
            auto def = type->type_def_;
            auto &field_tbl = file_->GetMDTable<FieldDef>();
            auto &method_tbl = file_->GetMDTable<MethodDef>();
            auto &typedef_tbl = file_->GetMDTable<TypeDefinition>();
            
            auto fields = &type->fields_;
            assert (fields->empty());
            
            int field_end = field_tbl.size() + 1;
            int method_end = method_tbl.size() + 1;
            if (def->index() < typedef_tbl.size())
            {
                const TypeDefinition &next = typedef_tbl.get(def->index() + 1);
                field_end = next.FieldList;
                method_end = next.MethodList;
            }
            
            for (int i = def->FieldList; i < field_end; ++i)
            {
                auto f = CreateFieldDefinitionAtRow(i, type);
                assert (f && f->field_type());
                fields->push_back(f);
            }
            
            auto methods = &type->methods_;
            assert (methods->empty());
            
            for (int i = def->MethodList; i < method_end; ++i)
                methods->push_back(CreateMethodDefinitionAtRow(i, type));
        }
        
        void PEFileToObjectModel::EnsureParentTypeLoaded(int idx, std::function<int(const decil::TypeDefinition&)> idx_func)
        {
            auto &tbl = file_->GetMDTable<TypeDefinition>();

            size_t i;
            for (i = 1; i < tbl.size() + 1; ++i)
            {
                auto &e = tbl.get(i);
                
                if (idx_func(e) > idx)
                    break;
            }
            GetTypeDefinitionAtRow(i - 1);
        }

        
        AssemblyReference *PEFileToObjectModel::GetAssemblyReferenceAtRow(size_t idx)
        {
            auto &tbl = file_->GetMDTable<AssemblyRef>();
            return idx <= 0 || idx > tbl.size() ? nullptr : assembly_references_[idx];
        }
        
        INamedTypeDefinition *PEFileToObjectModel::GetTypeDefinitionAtRow(size_t idx)
        {
            auto &tbl = file_->GetMDTable<TypeDefinition>();
            if (idx <= 0 || idx > tbl.size())
                return nullptr;
            
            if (named_typedefs_[idx] != nullptr)
                return named_typedefs_[idx];
            
            TypeBase *ret = nullptr;
            
            auto &e = tbl.get(idx);
            if (e.is_nested())
            {
                auto parent_type = FindParentOfNestedClassByRowId(e.index());
                ret = new NonGenericNestedType(this, &e, parent_type);
            }
            else
            {
                auto tc = GetTypeCodeForTypeDefAtRow(idx);
                if (tc == INamedTypeDefinition::TypeCode::NotPrimitive)
                {
                    ret = new NonGenericNamespaceType(this, &e);
                }
                else
                {
                    ret = new NonGenericNamespaceTypeWithPrimitiveType(this, &e, tc);
                }
            }
            named_typedefs_[idx] = ret;
            assert (ret);
            LoadTypeBaseMembers(ret);
            
//            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
//            std::string s = utf16conv.to_bytes(ret->name());
//            std::cout << s << ":" << e.index() << "\n";
            return ret;
        }
        
        FieldDefinition *PEFileToObjectModel::GetFieldDefAtRow(size_t idx)
        {
            auto &tbl = file_->GetMDTable<FieldDef>();
            if (idx <= 0 || idx > tbl.size())
                return nullptr;
            
            // Load the type lazily
            if (!fields_[idx])
                EnsureParentTypeLoaded(idx, [](const TypeDefinition &t) { return t.FieldList; });

            return fields_[idx];
        }
        
        FieldDefinition *PEFileToObjectModel::CreateFieldDefinitionAtRow(size_t idx, TypeBase *containing_type)
        {
            auto &tbl = file_->GetMDTable<FieldDef>();
            assert (0 < idx && idx <= tbl.size() && !fields_[idx]);
            
            auto ret = new FieldDefinition(this, &tbl.get(idx), containing_type);
            fields_[idx] = ret;
            return ret;            
        }
        
        MethodDefinition *PEFileToObjectModel::GetMethodDefAtRow(size_t idx)
        {
            auto &tbl = file_->GetMDTable<MethodDef>();
            if (idx <= 0 || idx > tbl.size())
                return nullptr;
            
            // Load the type lazily
            if (!methods_[idx])
                EnsureParentTypeLoaded(idx, [](const TypeDefinition &t) { return t.MethodList; });

            auto ret = methods_[idx];
            assert(ret);
            return ret;
        }
        
        MethodDefinition *PEFileToObjectModel::CreateMethodDefinitionAtRow(size_t idx, TypeBase *containing_type)
        {
            auto &tbl = file_->GetMDTable<MethodDef>();
            assert (0 < idx && idx <= tbl.size() && !methods_[idx]);

//            if (tbl.size() == 691)
//                std::cout << "&tbl.get(" << idx << "):" << &tbl.get(idx) << "\n";
            
            auto ret = new MethodDefinition(this, &tbl.get(idx), containing_type);
            methods_[idx] = ret;
            assert (ret->method_def()->index() == idx);
            return ret;
        }
        
        ITypeReference *PEFileToObjectModel::GetTypeReferenceAtRow(size_t idx)
        {
            auto &tbl = file_->GetMDTable<TypeRef>();
            if (idx <= 0 || idx > tbl.size())
                return nullptr;
            
            if (type_refs_[idx])
                return type_refs_[idx];
            
            auto &e = tbl.get(idx);
            auto tok = e.ResolutionScope;
            ITypeReference *ret = NULL;
            switch (tok.id())
            {
                case kAssemblyReference:
                    ret = new TypeReference(GetAssemblyReferenceAtRow(tok.idx()), e.TypeName, e.TypeNamespace);
                    break;
                    
                default:
                    assert (0 && "Unimplemented");
                    break;
            }
            type_refs_[idx] = ret;
            return ret;
        }
        
        ITypeReference *PEFileToObjectModel::GetTypeReferenceForToken(const MDTokenBase *tok)
        {
            auto type = tok->id();
            auto idx = tok->idx();
            switch (type)
            {
                case kTypeDefinition:
                    return GetTypeDefinitionAtRow(idx);
                    
                case kTypeReference:
                    return GetTypeReferenceAtRow(idx);
                    
                case kTypeSpecification:
                    assert (0 && "Unimplemented");
                    return nullptr;
            }
            return nullptr;
        }
        
        IMethodReference *PEFileToObjectModel::GetMethodReferenceForToken(const MDTokenBase *tok)
        {
            auto type = tok->id();
            auto idx = tok->idx();
            switch (type)
            {
                case kMethodDefinition:
                    return GetMethodDefAtRow(idx);
                    
                case kMemberReference:
                    return dynamic_cast<IMethodReference*>(GetMemberReferenceAtRow(idx));
                    
                default:
                    assert (0 && "Unimplemented");
                    return nullptr;
            }
            return nullptr;
        }
        
        IFieldReference *PEFileToObjectModel::GetFieldReferenceForToken(const MDTokenBase *tok)
        {
            auto type = tok->id();
            auto idx = tok->idx();
            switch (type)
            {
                case kField:
                    return GetFieldDefAtRow(idx);
                    
                case kMemberReference:
                    return dynamic_cast<IFieldReference*>(GetMemberReferenceAtRow(idx));
                    
                default:
                    assert (0 && "Unimplemented");
                    return nullptr;
            }
            return nullptr;
        }
        
        ITypeMemberReference* PEFileToObjectModel::GetMemberReferenceAtRow(size_t idx)
        {
            auto &tbl = file_->GetMDTable<MemberReference>();
            if (idx <= 0 || idx > tbl.size())
                return nullptr;
            
            if (member_refs_[idx])
                return member_refs_[idx];
            
            auto &e = tbl.get(idx);
            auto type_id = e.Class.id();
            
            ITypeMemberReference *ret = nullptr;
            
            if (type_id == kTypeDefinition || type_id == kTypeReference)
            {
                ITypeReference *parent = nullptr;
                if (type_id == kTypeDefinition)
                {
                    parent = GetTypeDefinitionAtRow(e.Class.idx());
                }
                else
                {
                    parent = GetTypeReferenceAtRow(e.Class.idx());
                }
                
                auto is = e.Signature.to_istream();
                uint8_t first_byte = is.peek<uint8_t>();
                if (SignatureConverter::IsFieldSignature(first_byte))
                {
                    ret = new FieldReference(this, &e, parent);
                }
                else if (SignatureConverter::IsMethodSignature(first_byte))
                {
                    ret = new MethodReference(this, &e, parent);
                }
                else
                {
                    assert (0 && "Malformed PE file");
                }
            }
            else if (type_id == kMethodDefinition)
            {
                ret = GetMethodDefAtRow(e.Class.idx());
            }
            else
            {
                assert (0 && "unimplemented");
            }

            member_refs_[idx] = ret;
            return ret;
        }
        
        INamedTypeDefinition::TypeCode PEFileToObjectModel::GetTypeCodeForTypeDefAtRow(size_t idx) const
        {
            static struct
            {
                std::u16string ns;
                std::u16string name;
                INamedTypeDefinition::TypeCode code;
            } core_types[] =
            {
                {u"System", u"Void", INamedTypeDefinition::TypeCode::Void},
                {u"System", u"Boolean", INamedTypeDefinition::TypeCode::Boolean},
                {u"System", u"Char", INamedTypeDefinition::TypeCode::Char},
                {u"System", u"SByte", INamedTypeDefinition::TypeCode::Int8},
                {u"System", u"Byte", INamedTypeDefinition::TypeCode::UInt8},
                {u"System", u"Int16", INamedTypeDefinition::TypeCode::Int16},
                {u"System", u"UInt16", INamedTypeDefinition::TypeCode::UInt16},
                {u"System", u"Int32", INamedTypeDefinition::TypeCode::Int32},
                {u"System", u"UInt32", INamedTypeDefinition::TypeCode::UInt32},
                {u"System", u"Int64", INamedTypeDefinition::TypeCode::Int64},
                {u"System", u"UInt64", INamedTypeDefinition::TypeCode::UInt64},
                {u"System", u"Single", INamedTypeDefinition::TypeCode::Single},
                {u"System", u"Double", INamedTypeDefinition::TypeCode::Double},
                {u"System", u"IntPtr", INamedTypeDefinition::TypeCode::IntPtr},
                {u"System", u"UIntPtr", INamedTypeDefinition::TypeCode::UIntPtr},
            };

            auto &tbl = file_->GetMDTable<TypeDefinition>();
            assert (idx > 0 && idx < tbl.size() + 1);
            
            auto corelib_id = host_->platform_type()->corelib_id();
            if (corelib_id.name() != containing_assembly_->identity().name())
            {
                return INamedTypeDefinition::TypeCode::NotPrimitive;
            }
            
            auto &e = tbl.get(idx);
            auto ns = (std::u16string)e.TypeNamespace;
            auto n = (std::u16string)e.TypeName;
            
            for (auto &c : core_types)
            {
                if (ns == c.ns && n == c.name)
                    return c.code;
            }
            
            return INamedTypeDefinition::TypeCode::NotPrimitive;
        }
        
        INamedTypeDefinition *PEFileToObjectModel::ResolveNamespaceTypeDefinition(const std::u16string &namespace_name, const std::u16string &name)
        {
//            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
//            std::cout << "Resolve:" << utf16conv.to_bytes(namespace_name + u"." + name) << "\n";
//            
            auto &tbl = file_->GetMDTable<TypeDefinition>();
            for (auto &e : tbl)
            {
//                std::cout << utf16conv.to_bytes((std::u16string)e.TypeNamespace + u"." + (std::u16string)e.TypeName) << "\n";

                if ((std::u16string)e.TypeNamespace == namespace_name
                    && (std::u16string)e.TypeName == name)
                    return GetTypeDefinitionAtRow(e.index());
            }
            return nullptr;
        }
        
        void PEFileToObjectModel::LoadMethodDefinition(MethodDefinition *method, const MethodDef *def)
        {
            auto signature = def->Signature.to_istream();
            MethodSignatureConverter converter(this, signature);
            method->signature_flags_ = converter.flags();
            method->return_type_ = converter.return_type();
            
            auto &method_tbl = file_->GetMDTable<MethodDef>();
            auto &param_tbl = file_->GetMDTable<ParamDef>();
            auto param_end = param_tbl.size() + 1;

            if (def->index() < method_tbl.size())
            {
                auto &next = method_tbl.get(def->index() + 1);
                param_end = next.ParamList;
            }
            
            auto &params = converter.param_type();
            
            auto has_implicit_this = method->has_this() && !method->explicit_this();
            if (has_implicit_this)
            {
                auto this_param = new ParameterDefinition(this, u"this", method->containing_type());
                method->params_.push_back(this_param);
            }

            for (unsigned i = def->ParamList; i < param_end; ++i)
            {
                auto &p = param_tbl.get(i);
                
                // Don't push the return type into the parameter list
                if (p.Sequence != 0)
                {
                    auto param_def = new ParameterDefinition(this, p.Name, params.at(p.Sequence - 1));
                    method->params_.push_back(param_def);
                }
            }
            
            auto method_il = file_->GetMethodIL(def->index());
            method->method_il_ = method_il;
            if (method_il)
            {
                // Declare local variables when they exist
                if (method_il->LocalSignatureToken.id() == kStandAloneSignature)
                {
                    auto &tbl = file_->GetMDTable<StandAloneSignature>();
                    auto &e = tbl.get(method_il->LocalSignatureToken.idx());
                    LocalVariableSignatureConverter converter(this, e.Signature.to_istream());
                    method->locals_.swap(converter.locals());
                }
            }
        }
        
        raw_istream PEFileToObjectModel::GetFieldMapping(const FieldDef *field_def)
        {
            auto field_idx = field_def->index();
            auto &field_rva_tbl = file_->GetMDTable<FieldRVA>();
            auto it = std::find_if (field_rva_tbl.begin(), field_rva_tbl.end(),
                                    [=](const FieldRVA &e) { return e.Field == field_idx; });
            
            if (it == field_rva_tbl.end())
            {
                return raw_istream();
            }
            else
            {
                return file_->RVAToIStream(it->RVA);
            }
        }
        
        void PEFileToObjectModel::GetClassLayout(const TypeDefinition *type_def,
                                                 uint32_t *packing_size, uint32_t * class_size)
        {
            unsigned idx = type_def->index();
            auto &tbl = file_->GetMDTable<ClassLayout>();
            auto it = std::find_if (tbl.begin(), tbl.end(),
                                    [=](const ClassLayout &e) { return e.Parent == idx; });
            
            if (it != tbl.end())
            {
                *packing_size = it->PackingSize;
                *class_size = it->ClassSize;
            }
            else
            {
                *packing_size = 0;
                *class_size = 0;
            }

        }
    }
}
