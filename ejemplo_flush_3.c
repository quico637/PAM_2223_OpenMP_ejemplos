#include <omp.h>
#include <stdio.h>

int main()
{
        int data, data2, res;
        int flag=0;
        int flag2=0;
        int iam;

        #pragma omp parallel sections num_threads(3)  private(iam)
        {
                #pragma omp section
                {
                        iam=omp_get_thread_num();
                        printf("Thread %d, esperando dato: \n",iam);
                        scanf("%d",&data);
                        scanf("%d",&data2);

                        #pragma omp flush(data)
                        #pragma omp flush(data2)
			       flag = 1;
                        #pragma omp flush(flag)
		}


		#pragma omp section
                {
                        iam=omp_get_thread_num();
                        printf("ANTES: Thread %d, dato a procesar = %d \n",iam,data);
                        printf("ANTES: Thread %d, dato2 a procesar = %d \n",iam,data2);
                        printf("ANTES: Thread %d, resultado = %d \n",iam,res);

                        while (!flag)
                        {
                                #pragma omp flush(flag)
                        }

                        #pragma omp flush(data)
                        #pragma omp flush(data2)

                        res = data + data2;
                        #pragma omp flush(res)
                        flag2 = 1;


                        // do more work with data
                }

                #pragma omp section
                {
                        iam=omp_get_thread_num();
                        while (!flag2)
                        {
                                #pragma omp flush(flag2)
                        }

                        printf("DESPUES: Thread %d, dato a procesar = %d \n",iam,data);
                        printf("DESPUES: Thread %d, dato2 a procesar = %d \n",iam,data2);
                        printf("DESPUES: Thread %d, resultado = %d \n",iam,res);
                }


        } //parallel
}


