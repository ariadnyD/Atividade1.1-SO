#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

int *chopstick_use;

int N_FILOS;

sem_t *chopsticks_sem;

void * filosofo(void *);
void pegar(int, int);
void liberar(int, int);
int gera_rand(int);

int main(int argc, char ** argv)
{
    pthread_t * tids;
    long i;

    srand(time(NULL));

    if ( argc < 2 )
    {
        printf("Usage: %s num_filosofos\n", argv[0]);
        return 0;
    }
    
    N_FILOS = atoi(argv[1]);
    
    tids = malloc(N_FILOS * sizeof(pthread_t));
    chopstick_use = malloc(N_FILOS * sizeof(int));
    
    chopsticks_sem = malloc(N_FILOS * sizeof(sem_t));
    
    for (i = 0; i < N_FILOS; i++)
    {
        chopstick_use[i] = -1;
        sem_init(&chopsticks_sem[i], 0, 1);
    }

    for (i = 0; i < N_FILOS; i++)
    {
        pthread_create(&tids[i], NULL, filosofo, (void *)i);
    }
    
    for (i = 0; i < N_FILOS; i++)
    {
        pthread_join(tids[i], NULL);
    }
    
    for (i = 0; i < N_FILOS; i++)
    {
        sem_destroy(&chopsticks_sem[i]);
    }

    free(tids);
    free(chopstick_use);
    free(chopsticks_sem);

    return 0;
}

void * filosofo(void * id)
{
    long i = (long)id;
    
    printf("\t> Filosofo %ld pensando\n",i);
    usleep(gera_rand(1000000));

    int c1, c2;

    if (i % 2 == 0) 
    {
        c1 = i;             // esquerda
        c2 = (i+1) % N_FILOS; // direita
    }
    else
    {
        c1 = (i+1) % N_FILOS; // direita
        c2 = i;             // esquerda 
    }

    pegar(i, c1);
    pegar(i, c2);
    
    printf("\t> Filosofo %ld comendo\n", i);
    usleep(gera_rand(1000000));
    
    liberar(i, c1);
    liberar(i, c2);
    
    return NULL;
}

void pegar(int i, int num)
{
    sem_wait(&chopsticks_sem[num]);

    if (chopstick_use[num] != -1)
    {
        printf("===== ALERTA DO FILOSOFO %d =====\n===== CHOPSTICK[%d] EM USO POR %d =====\n", i, num, chopstick_use[num]);
    }
    
    chopstick_use[num] = i;
    printf("+ Filosofo %d pegou o chopstick[%d]\n", i, num);
}

void liberar(int i, int num)
{
    chopstick_use[num] = -1;
    printf("- Filosofo %d liberou o chopstick[%d]\n", i, num);
    sem_post(&chopsticks_sem[num]);
}

int gera_rand(int limit)
{
    return rand() % limit;
}