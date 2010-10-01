#ifndef VERIFYTAPN_TAPN_TimedArcPetriNet_HPP_
#define VERIFYTAPN_TAPN_TimedArcPetriNet_HPP_

#include <iostream>
#include "TimedPlace.hpp"
#include "TimedTransition.hpp"
#include "TimedInputArc.hpp"
#include "OutputArc.hpp"
#include "boost/make_shared.hpp"

namespace VerifyTAPN {
	namespace TAPN {

		class TimedArcPetriNet
		{
		public:// construction
			TimedArcPetriNet(const TimedPlace::Vector& places,
				const TimedTransition::Vector& transitions,
				const TimedInputArc::Vector& inputArcs,
				const OutputArc::Vector& outputArcs)
				: places(places), transitions(transitions), inputArcs(inputArcs), outputArcs(outputArcs) { };
			virtual ~TimedArcPetriNet() { /* empty */ }

		public: // inspectors
			void Print(std::ostream& out) const;

		public: // modifiers
			void Initialize();

		private: // data
			const TimedPlace::Vector places;
			const TimedTransition::Vector transitions;
			const TimedInputArc::Vector inputArcs;
			const OutputArc::Vector outputArcs;
		};

		inline std::ostream& operator<<(std::ostream& out, const VerifyTAPN::TAPN::TimedArcPetriNet& tapn)
		{
			tapn.Print( out );
			return out;
		}
	}
}

#endif /* VERIFYTAPN_TAPN_TimedArcPetriNet_HPP_ */