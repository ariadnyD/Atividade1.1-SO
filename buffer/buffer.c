#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

sem_t mutex;
sem_t espacos_vazios;
sem_t itens_disponiveis;

int * buffer;
int N_BUFFER;
int PROD_NUM;

int in = 0;
int out = 0;

void *producer(void *);
void *consumer();
int gera_rand(int);
void print_buffer();

int main(int argc, char ** argv)
{
    pthread_t tcons;
    pthread_t *nprod;
    long i;

    if ( argc != 3 )
    {
        printf("Usage: %s buffer_size num_producers\n", argv[0]);
        return 0;
    }

    srand(time(NULL));

    N_BUFFER = atoi(argv[1]);
    PROD_NUM = atoi(argv[2]);

    sem_init(&mutex, 0, 1);
    
    sem_init(&espacos_vazios, 0, N_BUFFER);
    
    sem_init(&itens_disponiveis, 0, 0);

    buffer = malloc(N_BUFFER * sizeof(int));

    for (i = 0; i < N_BUFFER; i++)
    {
        buffer[i] = -1;
    }

    nprod = malloc(PROD_NUM * sizeof(pthread_t));

    pthread_create(&tcons, NULL, consumer, NULL);
    for (i = 0; i < PROD_NUM; i++)
    {
        pthread_create(&nprod[i], NULL, producer, (void *)i);
    }
    
    for (i = 0; i < PROD_NUM; i++)
    {
        pthread_join(nprod[i], NULL);
    }
    pthread_join(tcons, NULL);

    sem_destroy(&mutex);
    sem_destroy(&espacos_vazios);
    sem_destroy(&itens_disponiveis);

    free(buffer);
    free(nprod);

    return 0;
}

void * consumer()
{
    usleep(gera_rand(1000000));
    int produto;
    int i = 0;

    while (i != PROD_NUM)
    {
        printf("- Consumidor esperando por recurso!\n");
        sem_wait(&itens_disponiveis);
        sem_wait(&mutex);

        printf("- Consumidor entrou em ação!\n");
        printf("\t- Consumidor vai limpar posição %d\n", out);

        produto = buffer[out];
        printf("\t- Consumiu o valor: %d\n",produto);
        
        if (buffer[out] == -1)
        {
            printf("==== ALERTA DO CONSUMIDOR ====\n");
            printf("Posicao %d estava vazia\n", out);
            printf("==============================\n");
        }
        
        buffer[out] = -1;
        out = (out + 1) % N_BUFFER;

        sem_post(&mutex);
        sem_post(&espacos_vazios);

        i++;
    }

    return NULL;
}

void * producer(void * id)
{
    usleep(gera_rand(1000000));
    long i = (long)id;
    int produto;

    printf("> Produtor %ld esperando por recurso!\n", i);

    sem_wait(&espacos_vazios);

    sem_wait(&mutex);

    printf("> Produtor %ld entrou em ação!\n", i);

    produto = gera_rand(100);

    if (buffer[in] != -1)
    {
        printf("==== ALERTA DO PRODUTOR %ld ====\n", i);
        printf("Posicao %d ocupada com o valor %d\n", in, buffer[in]);
        printf("===============================\n");
    }

    printf("\t> Produtor %ld vai gravar o valor %d na pos %d\n", i, produto, in);
    buffer[in] = produto;
    
    in = (in + 1) % N_BUFFER;

    sem_post(&mutex);

    sem_post(&itens_disponiveis);
    
    return NULL;
}

int gera_rand(int limit)
{
    return rand() % limit;
}

void print_buffer()
{
    int i;
    printf("\t== BUFFER ==\n");
    for (i = 0; i < N_BUFFER; i++)
    {
        printf("\ti: %d | v: %d\n", i, buffer[i]);
    }
    printf("\n");
}