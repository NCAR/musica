Overview
========

The MUSICA-Fortran interface exposes the MUSICA C++ library to Fortran programs via the
Fortran-C interoperability features of ``iso_c_binding``. For installation, see
:doc:`../../getting_started/fortran`.

The tutorials below progress from printing the MICM version through running a
multi-grid-cell box model. Each section follows the same pattern:

- **Defining a Mechanism** — load a :doc:`mechanism configuration file <mc:index>` directory
- **Creating a Solver** — instantiate ``micm_t`` with a config path and solver type
- **Setting Conditions** — populate the ``state`` concentrations and environmental parameters
- **Solving** — call ``micm%solve`` at each time step
- **Accessing Results** — read updated concentrations from the state array

.. note::

   TUV-x photolysis and CARMA aerosol support are not yet available in the
   Fortran API.

Fortran-C Interoperability
---------------------------

MUSICA's Fortran API is built on the standard ``iso_c_binding`` module. Here is a minimal
example showing how a C function can be called from Fortran, which is the same mechanism
underlying all MUSICA Fortran bindings:

.. code-block:: cpp

  #include <stdio.h>

  void test_proc_c(int n, double A[3][2]) {
      printf("n = %d\n", n);
      for (int i = 0; i < 2; i++) {
          for (int j = 0; j < 3; j++) printf("%6.2f ", A[j][i]);
          printf("\n");
      }
  }

.. code-block:: f90

  program demo_fort
      use iso_c_binding, only: c_int, c_double
      implicit none
      integer(c_int) :: n_fort = 7
      real(c_double), dimension(2, 3) :: A_fort
      interface
          subroutine test_proc_c(n_c, A_c) bind(C, name='test_proc_c')
              use iso_c_binding, only: c_int, c_double
              integer(c_int), intent(in), value :: n_c
              real(c_double), dimension(2, 3), intent(in) :: A_c
          end subroutine test_proc_c
      end interface
      integer :: i, j
      do j = 1, 3
          do i = 1, 2
              A_fort(i, j) = real(i + j, c_double)
          end do
      end do
      call test_proc_c(n_fort, A_fort)
  end program demo_fort

.. toctree::
   :maxdepth: 1
   :caption: Tutorials:

   chapter1
   chapter2
   chapter3
