Tema 2 APD - Implementarea protocolului BitTorrent

In aceasta tema, asa cum se evidentiaza din titlu, am avut de implementat protocolul BitTorrent folosind MPI si pthread.
Am decis sa folosesc C++ pentru implementarea temei, deoarece mi s-a parut mult mai usor de implementat protocolul,
plus ca am mai lucrat cu C++ in trecut si mi s-a parut un limbaj destul de usor de folosit.

Acum, voi prezenta fiecare functie, structura si clasa implementata in cadrul temei:

struct file: Aici retinem informatiile despre un fisier. Contine numele fisierului (std::string name), numarul de segmente
(int num_segments), vectorul de hash-uri ale segmentelor (std::vectorstd::string segments) si un unordered_set cu
segmentele descarcate (std::unordered_set<int> downloaded_segments).

struct ThreadData: Aici vom retine datele partajate intre thread-urile de download si upload. Contine rank-ul procesului
(int rank), vectorul de fisiere detinute initial (std::vector<file> owned_files), vectorul cu numele fisierelor dorite
(std::vectorstd::string wanted_files), un unordered_map cu fisierele descarcate (std::unordered_map<std::string, file>
downloaded_files), o variabila booleana ce indica daca thread-ul ar trebui sa se opreasca (bool should_exit) si un mutex
de tip pthread_mutex_t pentru sincronizarea accesului la datele partajate.

void read_input_file(int rank, ThreadData* data): Aceasta functie citeste fisierul de intrare specific pentru procesul curent (in functie de rank) si populeaza campurile 
owned_files si wanted_files din structura ThreadData.

void save_downloaded_files(int rank, const std::string& file_name, const file& f): Aceasta functie salveaza continutul unui fisier descarcat intr-un fisier de iesire specific 
procesului curent (in functie de rank si numele fisierului).

void *download_thread_func(void *arg): Aceasta este functia executata de thread-ul responsabil cu descarcarea fisierelor. Mai intai trimite tracker-ului informatii despre 
fisierele detinute initial. Apoi, pentru fiecare fisier dorit, trimite o cerere tracker-ului pentru a obtine lista de peer-i care detin acel fisier. Selecteaza random un peer 
din lista primita si ii cere informatii despre numarul de segmente ale fisierului. Apoi, pentru fiecare segment, selecteaza random un peer si cere segmentul respectiv. Dupa 
primirea fiecarui segment, il adauga in fisierul descarcat si incrementeaza un contor. Cand contorul ajunge la un prag (UPDATE_INTERVAL), cere din nou tracker-ului lista 
actualizata de peer-i. La final, salveaza fisierul descarcat si trimite un mesaj tracker-ului ca a terminat de descarcat fisierul curent. Dupa ce a terminat de descarcat toate 
fisierele, trimite un mesaj tracker-ului ca a terminat complet si asteapta un mesaj inapoi inainte de a iesi.

void *upload_thread_func(void *arg): Aceasta este functia executata de thread-ul responsabil cu incarcarea (upload) segmentelor catre alti peer-i. Intr-un loop, asteapta sa 
primeasca cereri de la alti peer-i. Daca primeste o cerere de tip "CHUNK_REQUEST", extrage numele fisierului si indexul segmentului cerut. Cauta in fisierele detinute initial 
si in cele descarcate daca detine fisierul si segmentul cerut. Daca le detine, trimite inapoi numarul total de segmente (daca s-a cerut metadata) sau hash-ul segmentului 
specific. Daca nu le detine, trimite inapoi 0 (daca s-a cerut metadata) sau un hash gol. La primirea mesajului "DONE" de la tracker, iese din loop si se opreste.

void tracker(int numtasks, int rank): Aceasta functie implementeaza logica tracker-ului. Mai intai asteapta sa primeasca de la fiecare peer informatii despre fisierele 
detinute initial si actualizeaza in consecinta un unordered_map (file_swarm) ce mapeaza numele unui fisier la multimea de peer-i (rankuri) ce detin acel fisier. Dupa aceea, 
intra intr-un loop in care asteapta mesaje de la peer-i. Daca primeste un mesaj de tip MSG_REQUEST_SWARM, raspunde cu lista de peer-i ce detin fisierul cerut. Daca primeste 
MSG_FILE_COMPLETED, actualizeaza file_swarm adaugand peer-ul curent la multimea celor ce detin fisierul respectiv. Daca primeste MSG_ALL_COMPLETED de la toti peer-ii, iese din 
loop si trimite tuturor mesajul "DONE" pentru a-i notifica sa se opreasca.

void peer(int numtasks, int rank): Aceasta functie implementeaza logica unui peer. Initializeaza o structura ThreadData, citeste fisierul de intrare corespunzator rank-ului 
sau, apoi lanseaza doua thread-uri - unul de download si unul de upload. Asteapta ca ambele thread-uri sa se termine, dupa care elibereaza resursele alocate (destructor pentru 
mutex).

int main(int argc, char *argv[]): Aici se afla functia main ce initializeaza mediul MPI, determina numarul de procese (numtasks) si rank-ul procesului curent, apoi apeleaza 
fie functia tracker (pe procesul cu rank 0), fie functia peer (pe celelalte procese). La final, inchide mediul MPI.

In concluzie, aceasta tema a fost o oportunitate excelenta de a exersa utilizarea MPI si pthread pentru a implementa un protocol complex precum BitTorrent. Am invatat cum sa impartim responsabilitatile intre procese cu roluri diferite (tracker si peer-i) si intre thread-uri cu sarcini diferite (download si upload). De asemenea, am exersat comunicarea intre procese folosind MPI si sincronizarea accesului la date partajate intre thread-uri folosind mutex-uri.