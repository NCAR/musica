#pragma once

#include <string>
#include <vector>
#include <cstddef>
#include <memory>

#include <micm/solver/rosenbrock.hpp>
#include <micm/util/sparse_matrix_vector_ordering.hpp>
#include <micm/util/vector_matrix.hpp>

class MICM 
{
public:
    MICM(const std::string& config_path);
    ~MICM();

    // TODO(jiwon): can return type indicate error?
    int create_solver();
    void delete_solver() const;
    void solve(double temperature, double pressure, double time_step, double*& concentrations, size_t num_concentrations);

private:
    static constexpr size_t NUM_GRID_CELLS = 1; // TODO(jiwon)

    std::string config_path_;

    // TODO(jiwon) - currently hard coded
    std::vector<double> v_concentrations_;

    // TODO(jiwon) - currently hard coded
    template <class T = double>
    using Vector1MatrixParam = micm::VectorMatrix<T, 1>;
    template <class T = double>
    using Vector1SparseMatrixParam = micm::SparseMatrix<T, micm::SparseMatrixVectorOrdering<1>>;
    typedef micm::RosenbrockSolver<Vector1MatrixParam, Vector1SparseMatrixParam> VectorRosenbrockSolver;
    VectorRosenbrockSolver* solver_;
};

