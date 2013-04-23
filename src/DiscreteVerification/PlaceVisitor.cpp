/*
 * PlaceVisitor.cpp
 *
 *  Created on: 21/03/2012
 *      Author: MathiasGS
 */

#include "PlaceVisitor.hpp"

namespace VerifyTAPN {
namespace DiscreteVerification {

		void PlaceVisitor::visit(const NotExpression& expr, boost::any& context)
		{
			expr.Child().Accept(*this, context);
		}

		void PlaceVisitor::visit(const ParExpression& expr, boost::any& context)
		{
			expr.Child().Accept(*this, context);
		}

		void PlaceVisitor::visit(const OrExpression& expr, boost::any& context)
		{
			expr.Left().Accept(*this, context);
			expr.Right().Accept(*this, context);
		}

		void PlaceVisitor::visit(const AndExpression& expr, boost::any& context)
		{
			expr.Left().Accept(*this, context);
			expr.Right().Accept(*this, context);
		}

		void PlaceVisitor::visit(const AtomicProposition& expr, boost::any& context)
		{
			std::vector<int> v = boost::any_cast< std::vector< int > >(context);
			v.push_back(expr.Place());
			context = v;
		}

		void PlaceVisitor::visit(const BoolExpression& expr, boost::any& context)
		{
		}

		void PlaceVisitor::visit(const Query& query, boost::any& context)
		{
			query.Child().Accept(*this, context);
		}
} /* namespace DiscreteVerification */
} /* namespace VerifyTAPN */
