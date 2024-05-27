##############
Model Inteface
##############

Fortran C Interface Example
---------------------------

.. code-block:: cpp

  #include <stdio.h>

  void test_proc_c(int n, double A[3][2]) {

      printf("test_proc_c\n");
      printf("n = %d\n", n);

      printf("matrix A\n");

      for (int i = 0; i < 2; i++) {
          for (int j = 0; j < 3; j++) {
              printf("%6.2f ", A[j][i]);
          }
          printf("\n");
      }
  }


.. code-block:: f90

  program demo_fort
      use iso_c_binding, only: c_int, c_double
      implicit none
      integer :: i, j
      integer(c_int) :: n_fort = 7
      real(c_double), dimension(2, 3) :: A_fort

      interface
          subroutine test_proc_c(n_c, A_c) bind(C, name='test_proc_c')
              use iso_c_binding, only: c_int, c_double
              integer(c_int), intent(in), value :: n_c
              real(c_double), dimension(2, 3), intent(in) :: A_c
          end subroutine test_proc_c
      end interface

      do j = 1, 3
          do i = 1, 2
              A_fort(i, j) = real(i + j, c_double)
          end do
      end do

      call test_proc_c(n_fort, A_fort)
  end program demo_fort

.. code-block:: bash

    all: test_proc_c.o demo_fort.o demo_fort

    test_proc_c.o : test_proc_c.c
        gcc -c test_proc_c.c

    demo_fort.o : demo_fort.f90
        gfortran -c demo_fort.f90

    demo_fort : test_proc_c.o demo_fort.o
        gfortran -o demo_fort demo_fort.o test_proc_c.o -lc

    clean:
        rm -f test_proc_c.o demo_fort.o demo_fort

.. code-block:: console

  $ ./demo_fort
  test_proc_c
  n = 7
  matrix A
    2.00   3.00   4.00 
    3.00   4.00   5.00

