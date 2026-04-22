#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct{
  int report_id;
  char inspector_name[50];
  float longitude, latitude;
  char issue_category[30];
  int severity;
  time_t timestamp;
  char description_text[250];
}Report;

int main(int argc, char* argv[])
{
  char* role=NULL;
  char* user="unknown";
  char* district_name=NULL;

  //extrag rolul, userul si numele districtului
  for(int i=0; i<argc; i++)
    {
      if(strcmp(argv[i], "--role")==0 && (i+1<argc))
      {
	role=argv[i+1];
      }
      if(strcmp(argv[i], "--user")==0 && (i+1<argc))
	{
	  user=argv[i+1];
	}
      if(strcmp(argv[i], "--add")==0 && (i+1<argc))
	{
	  district_name=argv[i+1];
	}
    }

  if(role==NULL)
    {
      printf("nu s-a specificat niciun rol\n");
      return 1;
    }

  if(district_name!=NULL)
    {
      struct stat st={0};
      char report_file[512];
      sprintf(report_file, "%s/reports.dat", district_name);

      //daca districyul nu exista il cream
      if(stat(district_name, &st)==-1)
	{
	  if(strcmp(role, "manager")!=0)
	    {
	      printf("doar un manager poate crea un district\n");
	      return 1;
	    }
	  printf("Districtul nou: %s\n", district_name);
	  mkdir(district_name, 0750);

	  FILE* f_init=fopen(report_file, "wb");
	  if(f_init !=NULL)
	    fclose(f_init);
	  chmod(report_file, 0664);

	  //district.cfg
	  char file_path[512];
	  sprintf(file_path, "%s/district.cfg", district_name);
	  FILE* f2=fopen(file_path, "w");
	  if(f2!=NULL)
	    {
	      fprintf(f2, "2\n");
	      fclose(f2);
	    }
	  chmod(file_path, 0640);

	  //logged_district
	  sprintf(file_path, "%s/logged_district", district_name);
	  FILE* f3=fopen(file_path, "w");
	  if(f3!=NULL)
	    {
	      fprintf(f3, "Actiune: CREATE | user: %s | role: %s\n", user, role);
	      fclose(f3);
	    }
	  chmod(file_path, 0644);
	  
	}
      FILE* file=fopen(report_file, "ab");
      if(file==NULL)
	{
	  printf("nu s-a putut accesa continutul raportului\n");
	  return 1;
	}
     
      Report rap_nou;
      printf("\nADAUGARE RAPORT NOU:\n");
      
      printf("introdu un id:");
      scanf("%d", &rap_nou.report_id);
      
      strncpy(rap_nou.inspector_name, user, 50);

      printf("categorie:");
      scanf("%29s", rap_nou.issue_category);

      printf("severitate:");
      scanf("%d", &rap_nou.severity);

      printf("longitudine:");
      scanf("%f", &rap_nou.longitude);
      printf("latitudine:");
      scanf("%f", &rap_nou.latitude);

      rap_nou.timestamp=time(NULL);
      while(getchar()!='\n');

      printf("descriere:");
      fgets(rap_nou.description_text, 250, stdin);
      rap_nou.description_text[strcspn(rap_nou.description_text, "\n")]='\0';

      //scriere in fisier
      if(fwrite(&rap_nou,sizeof(Report), 1, file)==1)
	{
	  printf("raportul %d a fost adaugat in continuarea fisierului\n", rap_nou.report_id);
	}
      else{
	printf("nu s-a putut adauga raportul\n");
      }

      fclose(file);
      chmod(report_file, 0664);
     
    }

  for(int i=0; i<argc; i++)
    {
      if(strcmp(argv[i], "--list")==0 && (i+1<argc))
	{
	  char* district_curent=argv[i+1];
	  char cale_fisier[512];
	  sprintf(cale_fisier, "%s/reports.dat", district_curent);

	  struct stat file_info;
	  if(stat(cale_fisier, &file_info)==-1)
	    {
	      printf("districtul %s nu exista sau nu are rapoarte\n", district_curent);
	      continue;
	    }
	  printf("\nInformatii fisier %s:\n", cale_fisier);
	  printf("marime in bytes: %ld\n", file_info.st_size);
	  printf("permisiuni:\n");
	  printf((file_info.st_mode & S_IRUSR) ? "r" : "-");
	  printf((file_info.st_mode & S_IWUSR) ? "w" : "-");
	  printf((file_info.st_mode & S_IXUSR) ? "x" : "-");
	  printf((file_info.st_mode & S_IRGRP) ? "r" : "-");
	  printf((file_info.st_mode & S_IWGRP) ? "w" : "-");
	  printf((file_info.st_mode & S_IXGRP) ? "x" : "-");
	  printf((file_info.st_mode & S_IROTH) ? "r" : "-");
	  printf((file_info.st_mode & S_IWOTH) ? "w" : "-");
	  printf((file_info.st_mode & S_IXOTH) ? "x" : "-");
	  printf("\n");
	  printf("data ultimei modificari: %ld\n", file_info.st_mtime);
	  
	  
	  FILE* bin_file=fopen(cale_fisier, "rb");
	  if(bin_file==NULL)
	    {
	      printf("nu s-a putut deschide fisierul\n");
	      continue;
	    }
	  Report temp;
	  printf("continut raport:\n");
	  while(fread(&temp, sizeof(Report), 1, bin_file)==1)
	    {
	      char time_str[20];
	      struct tm *tm_info=localtime(&temp.timestamp);
	      strftime(time_str, 20, "%Y-%m-%d %H:%M", tm_info);
	      printf("%d | %s | %s | %d | %s | %s\n", temp.report_id, temp.inspector_name, temp.issue_category, temp.severity, time_str, temp.description_text);
	    }
	  fclose(bin_file);
	}
    }
  

  for(int i=0; i<argc; i++)
    {
      if(strcmp(argv[i], "--view")==0 && (i+2<argc))
	{
	  char* district_id=argv[i+1];
	  int target_id=atoi(argv[i+2]);
	  char cale[512];
	  sprintf(cale, "%s/reports.dat", district_id);

	  struct stat file_info;
	  if(stat(cale, &file_info)==-1)
	    {
	      printf("districtul %s nu exista sau nu are rapoarte\n", district_id);
	      continue;
	    }
	  FILE* file=fopen(cale, "rb");
	  if(file==NULL)
	    {
	      printf("nu s-a putut deschide fisierul\n");
	      continue;
	    }
	  printf("\ninformatii fisier:%s\n",cale);
	  printf("marimea:%ld\n", file_info.st_size);
	  printf("Permisiuni:\n");
	  printf((file_info.st_mode & S_IRUSR) ? "r" : "-");
	  printf((file_info.st_mode & S_IWUSR) ? "w" : "-");
	  printf((file_info.st_mode & S_IXUSR) ? "x" : "-");
	  printf((file_info.st_mode & S_IRGRP) ? "r" : "-");
	  printf((file_info.st_mode & S_IWGRP) ? "w" : "-");
	  printf((file_info.st_mode & S_IXGRP) ? "x" : "-");
	  printf((file_info.st_mode & S_IROTH) ? "r" : "-");
	  printf((file_info.st_mode & S_IWOTH) ? "w" : "-");
	  printf((file_info.st_mode & S_IXOTH) ? "x" : "-");

	  
	  Report temp;
	  while(fread(&temp, sizeof(Report), 1, file)==1)
	    {
	      if(temp.report_id==target_id)
		{
		  printf("report id: %d\n", temp.report_id);
		  printf("nume inspector: %s\n", temp.inspector_name);
		  printf("tipul problemei: %s\n", temp.issue_category);
		  printf("severitate: %d\n", temp.severity);
		  printf("descriere: %s\n", temp.description_text);
		  break;
		  
		}
	    }
	  fclose(file);
	  i=i+2;
	}
    }

  for(int i=0; i<argc; i++)
    {
      if(strcmp(argv[i], "--remove_report")==0 && (i+2<argc))
	{
	  char* district_target=argv[i+1];
	  int target_id=atoi(argv[i+2]);

	  //verificare rol, doar managerul poate sterge
	  if(role==NULL || strcmp(role, "manager")!=0)
	    {
	      printf("doar managerul poate sterge rapoarte\n");
	      continue;
	    }
	  char cale[512];
	  sprintf(cale, "%s/reports.dat", district_target);
	  int f=open(cale, O_RDWR);
	  if(f<0)
	    {
	      printf("nu s-a putut deschide fisierul\n");
	      continue;
	    }
	  Report temp;
	  int gasit=0;
	  long offset_sters=0;
	  //caut raportul si salvez poz
	  while(read(f, &temp, sizeof(Report))==sizeof(Report))
	    {
	      if(temp.report_id==target_id)
		{
		  gasit=1;
		  offset_sters=lseek(f, 0, SEEK_CUR)-sizeof(Report);
		  break;
		}
	    }
	  if(gasit==1)
	    {
	      //deplasez rapoartele de dupa cel de sters cu o pozitie in sus
	      long offset_urmator=offset_sters+sizeof(Report);
	      Report aux;

	      while(lseek(f, offset_urmator, SEEK_SET)>=0 && read(f, &aux, sizeof(Report))==sizeof(Report))
		{
		  lseek(f, offset_sters, SEEK_SET);
		  write(f, &aux, sizeof(Report));
		  //avansez pozitiile
		  offset_sters+=sizeof(Report);
		  offset_urmator+=sizeof(Report);
		}
	      struct stat st;
	      fstat(f, &st);
	      ftruncate(f, st.st_size-sizeof(Report));

	      printf("raportul %d a fost sters\n", target_id);
	    }
	  else{
	    printf("raportul %d nu a fost gasit\n", target_id);
	  }
	  close(f);
	}
    }
  
  
  
  return 0;
}
