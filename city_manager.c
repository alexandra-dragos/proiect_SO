#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>

typedef struct{
  int report_id;
  char inspector_name[50];
  float longitude, latitude;
  char issue_category[30];
  int severity;
  time_t timestamp;
  char description_text[250];
}Report;

int parse_condition(const char *input, char *field, char *op, char *value) {
    if (sscanf(input, "%[^:]:%[^:]:%s", field, op, value) == 3) {
        return 1;
    }
    return 0;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int val = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity == val;
        if (strcmp(op, "!=") == 0) return r->severity != val;
        if (strcmp(op, ">") == 0)  return r->severity > val;
        if (strcmp(op, ">=") == 0) return r->severity >= val;
        if (strcmp(op, "<") == 0)  return r->severity < val;
        if (strcmp(op, "<=") == 0) return r->severity <= val;
    } 
    else if (strcmp(field, "category") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->issue_category, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->issue_category, value) != 0;
    }
    else if (strcmp(field, "inspector") == 0) {
        if (strcmp(op, "==") == 0) return strcmp(r->inspector_name, value) == 0;
        if (strcmp(op, "!=") == 0) return strcmp(r->inspector_name, value) != 0;
    }
    else if (strcmp(field, "timestamp") == 0) {
        long val = atol(value);
        if (strcmp(op, "==") == 0) return r->timestamp == val;
        if (strcmp(op, ">") == 0)  return r->timestamp > val;
        if (strcmp(op, "<") == 0)  return r->timestamp < val;
    }
    return 0;
}


void check_and_report_links() {
    DIR *d = opendir(".");
    if (!d) {
        perror("nu s-a putut deschide directorul curent");
        return;
    }

    struct dirent *dir;
    struct stat st_link;
    struct stat st_target;

    printf("\nSCANARE LINK-URI SIMBOLICE\n");
    
    while ((dir = readdir(d)) != NULL)
      {
        if (strncmp(dir->d_name, "active_reports-", 15) == 0)
	  {
            if (lstat(dir->d_name, &st_link) == 0)
	      {
                if (S_ISLNK(st_link.st_mode))
		  {
                    if (stat(dir->d_name, &st_target) == -1) {
                        printf("%s -> Destinatia este inexistenta sau inaccesibila!\n", dir->d_name);
                    } else {
                        printf("link valid: %s\n", dir->d_name);
                    }
                }
            }
        }
    }
    closedir(d);
}

int main(int argc, char* argv[])
{
  char* role=NULL;
  char* user="unknown";
  char* district_name=NULL;

  check_and_report_links();

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
      if((strcmp(argv[i], "--add")==0 || strcmp(argv[i], "--list")==0 || strcmp(argv[i], "--filter")==0 || strcmp(argv[i], "--view")==0 || strcmp(argv[i], "--remove_district")==0) && (i+1<argc))
	{
	  district_name=argv[i+1];
	}
    }

  if(role==NULL)
    {
      printf("nu s-a specificat niciun rol\n");
      return 1;
    }

  //logare actiuni
  if(district_name!=NULL)
    {
      char log_general_path[512];
      sprintf(log_general_path, "%s/logged_district", district_name);

      FILE* f_log_general=fopen(log_general_path, "a");
      if(f_log_general!=NULL)
	{
	  time_t timp=time(NULL);
	  char* timp_str=ctime(&timp);
	  timp_str[strcspn(timp_str, "\n")]='\0';

	  fprintf(f_log_general, "%s: User: %s | Role:%s | s-a initiat o actiune\n", timp_str, user, role);
	  fclose(f_log_general);
	}
    }

  int vrea_sa_adauge=0;
  for(int i=0; i<argc; i++)
    {
      if(strcmp(argv[i], "--add")==0)
	vrea_sa_adauge=1;
    }

  if(vrea_sa_adauge==1 && district_name!=NULL)
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
	  
	  char link_name[512];
	  char target_path[512];

	  sprintf(link_name, "active_reports-%s", district_name);
	  sprintf(target_path, "%s/reports.dat", district_name);
	  unlink(link_name);

	  //link simbolic
	  if (symlink(target_path, link_name) == 0)
	    {
	      printf("link simbolic creat: %s -> %s\n", link_name, target_path);
	    }
	  else
	    {
	      perror("nu s-a putut crea linkul\n");
	    }

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

      int pid=-1;
      int trimitere_notificare=0;

      FILE* f_monitor=fopen(".monitor_pid", "r");
      if(f_monitor==NULL)
	{
	  printf("eroare la citirea din .monitor_pid\n");
	}
      else
	{
	  if(fscanf(f_monitor, "%d", &pid)==1)
	    {
	      if(kill(pid, SIGUSR1)==0)
		{
		  trimitere_notificare=1;
		}
	    }
	  fclose(f_monitor);
	}
      
      char log_path[512];
      sprintf(log_path, "%s/logged_district", district_name);
      FILE* file_log=fopen(log_path, "a");
      if(file_log==NULL)
	{
	  printf("nu s-a putut scrie in %s/logged_district\n", district_name);
	}
      else
	{
	  if(trimitere_notificare==1)
	    {
	      fprintf(file_log,"Notificare trimisa monitorului. PID:%d\n",pid);
	    }
	  else
	    {
	      fprintf(file_log, "Notificarea nu a fost trimisa monitorului. PID:%d\n", pid);
	    }
	  fclose(file_log);
	}
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

	  char mod_time_str[25];
	  struct tm *tm_mod = localtime(&file_info.st_mtime);
	  strftime(mod_time_str, sizeof(mod_time_str), "%Y-%m-%d %H:%M:%S", tm_mod);
	  printf("data ultimei modificari: %s\n", mod_time_str);
	  
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

  for(int i=0; i<argc; i++)
    {
      if(strcmp(argv[i], "--update_threshold")==0 && (i+2<argc))
	{
	  char* district_id=argv[i+1];
	  char* val=argv[i+2];
	  char cfg_path[512];
	  sprintf(cfg_path, "%s/district.cfg", district_id);

	  //verificare rol
	  if(role==NULL || strcmp(role, "manager")!=0)
	    {
	      printf("doar un manager poate schimba pragul de severitate\n");
	      continue;
	    }
	  struct stat st;
	  if(stat(cfg_path, &st)==-1)
	    {
	      printf("fisierul %s nu exista\n", cfg_path);
	      continue;
	    }
	  mode_t permissions=st.st_mode & 0777;
	  if(permissions != 0640){
	    printf("permisiunile pt %s nu sunt 640\n", cfg_path);
	    continue;
	  }
	  FILE* f_cfg=fopen(cfg_path, "w");
	  if(f_cfg!=NULL)
	    {
	      fprintf(f_cfg, "%s\n", val);
	      fclose(f_cfg);
	      printf("pragul de severitate a fost actualizat pt %s la %s\n", district_id, val);
	    }
	  else{
	    printf("nu s-a putut deschide fisierul\n");
	  }
	  i+=2;
	}
    }

  for(int i=0; i<argc; i++)
    {
      if(strcmp(argv[i], "--filter")==0 && (i+2<argc))
	{
	  char* district_id=argv[i+1];
	  int index_prima_conditie=i+2;
	  int total_conditii=0;

	  //aflu cate conditii sunt
	  for(int j=index_prima_conditie; j<argc; j++)
	    {
	      if(strncmp(argv[j], "--", 2)==0)
		break;
	      total_conditii++;
	    }
	  char cale[512];
	  sprintf(cale, "%s/reports.dat", district_id);
	  int f=open(cale, O_RDONLY);
	  if(f<0)
	    {
	      printf("eroare la deschiderea fisierului\n");
	      continue;
	    }
	  Report temp;
	  
	  printf("rezultate pentru districtul %s\n", district_id);

	  while(read(f, &temp, sizeof(Report))==sizeof(Report))
	    {
	      int potriviri=1;

	      for(int j=0; j<total_conditii; j++)
		{
		  char camp[50];
		  char op[10];
		  char valoare[100];
		  if(parse_condition(argv[index_prima_conditie+j], camp, op, valoare))
		    {
		      if(!match_condition(&temp, camp, op, valoare))
			{
			  potriviri=0;
			  break;
			}
		    }
		}
	      if(potriviri==1)
		{
		  struct tm* tm_info=localtime(&temp.timestamp);
		  char time_str[30];
		  strftime(time_str, 20, "%Y-%m-%d %H:%M", tm_info);
		  printf("ID: %d | Categorie: %s | Severitate: %d | nume insepctor: %s | data:%s\n", temp.report_id, temp.issue_category, temp.severity, temp.inspector_name, time_str);
		}
	    }
	  close(f);
	  i=i+(1+total_conditii);    //sar peste argumentele deja procesate
	}
    }
for(int i=0; i<argc; i++)
    {
      if(strcmp(argv[i], "--remove_district")==0 && (i+1<argc))
	{
	  char* district_id=argv[i+1];
	  
	  //verificare rol
	  if(role==NULL || strcmp(role, "manager")!=0)
	    {
	      printf("doar managerul poate sterge directorul distriuctului %s\n", district_id);
	    }

	  //verificare daca este director
	  struct stat st;
	  if(stat(district_id, &st)==-1)
	    {
	      printf("nu s-a putut accesa districtul\n");
	      continue;
	    }
	  if(!S_ISDIR(st.st_mode))
	    {
	      printf("%s nu este un director\n", district_id);
	      continue;
	    }
	  char link_path[512];
	  sprintf(link_path, "active_reports-%s", district_id);
	  if((unlink(link_path))==0)
	    {
	      printf("s-a sters linkul simbolic %s\n", link_path);
	    }
	  else
	    {
	      printf("nu s-a putut sterge linkul simbolic\n");
	    }
	  
	  int pid;
	  if((pid=fork())<0)
	    {
	      perror("eroare\n");
	      exit(1);
	    }
	  if(pid==0)
	  {
	    printf("STERGERE %s\n", district_id);
	    execlp("rm","rm", "-rf", district_id, NULL);
	    perror("eroare la execlp");
	    exit(1);
	  }
	  else
	    {
	      wait(NULL);
	      printf("districtul %s a fost sters\n", district_id);
	    }
	  i++;
	  
	}
    }
    
  
  
  return 0;
}
