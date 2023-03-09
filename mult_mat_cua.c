// Introducción a la Programación Paralela
// capítulo 4: códigos 4.6 y 4.7
// multiplicación matriz-vector en OpenMP
// julio 2007

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <assert.h>
int THREADS;

double **A;
double **B;
double **C;

// #define DEBUG
// #define TEST

void multiplicar(int n)
{
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      C[i][j] = 0.;
      for (int k = 0; k < n; k++)
      {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

///////////////////////////////////////////////////////////////

/**
 * double *a = MATRIX A --> m
 * int fa = num rows A --> fm
 * int ca = num columns A --> cm
 * int lda = longitud columna a --> ldm
 *
 * double *b = MATRIX B --> v
 * int fb = num rows B --> fb
 * int cb = num columns B --> cm
 * int ldb = longitud columna B --> ldm
 *
 *
 * double *c = MATRIX C --> w
 * int fc = num rows C --> fw
 * int cc = num columns C --> cm
 * int ldc = longitud columna C --> ldm
 */
void multiply_matrix(double *a, int fa, int ca, int lda, double *b, int fb, int cb, int ldb, double *c, int fc, int cc, int ldc, int f)
{
  int i, j, k, iam, nprocs;
  double s;

  assert(ca == fb);

#pragma omp parallel private(iam, i, j, k)
  {

#if defined(_OPENMP)
    iam = omp_get_thread_num();
#endif

#pragma omp for private(s) schedule(static, f)
    for (i = 0; i < fa; i++)
    {
#ifdef DEBUG
      printf("thread %d fila %d \n", iam, i);
#endif
      for (j = 0; j < cb; j++)
      {
        s = 0.0;
        for (k = 0; k < ca; k++)
          s += a[i * lda + k] * b[k * ldb + j];

        c[i * ldc + j] = s;
      }
    }
  }
}

//////////

void initialize_global(int n)
{
  A = (double **)malloc(sizeof(double *) * n);
  B = (double **)malloc(sizeof(double *) * n);
  C = (double **)malloc(sizeof(double *) * n);

  for (int i = 0; i < n; i++)
  {
    A[i] = (double *)malloc(sizeof(double) * n);
    B[i] = (double *)malloc(sizeof(double) * n);
    C[i] = (double *)malloc(sizeof(double) * n);
  }
}

///////////////////////////////////////////////////////////////

void initialize(double *m, int t)
{
  int i;
  for (i = 0; i < t; i++)
    m[i] = (double)(i);
}

///////////////////////////////////////////////////////////////
void initializealea(double *m, int t)
{
  int i;
  for (i = 0; i < t; i++)
    m[i] = (double)rand() / RAND_MAX;
}
///////////////////////////////////////////////////////////////
void escribir(double *m, int fm, int cm, int ldm)
{
  int i, j;
  for (i = 0; i < fm; i++)
  {
    for (j = 0; j < cm; j++)
      printf("%.4lf ", m[i * ldm + j]);
    printf("\n");
  }
}

void escribir_global(double **global, int fm, int cm, int ldm)
{
  int i, j;
  for (i = 0; i < fm; i++)
  {
    for (j = 0; j < cm; j++)
      printf("%.4lf ", global[i][j]);
    printf("\n");
  }
}

///////

void copy(double *source, double **dest, int n)
{
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      dest[i][j] = source[i * n + j];
    }
  }
}

int test(double *m, double **global, int n)
{

#ifdef DEBUG_2
  escribir(m, n, n, n);
  printf("-----\n");
  escribir_global(global, n, n, n);
#endif

  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      assert(global[i][j] == m[i * n + j]);
    }

  }
  printf("OK\n");
#ifdef DEBUG_2
  printf("\t\t-----\n");
#endif
}

///////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  int t, f, threads;
  double start, fin, tiempo, Mflops;
  double *a, *b, *c;

  if (argc < 4)
  {
    printf("\n\n USO %s <dim_mat_n> <num_filas_consecutivas_F> <num_hil_t> \n\n", argv[0]);
    return -1;
  }

  t = atoi(argv[1]);    // TAM
  f = atoi(argv[2]);
  threads = atoi(argv[3]);

  a = (double *)malloc(sizeof(double) * t * t); // inicializar matriz de manera unidimensional
  b = (double *)malloc(sizeof(double) * t * t);
  c = (double *)malloc(sizeof(double) * t * t);
  initializealea(a, t * t);
  initializealea(b, t * t);

#ifdef DEBUG
  escribir(a, t, t, t);
  escribir(b, t, t, t);
#endif

  omp_set_num_threads(threads);

  start = omp_get_wtime();
  multiply_matrix(a, t, t, t, b, t, t, t, c, t, t, t, f);
  fin = omp_get_wtime();

#ifdef TEST
// GLOBAL
  initialize_global(t);
  copy(a, A, t);
  copy(b, B, t);

  multiplicar(t);
  test(a, A, t);
  test(b, B, t);
  test(c, C, t);
  
#endif

  tiempo = fin - start;
  if (tiempo == 0.)
  {
    printf("No hay suficiente precision\n");
  }
  else
  {
    // Mflops = ((2. * t * t) / tiempo) / 1000000.;
    // printf("  threads %d, tamano %d\n    segundos: %.6lf, Mflops: %.6lf, Mflops por thread: %.6lf\n", threads, t, tiempo, Mflops, Mflops / threads);
    // printf("  Precision omp_get_wtick: numero de segundos entre sucesivos ticks de reloj usados por wtime=%lf \n", omp_get_wtick());
    printf("%d;%d;%d;%.2lf\n", t, threads, f, tiempo);
  }
#ifdef DEBUG
  escribir(c, t, t, t);
#endif
  free(a);
  free(b);
  free(c);
}
