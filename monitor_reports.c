#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define PID_FILE ".monitor_pid"

void gestionare_semnale(int semnal)
{
  if(semnal==SIGUSR1)
    {
      printf("s-a adaugat un nou raport in sistem\n");
    }
  else if(semnal==SIGINT)
    {
      printf("oprire monitor_reports\n");
      unlink(PID_FILE);
      exit(0);
    }
}

int main(void)
{
  FILE* file=fopen(PID_FILE, "w");
  if(file==NULL)
    {
      printf("eroare la deschiderea fisierului\n");
      return 1;
    }
  fprintf(file, "%d", getpid());
  fclose(file);

  struct sigaction sa;
  sa.sa_handler=gestionare_semnale;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags=0;

  if(sigaction(SIGUSR1, &sa, NULL)==-1)
    {
      printf("nu s-a putut configura SIGUSR1\n");
      return 1;
    }

  if(sigaction(SIGINT, &sa, NULL)==-1)
    {
      printf("nu s-a putut configura SIGINT\n");
      return 1;
    }
  
  printf("monitor activat. PID: %d\n", getpid());


  for(;;);
  
  return 0;
}
