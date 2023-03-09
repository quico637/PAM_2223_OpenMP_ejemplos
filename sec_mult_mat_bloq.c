#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <assert.h>

// sec_mult_mat_cua_bloq.c

double **A;
double **B;
double **C;

#define DEBUG
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

void mult_submatrix(double *a, int fa, int ca, int lda, double *b, int fb, int cb, int ldb, double *c, int fc, int cc, int ldc, int block_i, int block_j, int block_k, int block_size)
{
  int ii, jj, kk;

  for (ii = block_i * block_size; ii < block_i * block_size + block_size; ii++)
  {
    for (jj = block_j * block_size; jj < block_j * block_size + block_size; jj++)
    {
      for (kk = block_k * block_size; kk < block_k * block_size + block_size; kk++)
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
        c[ii * ldc + jj] += a[ii * lda + kk] * b[kk * ldb + jj];
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

  // Ver si se pueden multiplicar las matrices
  assert(ca == fb);
  int n = ca;

  int num_blocks_rows = fa / block_size;
  int num_blocks_cols = cb / block_size;
  int num_blocks = n / block_size;

  // recorro bloques y computo bloque
  for (i = 0; i < num_blocks_rows; i++)
  {
    for (j = 0; j < num_blocks_cols; j++)
    {
      for (k = 0; k < num_blocks; k++)

        mult_submatrix(a, fa, ca, lda, b, fb, cb, ldb, c, fc, cc, ldc, i, j, k, block_size);
    }
  }
}

//////////

void initialize_global(int m, int n, int k)
{
  A = (double **)malloc(sizeof(double *) * m);
  B = (double **)malloc(sizeof(double *) * k);
  C = (double **)malloc(sizeof(double *) * m);

  for (int i = 0; i < n; i++)
  {
    A[i] = (double *)malloc(sizeof(double) * k);
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
      dest[i][j] = source[i * t1 + j];
    }
  }
}

int test(double *m, double **global, int t1, int t2)
{

  for (int i = 0; i < t1; i++)
  {
    for (int j = 0; j < t2; j++)
    {
      assert(global[i][j] == m[i * t1 + j]);
    }
  }
  printf("OK\n");
}

///////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  int m, n, k, block_size;
  double start, fin, tiempo, Mflops;
  double *a, *b, *c;
  
  printf("sjdnsdj");

  if (argc < 5)
  {
    printf("\n\n USO %s <dim_mat_m> <dim_mat_n> <dim_mat_k> <tam_blo_b>\n\n", argv[0]);
    return -1;
  }

  m = atoi(argv[1]);
  n = atoi(argv[2]); 
  k = atoi(argv[3]); 
  block_size = atoi(argv[4]);

  printf("sjdnsdj");

  m = 4;
  n = 3;
  k = 2;
  block_size = 1;

  assert(m % block_size == 0);
  assert(n % block_size == 0);
  assert(k % block_size == 0);

  a = (double *)malloc(sizeof(double) * m * k); // inicializar matriz de manera unidimensional
  b = (double *)malloc(sizeof(double) * k * n);
  c = (double *)calloc(m * n, sizeof(double));
  
  initializealea(a, m * k);
  initializealea(b, k * n);

  start = omp_get_wtime();
  multiply_matrix(a, m, k, m, b, k, n, k, c, m, n, m, block_size);
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
