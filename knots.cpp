#include "knots.h"

#include <boost/assert.hpp>

RealType KnotCollection::KnotCurve::Value(Date const& d){
        // one of four cases
        //    A) d is before all knots, then we just return the first rate
        //    B) d is after all knots, then we just return the last rate
        //    C) d is on a knot
        //    D) d is between two knots
        
        auto lu = LowerUpperBound(d);

        switch(lu.Categorize()){
                case LUB_NotAnInterval:
                {
                        throw std::domain_error("no knots!");
                }
                case LUB_UnboundedBelow:
                {
                        return lu.upper->value;
                }
                case LUB_UnboundedAbove:
                {
                        return lu.lower->value;
                }
                case LUB_Singleton:
                {
                        return lu.lower->value;
                }
                case LUB_Bounded:
                {
                        BOOST_ASSERT( d <= lu.upper->date );
                        BOOST_ASSERT( lu.lower->date <= d );

                        double a = lu.upper->date - d;
                        double b = lu.upper->date - lu.lower->date;


                        //auto intrp = lu.lower->value * a / b + lu.upper->value * ( 1.0 - a/b );
                        auto intrp = std::exp(std::log(lu.lower->value) * a / b + std::log(lu.upper->value) * ( 1.0 - a/b ));

                        return intrp;
                }
        }
        
        std::abort();
        


}
