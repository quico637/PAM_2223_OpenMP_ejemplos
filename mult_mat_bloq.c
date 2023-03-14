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
// #define GDB

void multiplicar(int m, int n, int kk)
{
  for (int i = 0; i < m; i++)
  {
    for (int j = 0; j < n; j++)
    {
      C[i][j] = 0.;
      for (int k = 0; k < kk; k++)
      {
        C[i][j] += A[i][k] * B[k][j];
      }
    }
  }
}

///////////////////////////////////////////////////////////////

void mult_submatrix(double *a, double *b, double *c, int block_i, int block_j, int block_k, int block_size, int m1, int n2, int n1)
{
  int i, j, k;

  for (i = block_i * block_size; i < block_i * block_size + block_size; i++)
  {
    for (j = block_j * block_size; j < block_j * block_size + block_size; j++)
    {
      for (k = block_k * block_size; k < block_k * block_size + block_size; k++)
      {
#ifdef GDB
        printf("row_a: %d\n", block_i * block_size);
        printf("col_b: %d\n\n", block_j * block_size);

        printf("col_a: %d\n", ii);
        printf("row_b: %d\n", jj);
        printf("block_i: %d\n", block_i);
        printf("block_j: %d\n", block_j);
        printf("-----\n\n");
#endif
        c[i * n2 + j] += a[i * n1 + k] * b[k * n2 + j];
      }
#ifdef GDB
      printf("\t-----\n\n");
#endif
    }
  }
#ifdef GDB
  printf("FIN\n\n");
#endif
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

  int num_blocks_fa = fa / block_size;
  int num_blocks_cb = cb / block_size;
  int num_blocks_ca = ca / block_size;

#pragma omp parallel private(iam, i, j, k)
  {

#if defined(_OPENMP)
    iam = omp_get_thread_num();
#endif

#pragma omp for
    for (i = 0; i < num_blocks_fa; i++)
    {
      for (j = 0; j < num_blocks_cb; j++)
      {
        for (k = 0; k < num_blocks_ca; k++)
          mult_submatrix(a, b, c, i, j, k, block_size, fa, cb, ca);
      }
    }
  }
}

//////////

void initialize_global(int m, int n, int k)
{
  A = (double **)malloc(sizeof(double *) * m);
  B = (double **)malloc(sizeof(double *) * k);
  C = (double **)malloc(sizeof(double *) * m);

  for (int i = 0; i < m; i++)
  {
    A[i] = (double *)malloc(sizeof(double) * k);
    C[i] = (double *)malloc(sizeof(double) * n);
  }

  for (int i = 0; i < k; i++)
    B[i] = (double *)malloc(sizeof(double) * n);
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
void escribir(double *m, int fm, int cm)
{

  int i, j;
  int ldm = fm;
  for (i = 0; i < fm; i++)
  {
    for (j = 0; j < cm; j++)
      printf("%.4lf ", m[i * ldm + j]);
    printf("\n");
  }

  printf("\n--------------\n\n");
}

void escribir_global(double **global, int fm, int cm)
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

void copy(double *source, double **dest, int t1, int t2)
{
  for (int i = 0; i < t1; i++)
  {
    for (int j = 0; j < t2; j++)
    {
      dest[i][j] = source[i * t2 + j];
    }
  }
}

int test(double *m, double **global, int t1, int t2)
{

  for (int i = 0; i < t1; i++)
  {
    for (int j = 0; j < t2; j++)
    {
      assert(global[i][j] == m[i * t2 + j]);
    }
  }
  printf("OK\n");
}

///////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  int m, n, k, block_size, threads;
  double start, fin, tiempo, Mflops;
  double *a, *b, *c;

  if (argc < 6)
  {
    printf("\n\n USO %s <dim_mat_m> <dim_mat_n> <dim_mat_k> <tam_blo_b> <num_threads>\n\n", argv[0]);
    return -1;
  }

  m = atoi(argv[1]);
  n = atoi(argv[2]);
  k = atoi(argv[3]);
  block_size = atoi(argv[4]);
  threads = atoi(argv[5]);

  assert(m % block_size == 0);
  assert(n % block_size == 0);
  assert(k % block_size == 0);

  a = (double *)malloc(sizeof(double) * m * k); // inicializar matriz de manera unidimensional
  b = (double *)malloc(sizeof(double) * k * n);
  c = (double *)calloc(m * n, sizeof(double));

  initializealea(a, m * k);
  initializealea(b, k * n);

  omp_set_num_threads(threads);

  start = omp_get_wtime();
  multiply_matrix(a, m, k, k, b, k, n, n, c, m, n, n, block_size);
  fin = omp_get_wtime();

#ifdef TEST
  // GLOBAL
  initialize_global(m, n, k);
  copy(a, A, m, k);
  copy(b, B, k, n);

  multiplicar(m, n, k);

#ifdef DEBUG
  escribir(c, m, n);
  escribir_global(C, m, n);
#endif

  test(a, A, m, k);
  test(b, B, k, n);
  test(c, C, m, n);

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
#ifdef DEBUG
    printf("m;n;k;block_size;tiempo\n");
#endif
    printf("%d;%d;%d;%d;%.2lf\n", m, n, k, block_size, tiempo);
  }
  free(a);
  free(b);
  free(c);
}
