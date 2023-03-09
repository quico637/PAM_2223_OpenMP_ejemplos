
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// ejemplo_fibo.c

#define THRESHOLD 5

///////////////////////////////////////////////////
long comp_fib_numbers(int n)
{
  long fnm1, fnm2, fn;
  int iam;

  iam = omp_get_thread_num();
  

  if (n == 0 || n == 1)
    return (1);

  if (n > THRESHOLD)
  {

    printf(" Soy el thread %d, resolviendo fibo (%d) - declarando mas tareas.\n", iam, n);
    
  
#pragma omp task shared(fnm1)
    {
      fnm1 = comp_fib_numbers(n - 1);
    }

#pragma omp task shared(fnm2)
    {
      fnm2 = comp_fib_numbers(n - 2);
    }

#pragma omp taskwait
    fn = fnm1 + fnm2;
  }
  else
  {
    printf(" Soy el thread %d, resolviendo fibo (%d) - YO MISMO\n", iam, n);
    fnm1 = comp_fib_numbers(n - 1);
    fnm2 = comp_fib_numbers(n - 2);

    fn = fnm1 + fnm2;
  }

  return (fn);
}

///////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  int n = 8;
  int nthreads;
  long result;
  int iam;

  if (argc > 1)
    n = atoi(argv[1]);

  omp_set_num_threads(12);

#pragma omp parallel shared(nthreads)
  {
    iam = omp_get_thread_num();
    // printf("Soy el thread %d\n", iam);

#pragma omp single nowait
    {
      result = comp_fib_numbers(n);
    } // End of single
  }   // End of parallel region

  printf("Serie de Fibonacci para n=%d. El resultado es %ld \n", n, result);
  return 0;
}