# Decide how consumers get MIEM at link time.
#
# MIEM is installed as its own archive (libmiem.a) and exported as musica::miem.
# libmusica references miem:: symbols, so the pkg-config Libs line has to name it or
# a consumer fails on undefined miem::Emissions::Run.
#
# -lmiem must follow -lmusica: libmusica is what references into it.
#
# Sets, for use in the *.pc.in templates:
#   MUSICA_PC_MIEM_LIBS     extra Libs entry for MIEM, or empty
macro(musica_set_miem_packaging_vars)
  set(MUSICA_PC_MIEM_LIBS "")
  if(MUSICA_ENABLE_MIEM)
    set(MUSICA_PC_MIEM_LIBS " -lmiem")
  endif()
endmacro()
