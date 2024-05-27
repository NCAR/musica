##############
Model Inteface
##############

Fortran Interface Example
-------------------------

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
