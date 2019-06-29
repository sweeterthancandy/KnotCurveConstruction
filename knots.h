#ifndef KNOTS_H
#define KNOTS_H

#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <unordered_set>

#include <Eigen/Dense>

#include <ql/qldefines.hpp>
#include <ql/time/period.hpp>
#include <ql/instruments/forwardrateagreement.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/indexes/ibor/all.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/math/interpolations/all.hpp>

#include <boost/range.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/log/trivial.hpp>

#include <boost/range/any_range.hpp>

#define SLOG(level) BOOST_LOG_TRIVIAL(level) << "[" << __PRETTY_FUNCTION__ << "] "
using namespace QuantLib;

// this is important
using RealType = long double;

using MatrixType = Eigen::Matrix<RealType, Eigen::Dynamic, Eigen::Dynamic>;
using VectorType = Eigen::Matrix<RealType, Eigen::Dynamic, 1>; 

struct Knot{

        Knot(std::string const& curve_, Date date_, RealType value_)
                :curve(curve_),
                date(date_),
                value(value_)
        {}

        std::string curve;
        Date date;
        RealType value;
};



struct KnotCollection : std::vector<Knot>{
                
        using KnotRange = boost::any_range<
                Knot,
                boost::bidirectional_traversal_tag,
                Knot&,
                std::ptrdiff_t
        >;


        struct KnotCurve{
                KnotCurve(KnotCollection* collection, std::string const& curve)
                        :collection_(collection),
                        curve_(curve)
                {}

                std::string Name()const{ return curve_; }

                KnotRange Rng(){
                        return *collection_ | boost::adaptors::filtered([&](auto&& k){ return k.curve == curve_; });
                }

                KnotCurve& Add(Date const& d, RealType value = 1.0){
                        collection_->emplace_back(curve_, d, value);
                        return *this;
                }
                KnotCurve& Fill(RealType val){
                        for(auto& _ : Rng()){
                                _.value = val;
                        }
                        return *this;
                }



                RealType Value(Date const& d);

                enum LowerUpperBoundCategory{
                        LUB_NotAnInterval,
                        LUB_UnboundedBelow,
                        LUB_UnboundedAbove,
                        LUB_Singleton,
                        LUB_Bounded,
                };
                struct LowerUpperBoundResult{
                        boost::optional<Knot&> lower;
                        boost::optional<Knot&> upper;

                        LowerUpperBoundCategory Categorize()const{
                                if( ! lower && ! upper )
                                        return LUB_NotAnInterval;
                                if( ! lower )
                                        return LUB_UnboundedBelow;
                                if( ! upper )
                                        return LUB_UnboundedAbove;
                                if( &*lower == &*upper )
                                        return LUB_Singleton;
                                return LUB_Bounded;
                        }

                };
                LowerUpperBoundResult LowerUpperBound(Date const& d){
                        LowerUpperBoundResult result;
                        for( auto& k : Rng() ){
                                if( k.date <= d ){
                                        result.lower = k;
                                }
                                if( d <= k.date ){
                                        result.upper = k;
                                        break;
                                }
                        }
                        return result;
                }

                bool IsKnot(Date const& d){
                        for( auto const& k : Rng() ){
                                if( k.date == d )
                                        return true;
                        }
                        return false;
                }

                VectorType AsVector(){

                        VectorType ret(0);
                        size_t idx =0 ;
                        for(auto const& k : Rng()){
                                ret.resize(idx+1);
                                ret(idx) = k.value;
                                ++idx;
                        }
                        return ret;
                }
                void Display(){
                        std::cout << "=========" << curve_ << "=========\n";
                        for(auto const& _ : Rng()){
                                std::cout << _.date << " => " << _.value << "\n";
                        }
                        std::cout << "==================================\n";
                }

        private:
                KnotCollection* collection_;
                std::string curve_;
        };

        KnotCurve Curve(std::string const& name){
                names_.insert(name);
                return KnotCurve{this, name};
        }
        
        VectorType AsVector()const{
                VectorType ret(size());
                for(size_t idx=0;idx!=size();++idx){
                        ret(idx) = at(idx).value;
                }
                return ret;
        }
private:
        std::unordered_set<std::string> names_;
};

inline RealType RateFromDfCurve(KnotCollection& V,
                                Date const& start,
                                Date const& end,
                                std::string const& curve)
{
        RealType start_df = V.Curve(curve).Value( start );
        RealType end_df   = V.Curve(curve).Value( end );
        RealType yf = ( end - start ) / 365.0;
        RealType implied_rate = ( start_df / end_df - 1.0 ) / yf;
        return implied_rate;
}

#endif // KNOTS_H
