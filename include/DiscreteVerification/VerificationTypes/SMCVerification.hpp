#ifndef SMCVERIFICATION_HPP
#define SMCVERIFICATION_HPP

#include "DiscreteVerification/VerificationTypes/Verification.hpp"
#include "DiscreteVerification/DataStructures/NonStrictMarking.hpp"
#include "DiscreteVerification/Generators/RandomRunGenerator.h"

namespace VerifyTAPN::DiscreteVerification {

class SMCVerification : public Verification<NonStrictMarking> {

    public:

        SMCVerification(TAPN::TimedArcPetriNet &tapn, NonStrictMarking &initialMarking, AST::Query *query,
                        VerificationOptions options) 
            : Verification(tapn, initialMarking, query, options)
            , runGenerator(tapn), numberOfRuns(0), maxTokensSeen(0)
            {}

        virtual bool run() override;

        virtual bool executeRun();

        virtual void printStats() override;
        void printTransitionStatistics() const override;

        unsigned int maxUsedTokens() override;
        void setMaxTokensIfGreater(unsigned int i);

        virtual bool reachedRunBound();
        
        virtual void handleRunResult(const bool res) = 0;
        virtual bool mustDoAnotherRun() = 0;

    protected:

        RandomRunGenerator runGenerator;
        size_t numberOfRuns;
        unsigned int maxTokensSeen;

};

}

#endif  /* SMC_VERIFICATION_HPP */