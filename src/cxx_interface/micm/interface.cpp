#include <micm/interface.hpp>

#include <iostream>

// assumes that photo_rates, matches order in c++ already
void fortran_solve(void *micm_address, double time_start, double time_end, double concentrations[], double temperature, double pressure, double photo_rates[])
{
  MICM *micm = static_cast<MICM *>(micm_address);
  micm::State state = *(micm->state_);

  state.update_photo_rates(photo_rates);
  state.concentrations_ = concentrations;
  state.pressure_ = pressure;
  state.temperature_ = temperature;

  auto result = micm->solver_->Solve(time_start, time_end, state);
}

void solver(
    double state[], // NOLINT(misc-unused-parameters,cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    int64_t state_size,
    int64_t
        timestep)
{
  std::cout << "state size: " << state_size << std::endl;
  std::cout << "timestep: " << timestep << std::endl;

  for (int64_t i{}; i < state_size; ++i)
  {
    std::cout << "state[" << i << "] = " << state[i] << std::endl;
  }
}

FuncPtr get_solver(
    char filepath[]) // NOLINT(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
{
  std::cout << "file path: " << filepath << "\n";
  return &solver;
}