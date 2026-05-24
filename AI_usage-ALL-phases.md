Nume AI : Gemini
FAZA 1
Prompts: -am definit o structura Report in C cu campurile report_id, insepctor_name, issue_category, severity, timestamp, longitude, latitude si description_text. genereaza o functie parse_condition care sparga stringul de forma field:op:value cu sscanf -genreaza si o functie match_condition care primeste un raport si conditiile din stringul de mai sus si comapara severitatea, categoria si timestamp -cum pot detecta link urile simbolice dangling folosind lstat si stat in C -cum transform un camp de tip time_t intr un string care sa contina data so ora si ce functii sa folosesc

Ce s-a generat: -codul sursa pentru cele 2 functii -exemple pentru utilizarea functiilor symlink() si unlink() pentru crearea si actualizarea legaturilor simbolice -functionalitatea functiei lstat() -pasii pentru a transforma time_t intr-o structura struct tm cu localtime si apoi formatarea structurii intr un buffer de caractere cu strftime

Ce am schimbat: -am schimbat numele capurilor folosind structura mea in functia match_condition()

Ce am invatat: -diferenta dintre stat() si lstat() si ca lstat ne ofera informatii despre link-ul simbolic in sine -am invata cum pot sa folosesc functia strftime si localtime si cum pot sa stochez acest tip de date in fisiere binare

FAZA 2
1. Am folosit AI -ul pentru a afla diferenta dintre sigaction() si signal() si de ce ar trebui sa folosesc sigaction() in loc de signal(). Am cerut descrierea functiei sigaction() si am cerut informatii despre structura sigaction si cateva exemple.
Astfel am reusit sa verific ca festionarea semnalelor functioneaza fara erori si nu returneaza -1, care este returnat in caz de eroare.

2. Pentru intelegrea si clarificarea modului cum comunica cele 2 programe in C, monitor_reports.c si city_manager.c si cum pot sa le apelez in terminal corect.

3. Am cerut informatii despre functia execlp() pentru ca avem o eroare din cauza ca aveam lipsa un paramtru si astefl am reussit sa adaug si parametrul lipsa.

4. am cerut informatii si despre wait() care este esential at cand avem un proces copil si unul parinte pentru a evita oprirea procesului parinte inainte ca procesul copil sa execute rm. Astfel am reusit sa sincronizez pocesul copil si procesul parinte

FAZA 3

1. Am cerut explicatii privind cerinta legat de cum ar trebui procesele sa comunice intre ele prin pipe-uri pentru a intelege mai bine conexiunile dintre ele.

2. Informatii despre cum as putea sa curat procesele zombi, lucru folosit in city_hub.c pentru a evita umplerea tabelei de procese si blocarea sistemului de a nu mai putea crea procese noi, deoarece atunci cand comanda start_monitor sau calculate_scores se termina, procesul fiu moare, dar el nu dispare complet din sitem, ramane un proces Zombie.

3. Utilizarea fflush() si cum il pot folosi pentru curatarea buffer-ului chiar daca nu se intalnesc cele 2 cazuri de golire automata, cand intalneste \n si cand se cere un input.

4. In city_hub.c la parsarea listei de districte at cand folosesc strtok(), am aflat ca este mult mai sigur sa folosesc strdup() deoarece calculeaza lungimea textului din cuv si aloca memorie (cu malloc) si copiaza textul din cuv in noua zona de memorie si returneaza adresa acesteia. Astfel am evitat suprascrierea cu "gunoi" sau date noi in momentul in care bucla while(1) se rula si se introducea o noua comanda.

5. Am cerut informatii despre functia dup2() plus exemple cu redirectionarea input-ului si output-ului la nivel de sistem de operare.

6. Informatii despre cum pot conecta un proces de fundal(hub_mon) cu un executabil independent (monitor_reports.c) prin pipe-uri.

7. Am intrebat cum as putea sa fac conexiunea dintre city_hub.c si monitor_reports.c prin pipe si cum pot folosi prefixe precum INFO sau EROARE in fata mesajelor. Astfel am modificat citirea din pipe ca sa verifice aceste prefixe. Daca Hub-ul primeste EROARE isi da seama ca se incearca pornirea unei instante duplicat si opreste executia si afiseaza un mesaj.
