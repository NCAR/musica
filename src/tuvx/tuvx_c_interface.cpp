
#include <musica/tuvx/tuvx_c_interface.hpp>

namespace musica
{
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

  TUVX *CreateTuvxFromConfigOnly(const char *config_path, Error *error)
  {
    DeleteError(error);
    TUVX *tuvx = new TUVX();

    try
    {
      tuvx->CreateFromConfigOnly(config_path);
      *error = NoError();
    }
    catch (const std::exception &e)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString(e.what()) };
      delete tuvx;
      return nullptr;
    }
    catch (...)
    {
      *error = Error{ 1, CreateString(MUSICA_ERROR_CATEGORY), CreateString("Unknown error in CreateFromConfigOnly") };
      delete tuvx;
      return nullptr;
    }
    return tuvx;
  }

  void DeleteTuvx(const TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    if (tuvx == nullptr)
    {
      *error = NoError();
      return;
    }
    try
    {
      delete tuvx;
      *error = NoError();
    }
    catch (const std::system_error &e)
    {
      *error = ToError(e);
    }
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

  Mappings GetPhotolysisRateConstantsOrdering(TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    return tuvx->GetPhotolysisRateConstantsOrdering(error);
  }

  Mappings GetHeatingRatesOrdering(TUVX *tuvx, Error *error)
  {
    DeleteError(error);
    return tuvx->GetHeatingRatesOrdering(error);
  }

  void RunTuvx(
      TUVX *const tuvx,
      const double solar_zenith_angle,
      const double earth_sun_distance,
      double *const photolysis_rate_constants,
      double *const heating_rates,
      Error *const error)
  {
    DeleteError(error);
    tuvx->Run(solar_zenith_angle, earth_sun_distance, photolysis_rate_constants, heating_rates, error);
  }
}  // namespace musica