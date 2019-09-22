#include "knots_solver.h"

KnotCollection KnotSolver::Solve(KnotCollection k){

        enum{ Debug = 1 };

        enum{ MaxIter = 1000 };
        for(size_t iter=0;iter < MaxIter;++iter){
                
                #if 1
                MatrixType J = NumericalJacobian(k);
                MatrixType JT_J = ( J.transpose() * J );
                VectorType F = CalcResidue(k);

                MatrixType A = JT_J;
                MatrixType B = - J.transpose() * F;

                using namespace Eigen;
                VectorType sol = A.bdcSvd(ComputeThinU | ComputeThinV).solve(B);
                // sol = x_{n+1} - x_{n}
                // sol + x_{n} = x_{n_1}
                VectorType V = k.AsVector();

                // now we have the direction V, we want to figure out the step lengh \alpha

                double alpha = 0.5;
                VectorType next = V + alpha * sol;

                

                RealType norm = sol.lpNorm<2>();
                
                if( Debug ){
                        std::cout << "J = \n" << J << "\n";
                        std::cout << "F = \n" << F << "\n";
                        std::cout << "V = " << V << "\n";
                        std::cout << "next = " << next << "\n";
                        std::cout << "norm = " << norm << "\n";
                }
                
                for(size_t idx=0;idx!=k.size();++idx){
                        k[idx].value = next(idx);
                }
                
                if(0)
                do{
                        // wolfe condition
                        //    f(x_i + \alpha_i P_k) <= f(x_i) + c_1 \alpha_k P^T_k \grad f(x_k)
                        //    \\-- F_next --------/    \--F-/                \---- tmp ------/

                        VectorType F_next = CalcResidue(k);

                        double c_1   = 1.0;
                        double alpha = 1.0;

                        VectorType lhs = F_next - F;
                        VectorType rhs = c_1 * alpha * V.transpose() * J;
                        //VectorType res = F_next - F - c_1 * alpha * V.transpose() * J;


                        SLOG(trace) << "lhs = " << ToString(lhs);
                        SLOG(trace) << "rhs = " << ToString(rhs);
                        VectorType res = lhs - rhs;
                        SLOG(trace) << "res = " << ToString(res);

                }while(0);

                if( norm < 1e-5 ){
                        return k;
                }

                #else
                MatrixType J = NumericalJacobian(k);

                //RealType det = J.determinant();
                #if 1
                MatrixType JT_J = ( J.transpose() * J );
                MatrixType G = JT_J.inverse() * J.transpose();
                #else
                MatrixType G = J.inverse();
                #endif
                VectorType F = CalcResidue(k);
                VectorType V = k.AsVector();

                using namespace Eigen;
                JacobiSVD<MatrixType> svd(JT_J);
                auto const& sv = svd.singularValues();
                RealType cond = sv(0) / sv(sv.size()-1);

        
                VectorType G_F = G * F;


                VectorType next = V - G*F;

                VectorType d = next - V;

                RealType norm = d.lpNorm<2>();
                
                if( Debug ){
                        std::cout << "J = \n" << J << "\n";
                        std::cout << "J^T*J = \n" << JT_J << "\n";
                        std::cout << "cond = " << cond << "\n";
                        std::cout << "G = \n" << G << "\n";
                        std::cout << "F = \n" << F << "\n";
                        std::cout << "G*F = \n" << G_F << "\n";
                        std::cout << "V = " << V << "\n";
                        std::cout << "next = " << next << "\n";
                        std::cout << "norm = " << norm << "\n";
                }

                for(size_t idx=0;idx!=k.size();++idx){
                        k[idx].value = next(idx);
                }

                if( norm < 1e-5 ){
                        return k;
                }
                #endif

        }

        std::cerr << "Failed to converge\n";
        std::exit(1);
        return k;
}
