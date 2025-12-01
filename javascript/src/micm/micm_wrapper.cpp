#include "micm_wrapper.h"

#include "state_wrapper.h"

#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>

// Include MUSICA headers for real functionality
#include <musica/component_versions.hpp>
#include <musica/micm/micm.hpp>
#include <musica/micm/micm_c_interface.hpp>
#include <musica/micm/state_c_interface.hpp>
#include <musica/util.hpp>
#include <musica/version.hpp>

#include <micm/system/conditions.hpp>

namespace musica_addon
{

  // ============================================================================
  // MICMWrapper Implementation
  // ============================================================================

  MICMWrapper::MICMWrapper(const std::string& config_path, int solver_type)
      : micm_(nullptr),
        solver_type_(solver_type)
  {
    musica::Error error;
    micm_ = musica::CreateMicm(config_path.c_str(), static_cast<musica::MICMSolver>(solver_type), &error);

    if (!musica::IsSuccess(error))
    {
      std::string error_msg = "Failed to create MICM solver: ";
      if (error.message_.value_ != nullptr)
      {
        error_msg += error.message_.value_;
        musica::DeleteString(&error.message_);
      }
      musica::DeleteError(&error);
      throw std::runtime_error(error_msg);
    }
    musica::DeleteError(&error);
  }

  MICMWrapper::~MICMWrapper()
  {
    if (micm_ != nullptr)
    {
      musica::Error error;
      musica::DeleteMicm(micm_, &error);
      musica::DeleteError(&error);
    }
  }

  musica::State* MICMWrapper::CreateState(size_t number_of_grid_cells)
  {
    musica::Error error;
    musica::State* state = musica::CreateMicmState(micm_, number_of_grid_cells, &error);

    if (!musica::IsSuccess(error))
    {
      std::string error_msg = "Failed to create state: ";
      if (error.message_.value_ != nullptr)
      {
        error_msg += error.message_.value_;
        musica::DeleteString(&error.message_);
      }
      musica::DeleteError(&error);
      throw std::runtime_error(error_msg);
    }
    musica::DeleteError(&error);
    return state;
  }

  micm::SolverResult MICMWrapper::Solve(musica::State* state, double time_step)
  {
    musica::Error error;
    micm::SolverResult result = micm_->Solve(state, time_step);

    // No error handling needed as exceptions are propagated from MICM::Solve
    return result;
  }

  int MICMWrapper::GetSolverType() const
  {
    return solver_type_;
  }

}  // namespace musica_addon
