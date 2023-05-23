#include <ISO_Fortran_binding.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <musica/component_versions.h>

#ifdef __cplusplus
extern "C" {
#endif
  void c_getAllComponentVersions(CFI_cdesc_t* Fstr, int *ret);
  int c_returnInteger();
#ifdef __cplusplus
}
#endif

void c_getAllComponentVersions(CFI_cdesc_t* Fstr, int *ret) {
  char* versions = getAllComponentVersions();
  size_t len = strlen(versions);
  printf("Length: %zu\n", len);
  printf("Versions: %s\n", versions);
  *ret = CFI_allocate(Fstr, (CFI_index_t *)0, (CFI_index_t *)0, len);
  if (*ret == 0) {
    printf("CFI_allocate succeeded\n");
    memcpy(Fstr->base_addr, versions, len);
  }
  free(versions);
}

int c_returnInteger() {
  // Return an integer value to Fortran
  return 42;
}