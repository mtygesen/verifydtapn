/* 
 * File:   CoveredMarkingVisitor.h
 * Author: Peter G. Jensen
 *
 * Created on 17 June 2015, 00:04
 */

#ifndef COVEREDMARKINGVISITOR_H
#define	COVEREDMARKINGVISITOR_H

#include "binarywrapper.h"
#include <assert.h>
#include <limits>
#include <vector>
#include "ptrie.h"
#include "visitor.h"
#include "MarkingEncoder.h"
#include "NonStrictMarkingBase.hpp"
#include "NonStrictMarking.hpp"


using namespace ptrie;
namespace VerifyTAPN {
namespace DiscreteVerification {
    
    class CoveredMarkingVisitor
    : public visitor_t<MetaData*>
    {
        typedef binarywrapper_t<MetaData*> encoding_t;
        private:
            MarkingEncoder<MetaData*, NonStrictMarking>& encoder;
            NonStrictMarking* target;
            encoding_t scratchpad;
            ptriepointer_t<MetaData*> match;
            bool _found;
            
        private:
            bool target_contains_token(uint placeage, uint count);
            
        public:
            CoveredMarkingVisitor(
                            MarkingEncoder<MetaData*, NonStrictMarking>& enc);
            ~CoveredMarkingVisitor();
            virtual bool back(uint32_t index);
            virtual bool set(uint32_t index, bool value);
            virtual bool set_remainder(uint32_t index,
                                            ptriepointer_t<MetaData*> pointer);
            void set_target(NonStrictMarking* m) {target = m;_found=false;}
            NonStrictMarking* decode();
            bool found(){return _found;}
    };
    
}
}

#endif	/* BINARYMARKINGVISITOR_H */

