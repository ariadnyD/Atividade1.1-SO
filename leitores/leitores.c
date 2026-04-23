#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

int readcount = 0;

sem_t mutex;
sem_t wrt;

int shared = 0;

void * leitor(void *);
void * escritor(void *);
int gera_rand(int);

int main(int argc, char ** argv)
{

    pthread_t * tl;
    pthread_t * te;

    long i;

    srand(time(NULL));

    if ( argc < 3 )
    {
        printf("Usage: %s num_leitores num_escritores\n", argv[0]);
        return 0;
    }

    sem_init(&mutex, 0, 1);
    sem_init(&wrt, 0, 1);
 
    int N_LEITORES = atoi(argv[1]);
    int N_ESCRITORES = atoi(argv[2]);
    
    tl = malloc(N_LEITORES * sizeof(pthread_t));

    for (i = 0; i < N_LEITORES; i++)
    {
        pthread_create(&tl[i], NULL, leitor, (void *)i);
    }
    
    te = malloc(N_ESCRITORES * sizeof(pthread_t));

    for (i = 0; i < N_ESCRITORES; i++)
    {
        pthread_create(&te[i], NULL, escritor, (void *)i);
    }

    for (i = 0; i < N_LEITORES; i++)
    {
        pthread_join(tl[i], NULL);
    }
    
    for (i = 0; i < N_ESCRITORES; i++)
    {
        pthread_join(te[i], NULL);
    }


    sem_destroy(&mutex);
    sem_destroy(&wrt);

    free(tl);
    free(te);

    return 0;
}

void * leitor(void * id)
{
    usleep(gera_rand(1000000));

    long i = (long)id;
    int shared_in;

    printf("> Leitor %ld tentando acesso\n",i);

    sem_wait(&mutex);
    readcount++;
    if (readcount == 1) {
        sem_wait(&wrt);
    }
    sem_post(&mutex);
    
    printf("> Leitor %ld conseguiu acesso\n",i);

    printf("\t> Leitor %ld acessando\n", i);
    shared_in = shared;
    usleep(gera_rand(1000000));

    printf("\t> Leitor %ld - tmp: %d - shared: %d - readcount: %d\n", i, shared_in, shared, readcount);

    if (shared_in != shared)
    {
        printf("\t==== ALERTA DO LEITOR ====\n");
        printf("\t> Valor interno diferente do compartilhado\n\tshared_in: %d - shared: %d\n", shared_in, shared);
        printf("\t==========================\n");
    }

    printf("< Leitor %ld liberando acesso\n",i);
    
    sem_wait(&mutex);
    readcount--;
    if (readcount == 0) {
        sem_post(&wrt);
    }
    sem_post(&mutex);

    return NULL;
}

void * escritor(void * id)
{
    usleep(gera_rand(1000000));

    long i = (long)id;
    printf("+ Escritor %ld tentando acesso\n",i);

    sem_wait(&wrt);
    printf("\t+ Escritor %ld conseguiu acesso\n",i);

    if (readcount > 0)
    {
        printf("==== ALERTA DO ESCRITOR ====\n");
        printf("Readcount possui valor: %d\n",readcount);
        printf("============================\n");
    }

    int rnd = gera_rand(100);
    printf("\t+ Escritor %ld gravando o valor %d em shared\n", i, rnd);
    usleep(gera_rand(1000000));
    shared = rnd;
    
    printf("+ Escritor %ld saindo\n",i);
    sem_post(&wrt);

    return NULL;
}

int gera_rand(int limit)
{
    return rand()%limit;
}