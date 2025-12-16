#include <musica/tuvx/tuvx_c_interface.hpp>

namespace musica
{
#ifdef __cplusplus
  extern "C"
  {
#endif

    // TUVX external C API functions
    TUVX *CreateTuvx(const char *config_path, GridMap *grids, ProfileMap *profiles, RadiatorMap *radiators, Error *error)
    {
      DeleteError(error);
      TUVX *tuvx = new TUVX();

      tuvx->Create(config_path, grids, profiles, radiators, error);
      if (!IsSuccess(*error))
      {
        delete tuvx;
        return nullptr;
      }
      return tuvx;
    }

    void DeleteTuvx(const TUVX *tuvx, Error *error)
    {
      DeleteError(error);
      try
      {
        if (tuvx != nullptr)
          delete tuvx;
      }
      catch (const std::system_error &e)
      {
        ToError(e, error);
      }
      NoError(error);
    }

    GridMap *GetGridMap(TUVX *tuvx, Error *error)
    {
      DeleteError(error);

      return tuvx->CreateGridMap(error);
    }

    ProfileMap *GetProfileMap(TUVX *tuvx, Error *error)
    {
      DeleteError(error);
      return tuvx->CreateProfileMap(error);
    }

    RadiatorMap *GetRadiatorMap(TUVX *tuvx, Error *error)
    {
      DeleteError(error);
      return tuvx->CreateRadiatorMap(error);
    }

    void GetPhotolysisRateConstantsOrdering(TUVX *tuvx, Mappings *mappings, Error *error)
    {
      DeleteError(error);
      return tuvx->GetPhotolysisRateConstantsOrdering(mappings, error);
    }

    void GetHeatingRatesOrdering(TUVX *tuvx, Mappings *mappings, Error *error)
    {
      DeleteError(error);
      return tuvx->GetHeatingRatesOrdering(mappings, error);
    }

    void GetDoseRatesOrdering(TUVX *tuvx, Mappings *mappings, Error *error)
    {
      DeleteError(error);
      return tuvx->GetDoseRatesOrdering(mappings, error);
    }

    void RunTuvx(
        TUVX *const tuvx,
        const double solar_zenith_angle,
        const double earth_sun_distance,
        double *const photolysis_rate_constants,
        double *const heating_rates,
        double *const dose_rates,
        Error *const error)
    {
      DeleteError(error);
      tuvx->Run(solar_zenith_angle, earth_sun_distance, photolysis_rate_constants, heating_rates, dose_rates, error);
    }

    void TuvxVersion(String *tuvx_version)
    {
      CreateString(TUVX::GetVersion().c_str(), tuvx_version);
    }

#ifdef __cplusplus
  }
#endif

}  // namespace musica