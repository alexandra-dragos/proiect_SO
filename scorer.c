#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX 100

typedef struct{
  int report_id;
  char inspector_name[50];
  float longitude, latitude;
  char issue_category[30];
  int severity;
  time_t timestamp;
  char description_text[250];
}Report;

typedef struct{
  char nume[50];
  int scor_total;
}InspectorScor;

int main(int argc, char* argv[])
{
  if(argc<2)
    {
      printf("EROARE:nu s-a specificat niciun district\n");
      return 1;
    }
  char* district=argv[1];
  char file_path[255];
  sprintf(file_path, "%s/reports.dat", district);

  FILE* file=fopen(file_path, "rb");
  if(file==NULL)
    {
      printf("EROARE:districtul %s nu are rapoarte sau fisierul lipseste\n", district);
      return 2;
    }

  InspectorScor vector[MAX];
  int inspector_count=0;

  Report r;
  while(fread(&r, sizeof(Report), 1, file)==1)
    {
      int gasit=0;
      for(int i=0; i<inspector_count; i++)
	{
	  if(strcmp(vector[i].nume, r.inspector_name)==0)
	    {
	      vector[i].scor_total+=r.severity;
	      gasit=1;
	      break;
	    }
	}
      //daca insepctorul nu este inca in tabel il adaugam
      if(gasit==0 && inspector_count<MAX)
	{
	  strcpy(vector[inspector_count].nume, r.inspector_name);
	  vector[inspector_count].scor_total=r.severity;
	  inspector_count++;
	}
    }
  fclose(file);

  //trimitem rezultatele catre STDOUT
  printf("\nRezultate district: %s\n", district);
  for(int i=0; i<inspector_count; i++)
    {
      printf("Inspector: %s | Scor: %d\n", vector[i].nume, vector[i].scor_total);
    }
  
  return 0;
}
