#include "knots.h"
#include "knots_solver.h"
#include "knots_residue.h"
#include <boost/lexical_cast.hpp>



void Example(){

        KnotCollection C;
        KnotSolver S;
        
        // this represents the knot points of the ois curve
        auto oisdf = C.Curve("oisdf");
        oisdf.Add(Date(2, Feb, 2016));
        oisdf.Add(Date(2, Aug, 2016));
        oisdf.Add(Date(2, Feb, 2017));
        oisdf.Add(Date(2, Feb, 2018));
        oisdf.Add(Date(2, Feb, 2019));
        oisdf.Add(Date(2, Feb, 2021));
        oisdf.Add(Date(2, Feb, 2023));
        oisdf.Add(Date(2, Feb, 2026));

        auto m3df = C.Curve("3mdf");

        // this represents the knot points of the 3 month curve
        m3df.Add(Date( 2, Feb, 2016));

        m3df.Add(Date( 2, May, 2016));
        m3df.Add(Date(15, Jun, 2016));
        m3df.Add(Date(14, Sep, 2016));
        m3df.Add(Date(14, Dec, 2016));

        m3df.Add(Date(15, Mar, 2017));
        m3df.Add(Date(14, Jun, 2017));
        m3df.Add(Date(13, Sep, 2017));
        m3df.Add(Date(13, Dec, 2017));
        
        m3df.Add(Date(14, Mar, 2018));
        m3df.Add(Date( 2, Feb, 2019));
        m3df.Add(Date( 2, Feb, 2021));
        m3df.Add(Date( 2, Feb, 2023));
        m3df.Add(Date( 2, Feb, 2026));

        S.Add<Constant>(Date(2 ,Feb,2016), 1.0, "3mdf");
        S.Add<Constant>(Date(2 ,Feb,2016), 1.0, "oisdf");

        S.Add<FraRate>(Date( 2,Feb,2016), 1.00, "3mdf");
        S.Add<FraRate>(Date(16,Mar,2016), 1.05, "3mdf");
        S.Add<FraRate>(Date(15,Jun,2016), 1.12, "3mdf");
        S.Add<FraRate>(Date(14,Sep,2016), 1.16, "3mdf");
        S.Add<FraRate>(Date(14,Dec,2016), 1.21, "3mdf");
        
        S.Add<FraRate>(Date(15,Mar,2017), 1.27, "3mdf");
        S.Add<FraRate>(Date(14,Jun,2017), 1.45, "3mdf");
        S.Add<FraRate>(Date(13,Sep,2017), 1.68, "3mdf");
        S.Add<FraRate>(Date(13,Dec,2017), 1.92, "3mdf");

        S.Add<SwapRate>(Date(2,Feb,2016), 1.68, 3 * 4, "3mdf");
        S.Add<SwapRate>(Date(2,Feb,2016), 2.1 , 5 * 4, "3mdf");
        S.Add<SwapRate>(Date(2,Feb,2016), 2.2 , 7 * 4, "3mdf");
        S.Add<SwapRate>(Date(2,Feb,2016), 2.07,10 * 4, "3mdf");
        
        S.Add<OisSwapRate>(Date(2,Feb,2016), 0.18, 2);
        S.Add<OisSwapRate>(Date(2,Feb,2016), 0.20, 1*4);
        S.Add<OisSwapRate>(Date(2,Feb,2016), 0.17, 2*4);
        S.Add<OisSwapRate>(Date(2,Feb,2016), 0.15, 3*4);
        S.Add<OisSwapRate>(Date(2,Feb,2016), 0.11, 5*4);
        S.Add<OisSwapRate>(Date(2,Feb,2016), 0.10, 7*4);
        S.Add<OisSwapRate>(Date(2,Feb,2016), 0.09,10*4);


        C.Curve("3mdf").Display();
        boost::optional<KnotCollection> opt_sol;
        try{
                opt_sol  = S.Solve(C);
        }catch(std::exception const& e){
                std::cerr << "Exception: " << e.what() << "\n";
                return;
        }
        auto& sol = *opt_sol;

        sol.Curve("3mdf").Display();
        sol.Curve("oisdf").Display();


        struct View{
                virtual ~View()=default;
                virtual void Emit(std::ostream& os, KnotCollection& C, Date const& d)const=0;
        };
        struct DateView : View{
                virtual void Emit(std::ostream& os, KnotCollection& C, Date const& d)const override{
                        auto end_date = d + Period(3,Months);
                        static auto format_date = [](Date const& d){
                                std::string date_s = boost::lexical_cast<std::string>(d);
                                for(char& c : date_s ) switch(c){case ' ': case ',': c = '_'; }
                                return date_s;
                        };
                        os << format_date(d) << "," << format_date(end_date) << "," << d.serialNumber() << ",";
                }
        };
        struct CurveView : View{
                CurveView(std::string const& curve)
                        :curve_(curve)
                {}
                virtual void Emit(std::ostream& os, KnotCollection& C, Date const& d)const override{
                        auto curve = C.Curve(curve_);

                        auto end_date     = d + Period(3,Months);
                        auto start_df     = curve.Value(d);
                        auto end_df       = curve.Value(end_date);
                        auto implied_rate = RateFromDfCurve(C, d, end_date, curve_);


                        os << start_df << "," << end_df << "," << implied_rate << "," << (curve.IsKnot(d)?1:0) << ",";
                }
        private:
                std::string curve_;
        };

        std::vector<std::shared_ptr<View> > V;
        V.push_back(std::make_shared<DateView>());
        V.push_back(std::make_shared<CurveView>("3mdf"));
        V.push_back(std::make_shared<CurveView>("oisdf"));

        std::ofstream of("3mrate.csv");
        if( of.is_open() ){
                of << "Date,ImpliedRate\n";
                for(Date iter(2,Feb,2016);iter<=Date(2,Nov,2025);++iter){

                        for(auto& _ : V)
                                _->Emit(of, sol, iter);
                        of << "\n";

                }
        }


}

int main(){
        std::cout << std::fixed;
        Example();
}
