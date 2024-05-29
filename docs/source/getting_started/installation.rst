Installation
============

MUSICA source code may be cloned from its public GitHub repository
and configured and built with the cmake utility.
In brief:

.. code-block:: console
 
    $ git clone https://github.com/NCAR/musica.git
    $ cd musica
    $ mkdir build
    $ cd build
    $ ccmake ..
    $ make
    $ make install

Linking
=======

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
