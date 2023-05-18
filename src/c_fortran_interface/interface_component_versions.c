#include <ISO_Fortran_binding.h>
#include <string.h>
#include <stdlib.h>

#include <musica/component_versions.h>

#ifdef __cplusplus
extern "C" {
#endif
  void c_getAllComponentVersions(CFI_cdesc_t* Fstr, int *ret);
#ifdef __cplusplus
}
#endif

void c_getAllComponentVersions(CFI_cdesc_t* Fstr, int *ret) {
  char* versions = getAllComponentVersions();
  size_t len = strlen(versions);
  *ret = CFI_allocate(Fstr, (CFI_index_t *)0, (CFI_index_t *)0, len);
  if (*ret == 0) {
    memcpy(Fstr->base_addr, versions, len);
  }
  free(versions);
}