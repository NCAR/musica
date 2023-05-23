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

  CFI_index_t lower[1] = {1};
  CFI_index_t upper[1] = {len};

  // Establish the descriptor for the character array
  CFI_establish(
    Fstr, versions, 
    CFI_attribute_other, CFI_type_char, 
    len, lower, upper
  );

  *ret = CFI_allocate(Fstr, (CFI_index_t *)0, (CFI_index_t *)0, len);
  if (*ret == 0) {
    memcpy(Fstr->base_addr, versions, len);
  }
  free(versions);
}

int c_returnInteger() {
  // Return an integer value to Fortran
  return 42;
}
