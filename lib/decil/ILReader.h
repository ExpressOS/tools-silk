//
//  ILReader.h
//  silk
//
//  Created by Haohui Mai on 12/7/12.
//  Copyright (c) 2012 Haohui Mai. All rights reserved.
//

#ifndef SILK_LIB_DECIL_ILREADER_H_
#define SILK_LIB_DECIL_ILREADER_H_

#include "silk/decil/ObjectModel.h"

namespace silk
{
    namespace decil
    {
        class PEFileToObjectModel;
        class MethodDefinition;
        class MDToken;
        class ILReader
        {
        public:
            ILReader(PEFileToObjectModel *model, MethodDefinition *method);
            std::vector<IOperation*> & GetInstructions()
            { return instructions_; }
            void ReadIL();
            
        private:
            void LoadLocalSignature();
            void PopulateCILInstructions();
            IMetadata *GetLocal(int idx);
            IMetadata *GetParameter(int idx);
            IMetadata *GetMethod(const MDToken *tok);
            IMetadata *GetType(const MDToken *tok);
            IMetadata *GetField(const MDToken *tok);
            IMetadata *GetRuntimeHandleFromToken(const MDToken *tok);
            std::u16string GetUserStringForToken(const MDToken *tok);
            Opcode GetOpcode(raw_istream &is);
            
            PEFileToObjectModel *model_;
            MethodDefinition *method_;
            std::vector<IOperation*> instructions_;
        };
    }
}

#endif

