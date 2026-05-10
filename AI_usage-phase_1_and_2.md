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