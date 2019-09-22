#ifndef KNOTS_SOLVER_H
#define KNOTS_SOLVER_H

#include "knots.h"

struct KnotSolver{
        struct Residue{
                virtual ~Residue()=default;
                virtual RealType Calc(KnotCollection& V, bool debug=false)const=0;
        };
        VectorType CalcResidue(KnotCollection& V)const{
                VectorType ret(res_.size());
                for(size_t idx=0;idx!=res_.size();++idx){
                        ret(idx) = res_[idx]->Calc(V, true);
                }
                return ret;
        }
        MatrixType NumericalJacobian(KnotCollection& V)const{
                const RealType epsilon = 1e-10;

                MatrixType J(res_.size(), V.size());
                for(size_t i=0;i!=V.size();++i){
                        KnotCollection upper_V = V;
                        KnotCollection lower_V = V;
                        upper_V[i].value += epsilon / 2;
                        lower_V[i].value -= epsilon / 2;
                        for(size_t j=0;j!=res_.size();++j){
                                RealType upper = res_[j]->Calc(upper_V);
                                RealType lower = res_[j]->Calc(lower_V);
                                RealType calc = ( upper - lower ) / epsilon;
                                J(j,i) = calc;
                        }
                }

                return J;
        }
        template<class T, class... Args>
        KnotSolver& Add(Args&&... args){
                res_.push_back(std::make_shared<T>(args...));
                return *this;
        }
        KnotCollection Solve(KnotCollection k);
private:
        std::vector<std::shared_ptr<Residue> > res_;
        /*
                Solving essentailly works by calculating
                a direction vector V such that
                        X_{i+1} = x_{i} + V,
                however we normally want to satisfy the
                wolfe conditions
                        
         */
        double beta_parameter_;
};

#endif // KNOTS_SOLVER_H
