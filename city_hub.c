#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_DISTRICTS 30

int citeste_linia(int fd, char* buffer, int max_len)
{
  int i=0;
  char c;
  while(i<max_len-1)
    {
      int n=read(fd, &c, 1);
      if(n<=0)
	{
	  if(i==0)
	    return 0;
	  break;
	}
      buffer[i]=c;
      i++;
      if(c=='\n')
	break;
    }
  buffer[i]='\0';
  return i;
}

int main(void)
{
  char comanda[1024];

  while(1)
    {
      while(waitpid(-1, NULL, WNOHANG)>0);
      printf("city_hub ");
      fflush(stdout);

      if(fgets(comanda, sizeof(comanda),stdin)==NULL)
	{
	  break;
	}
      comanda[strcspn(comanda, "\n")]='\0';

      if(strcmp(comanda, "exit")==0)
	{
	  printf("inchidere city_hub\n");
	  break;
	}
      else if(strcmp(comanda, "start_monitor")==0)
	{
	  pid_t pid_hub_mon=fork();

	  if(pid_hub_mon<0)
	    {
	      printf("nu s-a putut crea procesul hub_mon\n");
	    }
	  else if(pid_hub_mon==0)
	    {
	      int pfd[2];
	      if(pipe(pfd)<0)
		{
		  perror("hub_mon: eroare la crearea pipe-ului\n");
		  exit(-1);
		}
	      pid_t pid_monitor=fork();
	      if(pid_monitor<0)
		{
		  perror("hub_mon: eroare la fork pentr monitor\n");
		  exit(-1);
		}
	      else if(pid_monitor==0)
		{
		  //processul copil a lui hub_mon
		  dup2(pfd[1], STDOUT_FILENO);

		  close(pfd[0]);
		  close(pfd[1]);

		  execlp("./monitor_reports", "monitor_reports",NULL);
		  perror("eroare la execlp pentru monitor_reports\n");
		  exit(-1);
		}
	      else
		{
		  close(pfd[1]);

		  char buffer[1000];
		  int monitor_running=1;
		  while(citeste_linia(pfd[0], buffer, sizeof(buffer))>0)
		    {
		      if(strncmp(buffer, "EROARE:", 7)==0)
			{
			  printf("\nhub_mon notitficare: Monitorul nu porneste:  %s", buffer+7);
			  monitor_running=0;
			}
		      else if(strncmp(buffer, "INFO:", 5)==0)
			{
			  printf("\nhub_mon LOG: %s", buffer+5);
			}
		      else
			{
			  printf("hub_mon necunoscut: %s", buffer);
			}
		      fflush(stdout);
		    }
		  close(pfd[0]);

		  int status;
		  waitpid(pid_monitor, &status, 0);
		  if(!monitor_running)
		    {
		      printf("hub_mon: executie anulata din cauza unei instante care exista deja\n");
		    }
		  else
		    {
		      printf("hub_mon: procesul monitor s-a terminat\n");
		    }
		  fflush(stdout);
		  exit(0);
		}
	    }
	  else
	    {
	      printf("comanda start_monitor a fost trimisa in fundal\n");
	    }
	}
      else if(strncmp(comanda, "calculate_scores",16)==0)
	{
	  char* districte[MAX_DISTRICTS];
	  int nr_districte=0;

	  //extragere fiecare district din lista de districte
	  char* cuv=strtok(comanda, " ");
	  cuv=strtok(NULL, " ,");

	  while(cuv!=NULL && nr_districte<MAX_DISTRICTS)
	    {
	      districte[nr_districte]=strdup(cuv);
	      nr_districte++;
	      cuv=strtok(NULL, " ,");
	    }
	  if(nr_districte==0)
	    {
	      printf("Eroare: nu s-a specificat niciun district\n");
	      continue;
	    }
	  int pipe_fds[MAX_DISTRICTS][2];
	  pid_t pids[MAX_DISTRICTS];

	  printf("\nWORKLOAD SCORE:\n");

	  for(int i=0; i<nr_districte; i++)
	    {
	      if(pipe(pipe_fds[i])<0)
		{
		  perror("Eroare la crearea pipe ului pentru scorer\n");
		  pids[i]=-1;
		  continue;
		}
	      pids[i]=fork();
	      if(pids[i]<0)
		{
		  perror("Eroare la creearea procesului copil\n");
		  continue;
		}
	      else if(pids[i]==0)
		{
		  dup2(pipe_fds[i][1], STDOUT_FILENO);  //redirectionare STDOUT catre captul de scriere al pipeului

		  for(int j=0; j<=i; j++)
		    {
		      //inchidera tuturor descriptorilor
		      close(pipe_fds[j][0]);
		      close(pipe_fds[j][1]);
		    }

		  execlp("./scorer", "scorer", districte[i], NULL);

		  fprintf(stderr, "Eroare la executie scorer.c pentru districtul %s\n", districte[i]);
		  exit(-1);
		  
		}
	      else
		{
		  close(pipe_fds[i][1]);  //parintele inchide capatul de scriere
		}
	    }
	  for(int i=0; i<nr_districte; i++)
	    {
	      if(pids[i]>0)
		{
		  char buffer[1024];
		  while(citeste_linia(pipe_fds[i][0], buffer, sizeof(buffer))>0)
		    {
		      printf("%s", buffer);
		    }
		  close(pipe_fds[i][0]);
		  waitpid(pids[i], NULL, 0);
		}
	      free(districte[i]);
	    }
	  printf("\n");
	}
      else if(strlen(comanda)>0)
	{
	  printf("Comanda necunoscuta: %s\n", comanda);
	}
      
    }
  return 0;
}
