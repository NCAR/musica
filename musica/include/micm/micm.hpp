#pragma once

#include <string>
#include <vector>
#include <cstddef>

#include <micm/solver/rosenbrock.hpp>
#include <micm/util/sparse_matrix_vector_ordering.hpp>
#include <micm/util/vector_matrix.hpp>


class MICM
{
public:
    /// @brief Constructor
    /// @param config_path Path to the configuration file or the directory containing the configuration files 
    MICM(const std::string& config_path);

    /// @brief Destructor
    ~MICM();

    /// @brief Create a solver
    /// @return Status of solver creation related to parsing configuration files.
    ///         The return value represents the error code for CAM-SIMA 
    int create_solver();
    
    /// @brief Solve the system
    /// @param temperature Temperature [K]
    /// @param pressure Pressure [Pa-1]
    /// @param time_step Time [s] to advance the state by
    /// @param num_concentrations The number of species' concentrations
    /// @param concentrations Species's concentrations
    void solve(double temperature, double pressure, double time_step, int num_concentrations, double*& concentrations);

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