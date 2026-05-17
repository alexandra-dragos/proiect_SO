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
      printf("INFO:s-a adaugat un nou raport in sistem\n");
      fflush(stdout);
    }
  else if(semnal==SIGINT)
    {
      printf("INFO:oprire monitor_reports\n");
      fflush(stdout);
      unlink(PID_FILE);
      exit(0);
    }
}

int main(void)
{
  //verificare daca mai ruleaza deja alt monitor
  FILE* file_verificare_monitor=fopen(PID_FILE, "r");
  if(file_verificare_monitor!=NULL)
    {
      int pid_vechi;
      if(fscanf(file_verificare_monitor, "%d", &pid_vechi)==1)
	{
	  if(kill(pid_vechi, 0)==0)
	    {
	      printf("EROARE: monitorul deja ruleaza cu PID-ul: %d\n", pid_vechi);
	      fflush(stdout);
	      fclose(file_verificare_monitor);
	      return 1;
	    }
	}
      fclose(file_verificare_monitor);
    }
  
  FILE* file=fopen(PID_FILE, "w");
  if(file==NULL)
    {
      printf("EROARE:eroare la deschiderea fisierului\n");
      fflush(stdout);
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
      printf("INFO:nu s-a putut configura SIGUSR1\n");
      return 1;
    }

  if(sigaction(SIGINT, &sa, NULL)==-1)
    {
      printf("INFO:nu s-a putut configura SIGINT\n");
      return 1;
    }
  
  printf("INFO:monitor activat. PID: %d\n", getpid());
  fflush(stdout);

  for(;;);
  
  return 0;
}
