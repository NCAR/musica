// Copyright (C) 2023-2024 National Center for Atmospheric Research
// SPDX-License-Identifier: Apache-2.0
//
// This file contains the defintion of the MICM class, which represents a multi-component reactive transport model.
// It also includes functions for creating and deleting MICM instances with c bindings.
#pragma once

#include <musica/util.hpp>

#include <micm/configure/solver_config.hpp>
#include <micm/process/process_set.hpp>
#include <micm/solver/rosenbrock.hpp>
#include <micm/solver/rosenbrock_solver_parameters.hpp>
#include <micm/solver/solver.hpp>
#include <micm/solver/solver_builder.hpp>
#include <micm/util/matrix.hpp>
#include <micm/util/sparse_matrix_vector_ordering.hpp>
#include <micm/util/vector_matrix.hpp>

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <any>

#ifndef MICM_VECTOR_MATRIX_SIZE
  #define MICM_VECTOR_MATRIX_SIZE 4
#endif

namespace musica {
    class MICM;
    class State;

    State *CreateMicmState(musica::MICM *micm, Error *error);
    void DeleteState(const State *state_wrapper, Error *error);

    class State {
    public:
        State() = default;
        
        /// @brief Vector-ordered Rosenbrock
        using DenseMatrixVector = micm::VectorMatrix<double, MICM_VECTOR_MATRIX_SIZE>;
        using SparseMatrixVector = micm::SparseMatrix<double, micm::SparseMatrixVectorOrdering<MICM_VECTOR_MATRIX_SIZE>>;
        using VectorState = micm::State<DenseMatrixVector, SparseMatrixVector>;

        /// @brief Standard-ordered Rosenbrock solver type
        using DenseMatrixStandard = micm::Matrix<double>;
        using SparseMatrixStandard = micm::SparseMatrix<double, micm::SparseMatrixStandardOrdering>;
        using StandardState = micm::State<DenseMatrixStandard, SparseMatrixStandard>;

        // Define the variant that holds all solver types
        using StateVariant = std::variant<VectorState, StandardState>;

        // Getters and Setters for all the conditions (use Cptr for fortran)
        std::vector<micm::Conditions>& GetConditions() 
        {
            return std::visit([](auto& st) -> std::vector<micm::Conditions>& {
                if (st.conditions_.empty()) {
                    throw std::runtime_error("GetConditions: conditions_ is empty!");
                }
                return st.conditions_;
            }, state_variant_);
        }

        void SetConditions(const std::vector<micm::Conditions>& new_conditions) 
        {
            std::visit([&](auto& st) {
                if (st.conditions_.size() < new_conditions.size()) {
                    throw std::runtime_error("SetConditions: Provided conditions vector is larger than existing conditions.");
                }

                for (size_t i = 0; i < new_conditions.size(); ++i) {
                    st.conditions_[i] = new_conditions[i]; // Copy new conditions
                }
            }, state_variant_);
        }

        std::vector<double>& GetOrderedConcentrations() {
            return std::visit([](auto& st) -> std::vector<double>& {
                return st.variables_.AsVector();
            }, state_variant_);
        }

        // Setter for ordered_concentrations
        void SetOrderedConcentrations(const std::vector<double>& concentrations) { 
            
            std::visit([&](auto& st) {
                for(size_t i = 0; i < concentrations.size(); ++i){
                    st.variables_.AsVector()[i] = concentrations[i];
                }
             }, state_variant_);
        }

        // Setter for ordered_concentrations
        void SetOrderedConcentrations(const double *concentrations) { 
            
            std::visit([&](auto& st) {
                for(size_t i = 0; i < st.variables_.AsVector().size(); ++i){
                    st.variables_.AsVector()[i] = concentrations[i];
                }
             }, state_variant_);
        }

        // Getter for ordered_rate_constants
        std::vector<double>& GetOrderedRateConstants() { 
            return std::visit([](auto& st) -> std::vector<double>& {
                return st.custom_rate_parameters_.AsVector();
            }, state_variant_);
        }

        // Setter for ordered_concentrations
        void SetOrderedRateConstants(const std::vector<double>& rateConstant) { 
             std::visit([&](auto& st) {
                for(size_t i = 0; i < rateConstant.size(); ++i){
                    st.custom_rate_parameters_.AsVector()[i] = rateConstant[i];
                }
             }, state_variant_);
        }

        void SetOrderedRateConstants(const double *rateConstant) { 
             std::visit([&](auto& st) {
                for(size_t i = 0; i < st.custom_rate_parameters_.AsVector().size(); ++i){
                    st.custom_rate_parameters_.AsVector()[i] = rateConstant[i];
                }
             }, state_variant_);
        }
       
        StateVariant state_variant_;
    };

} // namespace musica