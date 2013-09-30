#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>

typedef struct argumento {
  int id;
  int local;
} argumento;

sem_t A;
sem_t B;
sem_t mutex;
sem_t lock;

int W = 40;
int H = 10;

volatile int vez;
volatile int a;
volatile int b;

volatile int *x;
volatile int N;
volatile int na;
volatile int reqa;
volatile int nb;
volatile int reqb;

int verifica_ocorrencia(int n) {
  int i;
  for (i=0; i<N; i++)
    if (x[i] == n)
      return 1;
  return 0;
}

void desenha_tela() {
  int i, j;
  printf("\nLocal A");
  for (i=0; i<W-6; i++)
    printf(" ");
  printf("Local B\n");
  printf("   |");
  for (i=0; i<W; i++)
    printf("-");
  printf("|\n");
  printf("   |");
  for (i=0; i<W; i++)
    if (verifica_ocorrencia(i))
      printf("*");
    else
      printf(" ");
  printf("|\n");
  for (i=0; i<H; i++) {
    printf("   |");
    for (j=0; j<W; j++)
      printf(" ");
    printf("|\n");
  }
  printf("|-----|");
  for (i=0; i<W-6; i++)
    printf(" ");
  printf("|-----|\n"); 
  printf("Número total de macacos em cada ponto:\n");
  printf("   %d", na);
  for (i=0; i<W; i++) 
    printf(" ");
  printf("%d\n", nb);
    printf("Número de macacos que querem atravessar:\n");
  printf("   %d", reqa);
  for (i=0; i<W; i++) 
    printf(" ");
  printf("%d\n", reqb);
  printf("Número macacos que conseguiram atravessar:\n");
  printf("   %d", na-reqa);
  for (i=0; i<W; i++) 
    printf(" ");
  printf("%d\n", nb-reqb);
}

void* f_baboon(void *v) {
  argumento arg = *((argumento *)(v));
  int i;
  int qnt_a, qnt_b;

  if (vez == -1) {
    vez = arg.local;
    if (arg.local == 0) {
    sem_wait(&mutex);
      a++;
      sem_post(&A);
    sem_post(&mutex);
    }
    else {
    sem_wait(&mutex);
      b++;
      sem_post(&B);
    sem_post(&mutex);
    }
  }
  else {
    if (arg.local == 0) {
      sem_wait(&A); /* macaco(s) espera(m) enquanto a vez não é dele(s) */
      sem_wait(&mutex);
        a++;
        if (a < 5)
          sem_post(&A);
      sem_post(&mutex);
    }
    else {
      sem_wait(&B);
      sem_wait(&mutex);
        b++;
        if (b < 5)
          sem_post(&B);
      sem_post(&mutex);
    }
  }

  /* Região crítica, inicio */
  
  sem_wait(&lock);
    if (vez == 0) {
      reqa--;
      na--;
   }
    else {
      reqb--;
      nb--;
  }
  sem_post(&lock);

  for (i=0; i<W; i++) {
    sleep(random()%3);
    sem_wait(&lock);
      desenha_tela();
      if (vez == 0)
        x[arg.id]++;
      else
        x[arg.id] = W-i;
    sem_post(&lock);
  }

  x[arg.id] = -1;

  sem_wait(&lock);
    if (vez == 0)
      nb++;
    else
      na++;
    desenha_tela();
  sem_post(&lock);
  
  /* Região crítica, fim */

  sem_wait(&mutex);
    if (arg.local == 0) {
      a--;
      arg.local = 1;
      if (a == 0) {
        sem_getvalue(&B, &qnt_b);
        if (qnt_b <= 0) { /* há threads B esperando */
         vez = 1;
         sem_post(&B);
        }
        else
         sem_post(&A);
      }
      else
        sem_post(&A);
    }
    else {
      b--;
      arg.local = 0;
      if (b == 0) {
        sem_getvalue(&A, &qnt_a);
        if (qnt_a <= 0) { /* há threads A esperando */
         vez = 0;
         sem_post(&A);
        }
        else
         sem_post(&B);
      }
      else
        sem_post(&B);
    }
  sem_post(&mutex);

  return NULL;
}

int main() {
  int i;
  pthread_t *thr;
  argumento *arg;
  
  printf("Insira o número de macacos no ponto A: ");
  scanf("%d", &na);
  printf("Insira o número de macacas no ponto B: ");
  scanf("%d", &nb);
 
  reqa = na;
  reqb = nb;
  N = na+nb;
  thr = malloc(N*sizeof(pthread_t));
  arg = malloc(N*sizeof(argumento));
  x = malloc(N*sizeof(int));

  sem_init(&A, 0, 0);
  sem_init(&B, 0, 0);
  sem_init(&mutex, 0, 1);
  sem_init(&lock, 0, 1);
  
  vez = -1;
  a = 0;
  b = 0;

  for (i=0; i<N; i++)
    x[i] = -1;
  for (i=0; i<N; i++) {
    arg[i].id = i;
    if (i < na)
      arg[i].local = 0;
    else
      arg[i].local = 1;
    pthread_create(&thr[i], NULL, f_baboon, (void*) &arg[i]);
  }  
  for (i = 0; i < N; i++) 
    pthread_join(thr[i], NULL);

  return 0;
}
