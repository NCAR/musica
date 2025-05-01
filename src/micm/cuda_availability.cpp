#include <musica/micm/cuda_availability.hpp>

#ifdef MUSICA_ENABLE_CUDA
  #include <cuda_runtime.h>
#endif

namespace musica
{
  bool IsCudaAvailable()
  {
    int device_count = 0;
#ifdef MUSICA_ENABLE_CUDA
    cudaError_t error = cudaGetDeviceCount(&device_count);
    if (error != cudaSuccess)
    {
      return false;
    }
#endif
    return device_count > 0;
  }
}  // namespace musica