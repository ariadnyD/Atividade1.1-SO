#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_FILEIRAS 38
#define NUM_COLUNAS 6
#define NUM_ASSENTOS (NUM_FILEIRAS * NUM_COLUNAS)

// disposicao dos assentos
char assentos[NUM_FILEIRAS][NUM_COLUNAS];

// alocacao dos passageiros
int passageiros[NUM_FILEIRAS * NUM_COLUNAS];

char colunas[NUM_COLUNAS] = {'A', 'B', 'C', 'D', 'E', 'F'};

// Variáveis de controle para Leitores e Escritores
sem_t mutex;
sem_t wrt;
int readcount = 0;

void print_assentos()
{
  printf("A B C    D E F\n");

  for (int i = 0; i < NUM_FILEIRAS; i++)
  {
    for (int j = 0; j < NUM_COLUNAS; j++)
    {
      printf("%c ", assentos[i][j]);

      if (j == 2)
      {
        printf("%2d ", i+1);
      }
    }

    printf("\n");
  }
}

void print_passageiros()
{
  int fileira;
  int coluna;
  
  for (int i = 0; i < NUM_FILEIRAS * NUM_COLUNAS; i++)
  {
    fileira = passageiros[i] / NUM_COLUNAS;
    coluna = passageiros[i] % NUM_COLUNAS;

    printf("Passageiro %3d - ", i+1);
    if (passageiros[i] >= 0)
    {
        printf("%d%c", fileira+1, colunas[coluna]);
    }
    else
    {
        printf("-");
    }
    printf("\n");
  }
}

void print_stats()
{
  printf("Total de Assentos: %d\n", NUM_ASSENTOS);
  
  printf("    Assentos Disponíveis: ");
  int num_disp = 0;
  for (int i = 0; i < NUM_FILEIRAS; i++)
  {
      for (int j = 0; j < NUM_COLUNAS; j++)
      {
          if (assentos[i][j] == '-')
          {
              printf("%d%c ", i+1, colunas[j]);
              num_disp++;
          }
      }
  }

  if (num_disp > 0)
    printf("\n");
  else
    printf("--\n");
  
  printf("Total de Assentos Disponíveis: %d\n", num_disp);

  printf("    Assentos Duplicados: ");
  int num_dup = 0;
  int assentos_ocup[NUM_ASSENTOS];
  memset(assentos_ocup, 0, sizeof(assentos_ocup));
  for (int i = 0; i < NUM_ASSENTOS; i++)
  {
    if (passageiros[i] >= 0)
    {
      assentos_ocup[passageiros[i]]++;

      if (assentos_ocup[passageiros[i]] > 1)
      {
        printf("%d%c ", 
                passageiros[i] / NUM_COLUNAS + 1,
                colunas[passageiros[i] % NUM_COLUNAS]);
        num_dup++;
      }
    }
  }
  
  if (num_dup > 0)
    printf("\n");
  else
    printf("--\n");

  printf("Total de Assentos Duplicados: %d\n", num_dup);

}

int consultar_assento(int n)
{
  int fileira = n / NUM_COLUNAS;
  int coluna = n % NUM_COLUNAS;

  if (assentos[fileira][coluna] == '-')
  {
    printf("Assento %d%c disponível.\n", fileira+1, colunas[coluna]);
    return 1;
  }
  else
  {
    printf("Assento %d%c ocupado.\n", fileira+1, colunas[coluna]);
    return 0;
  }
}

void reservar_assento(int n)
{
  int fileira = n / NUM_COLUNAS;
  int coluna = n % NUM_COLUNAS;
  assentos[fileira][coluna] = 'X';
}

void * passageiro(void * id)
{
  int p = * (int *) id;
  int assento;
  int status;
  int fileira;
  int coluna;

  // o passageiro nao finaliza enquanto nao conseguir um assento
  while (passageiros[p] == -1)
  {
      assento = rand() % NUM_ASSENTOS; // 0 - NUM_ASSENTOS-1
      status = rand() % 2;             // 0 - consulta / 1 - reserva 

      fileira = assento / NUM_COLUNAS;
      coluna = assento % NUM_COLUNAS;

      if (status == 0)
      {
          // === COMPORTAMENTO DO LEITOR ===
          sem_wait(&mutex);
          readcount++;
          if (readcount == 1) {
              sem_wait(&wrt); // Se for o primeiro leitor, bloqueia os escritores
          }
          sem_post(&mutex);

          printf("Passageiro %d - Consultando assento: %d%c [%d]\n", 
                  p+1, fileira+1, colunas[coluna], assento);
          consultar_assento(assento);

          sem_wait(&mutex);
          readcount--;
          if (readcount == 0) {
              sem_post(&wrt); // Se for o último leitor, libera os escritores
          }
          sem_post(&mutex);
      }
      else
      {
          // === COMPORTAMENTO DO ESCRITOR ===
          sem_wait(&wrt); // Pede acesso exclusivo antes de verificar e alterar o assento

          printf("Passageiro %d - Reservando assento: %d%c [%d]\n", 
                  p+1, fileira+1, colunas[coluna], assento);

          if ( consultar_assento(assento) )
          {
              reservar_assento(assento);
              passageiros[p] = assento;
          }

          sem_post(&wrt); // Libera o acesso exclusivo
      }
  }
  
  return NULL;
}

int main()
{
  // iniciando a semente aleatoria do random
  srand(time(NULL));

  // Inicializando os semáforos
  sem_init(&mutex, 0, 1);
  sem_init(&wrt, 0, 1);

  // inicializando os assentos com assentos disponivel: -
  memset(assentos, '-', sizeof(assentos));
  memset(passageiros, -1, sizeof(passageiros));

  // vetor threads dos passageiros
  pthread_t * tids = malloc(NUM_ASSENTOS * sizeof(pthread_t));
  
  int *ids = malloc(NUM_ASSENTOS * sizeof(int));

  // iniciando as threads dos passageiros
  for (int i = 0; i < NUM_ASSENTOS; i++)
  {
      ids[i] = i;
      pthread_create(&tids[i], NULL, passageiro, &ids[i]);
  }

  // aguardando todas as threads finalizarem
  for (int i = 0; i < NUM_ASSENTOS; i++)
  {
      pthread_join(tids[i], NULL);
  }

  free(ids);
  free(tids);

  // Destruindo os semáforos
  sem_destroy(&mutex);
  sem_destroy(&wrt);

  // finalizando
  print_assentos();
  print_passageiros();
  print_stats();

  return 0;
}