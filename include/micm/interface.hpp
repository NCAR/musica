#include <stdlib.h>

#include <micm/solver/rosenbrock.hpp>
#include <micm/solver/state.hpp>
#include <micm/util/matrix.hpp>

class MICM {
  public:
    micm::RosenbrockSolver<micm::Matrix>* solver_;
};

#ifdef __cplusplus
  extern "C"
  {
#endif

    typedef void (*FuncPtr)(double[], int64_t, int64_t);

    FuncPtr get_solver(char filepath[]);

#ifdef __cplusplus
  }  // extern "C"
#endif

