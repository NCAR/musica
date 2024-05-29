Chapter 1
=========

Linking a Fortran Model
-----------------------

.. code-block:: f90

  program demo
      use musica_util, only: string_t
      use musica_micm, only: get_micm_version
      implicit none
      type(string_t) :: micm_version
      micm_version = get_micm_version()
      print *, "MICM version ", micm_version%get_char_array()
  end program demo

.. code-block:: bash

  gfortran -o demo demo.f90 -I<MUSICA_DIR>/include -L<MUSICA_DIR>/lib -lmusica-fortran -lmusica -lstdc++
