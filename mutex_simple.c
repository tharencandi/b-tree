#include <stdio.h>
#include <string.h>
#include <pthread.h>

char *message = "Chocolate microscopes?";
int mindex = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
size_t message_len = 0;

pthread_t my_thread;

void* threadF(void *arg) {
  while (1) {
    pthread_mutex_lock(&lock);
    if (mindex < message_len) {
      printf("%c", message[mindex]);
      mindex++;
    } else {
      pthread_mutex_unlock(&lock);
      break;
    }
    pthread_mutex_unlock(&lock);
  }
  printf("\n");
  printf("Thread end at %d.\n", mindex);
  return NULL;
}

int main(void) {
  message_len = strlen(message);
  pthread_create(&my_thread,NULL,threadF,NULL);

  while (1) {
    pthread_mutex_lock(&lock);
    if (mindex < message_len) {
      printf("%c", message[mindex]);
      mindex++;
    } else {
        pthread_mutex_unlock(&lock);
        break;
    }
    pthread_mutex_unlock(&lock);
  }
  printf("main end at %d.\n", mindex);
  printf("\n");
  pthread_join(my_thread, NULL);
  printf("\n");
  printf("all threads ended: %d.\n", mindex);
  return 0;
}