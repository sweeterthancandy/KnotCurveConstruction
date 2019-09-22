#ifndef KNOTS_RESIDUE_H
#define KNOTS_RESIDUE_H



struct Constant : KnotSolver::Residue{
        enum{ Debug = 1 };
        Constant(Date date, RealType target, std::string const& curve)
                :date_(date),
                target_(target),
                curve_(curve)
        {}
        virtual RealType Calc(KnotCollection& V, bool debug)const{
                RealType val = V.Curve(curve_).Value(date_);
                RealType residue = std::fabs( val  - target_ );
                SLOG(trace) << "Constant.residue=" << residue << ", val=" << val;
                return residue;
        }
private:
        Date date_;
        RealType target_;
        std::string curve_;
};
struct RateBetween : KnotSolver::Residue{
        RateBetween(Date start, Date end, RealType rate, std::string const& curve)
                :start_(start),
                end_(end),
                rate_(rate),
                curve_(curve)
        {}
        virtual RealType Calc(KnotCollection& V, bool debug )const{
                RealType start_df = V.Curve(curve_).Value( start_ );
                RealType end_df   = V.Curve(curve_).Value( end_ );


                RealType val = ( start_df / end_df - 1.0 ) / (end_ - start_ ) * 365.0 * 100.0;

                RealType residue = std::fabs( val - rate_ );

                return residue;
        }
private:
        Date start_;
        Date end_;
        RealType rate_;
        std::string curve_;
};
struct BasisDiff : KnotSolver::Residue{
        BasisDiff(Date point, std::string const& left, std::string const& right, double basis)
                :point_(point),
                left_(left),
                right_(right),
                basis_(basis)
        {}
        virtual RealType Calc(KnotCollection& V, bool debug )const{
                auto A = V.Curve(left_).Value(point_);
                auto B = V.Curve(right_).Value(point_);
                auto val = ( A - B );
                auto residue = std::fabs( val - basis_ );
                return residue;
        }
private:
        Date point_;
        std::string left_;
        std::string right_;
        double basis_;
};
struct SwapRate : KnotSolver::Residue{
        SwapRate(Date start, RealType rate, double periods, std::string const& curve)
                :start_(start),
                rate_(rate),
                periods_(periods),
                curve_(curve)
        {}
        virtual RealType Calc(KnotCollection& V, bool debug )const{

                Period d(3, Months);

                RealType nume = 0.0;
                RealType deno = 0.0;
                
                Date iter = start_;
                for(size_t idx=0;idx!=periods_;++idx){
                        auto start = iter;
                        auto end  =  iter + d;
                        RealType yf = ( end - start ) / 365.0;
                        RealType df = V.Curve("oisdf").Value(end);


                        RealType start_df = V.Curve(curve_).Value( start );
                        RealType end_df   = V.Curve(curve_).Value( end );


                        RealType ri = ( start_df / end_df - 1.0 ) / yf * 100.0;

                        nume += yf * ri * df;
                        deno += yf * df;

                        iter += d;
                }

                RealType val = nume / deno;

                RealType residue = std::fabs( val - rate_ );

                return residue;
        }
private:
        Date start_;
        RealType rate_;
        double periods_;
        std::string curve_;
};

struct OisSwapRate : KnotSolver::Residue{
        OisSwapRate(Date start, RealType rate, double periods)
                :start_(start),
                rate_(rate),
                periods_(periods)
        {}
        virtual RealType Calc(KnotCollection& V, bool debug )const{

                Period d(3, Months);

                RealType m3_nume = 0.0;
                RealType m3_deno = 0.0;
                
                RealType ois_nume = 0.0;
                RealType ois_deno = 0.0;
                
                Date iter = start_;
                for(size_t idx=0;idx!=periods_;++idx){
                        auto start = iter;
                        auto end  =  iter + d;
        
                        RealType yf = ( end - start ) / 365.0;

                        auto m3rate  = RateFromDfCurve(V, start, end, "3mdf");
                        auto oisrate = RateFromDfCurve(V, start, end, "oisdf");
                        
                        RealType df = V.Curve("oisdf").Value(end);

                        m3_nume += yf * m3rate * df;
                        m3_deno += yf * df;
                        
                        ois_nume += yf * oisrate * df;
                        ois_deno += yf * df;

                        iter += d;
                }

                RealType m3_fixed = m3_nume / m3_deno;
                RealType ois_fixed = ois_nume / ois_deno;


                RealType basis = ( m3_fixed - ois_fixed ) * 100.0;

                SLOG(trace) << "m3_fixed = " << m3_fixed;
                SLOG(trace) << "ois_fixed = " << ois_fixed;
                SLOG(trace) << "basis = " << basis << "\n";
                RealType residue = std::fabs( basis - rate_ );

                return residue;
        }
private:
        Date start_;
        RealType rate_;
        double periods_;
};
struct FraRate : KnotSolver::Residue{
        enum{ Debug =0 };
        FraRate(Date d, RealType quote, std::string const& curve)
                :d_(d),
                quote_(quote),
                curve_(curve)
        {}
        virtual RealType Calc(KnotCollection& V, bool debug )const{
                static QuantLib::Period p(3, Months);
                Date end = d_ + p;
                RealType start_df = V.Curve(curve_).Value( d_ );
                RealType end_df   = V.Curve(curve_).Value( end );


                RealType rate = ( start_df / end_df - 1.0 ) / ( end - d_ ) * 365 * 100.0;

                RealType residue = std::fabs( rate - quote_ );

                if( Debug || debug){
                        std::cout << "---------------------\n";
                        std::cout << "quote_ = " << quote_ << "\n";
                        std::cout << "d_ = " << d_ << "\n";
                        std::cout << "end = " << end << "\n";
                        std::cout << "start_df = " << start_df << "\n";
                        std::cout << "end_df = " << end_df << "\n";
                        std::cout << "rate = " << rate << "\n";
                        std::cout << "residue = " << residue << "\n";
                }
                return residue;
        }
private:
        Date d_;
        RealType quote_;
        std::string curve_;
};

#endif // KNOTS_RESIDUE_H
