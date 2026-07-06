#include <musica/micm/cuda_availability.hpp>
#include <musica/micm/cuda_loader.hpp>

namespace musica
{
  bool IsCudaAvailable()
  {
    // Use the runtime CUDA loader to check availability
    return CudaLoader::GetInstance().HasDevices();
  }
}  // namespace musica
