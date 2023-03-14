#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <assert.h>

// sec_mult_mat_cua_bloq.c

double **A;
double **B;
double **C;

// #define DEBUG
#define TEST
#define GDB

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

void mult_submatrix(double *a, double *b, double *c, int n, int block_i, int block_j, int block_k, int block_size, int iam)
{
  int i, j, k;

#ifdef GDB
  printf("Thread ID: %d, block_i: %d, block_j: %d, block_k: %d\n--------------\n\n", iam, block_i, block_j, block_k);
#endif

  for (i = block_i * block_size; i < block_i * block_size + block_size; i++)
  {
    for (j = block_j * block_size; j < block_j * block_size + block_size; j++)
    {
      for (k = block_k * block_size; k < block_k * block_size + block_size; k++)
      {
        c[i * n + j] += a[i * n + k] * b[k * n + j];
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
void multiply_matrix(double *a, int fa, int ca, int lda, double *b, int fb, int cb, int ldb, double *c, int fc, int cc, int ldc, int block_size)
{
  int i, j, k, iam, nprocs;
  double s;

  assert(ca == fb);

  int n = ca;
  int num_blocks = n / block_size;

  // recorro bloques y computo bloque

#pragma omp parallel private(iam, i, j, k)
  {
#if defined(_OPENMP)
    iam = omp_get_thread_num();
#endif

#pragma omp for
    for (i = 0; i < num_blocks; i++)
    {
      for (j = 0; j < num_blocks; j++)
      {
        for (k = 0; k < num_blocks; k++)
          mult_submatrix(a, b, c, n, i, j, k, block_size, iam);
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

  printf("\n--------------\n\n");
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
  printf("\n--------------\n\n");
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

  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < n; j++)
    {
      assert(global[i][j] == m[i * n + j]);
    }
  }
  printf("OK\n");
}

///////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  int t, block_size, threads;
  double start, fin, tiempo, Mflops;
  double *a, *b, *c;

  if (argc < 4)
  {
    printf("\n\n USO %s <dim_mat_n> <tam_blo_b> <num_threads>\n\n", argv[0]);
    return -1;
  }

  t = atoi(argv[1]); // TAM
  block_size = atoi(argv[2]);
  threads = atoi(argv[3]);

  assert(t % block_size == 0);

  a = (double *)malloc(sizeof(double) * t * t); // inicializar matriz de manera unidimensional
  b = (double *)malloc(sizeof(double) * t * t);
  c = (double *)calloc(t * t, sizeof(double));
  initializealea(a, t * t);
  initializealea(b, t * t);
  // initialize_zero(c, t * t);

  omp_set_num_threads(threads);

  start = omp_get_wtime();
  multiply_matrix(a, t, t, t, b, t, t, t, c, t, t, t, block_size);
  fin = omp_get_wtime();

#ifdef TEST
  // GLOBAL
  initialize_global(t);
  copy(a, A, t);
  copy(b, B, t);

  multiplicar(t);

#ifdef DEBUG
  escribir(c, t, t, t);
  escribir_global(C, t, t, t);
#endif

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
    printf("%d;%d;%d;%.2lf\n", threads, t, block_size, tiempo);
  }
  free(a);
  free(b);
  free(c);
}
