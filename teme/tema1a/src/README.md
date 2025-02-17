Tema 1 APD - Implementarea paradigmei Map-Reduce pentru calculul paralel al unui index inversat

In aceasta tema, asa cum se evidentiaza din titlu, am avut de implementat un index inversat folosind paradigma Map-Reduce.
Am decis sa folosesc C++ pentru implementarea temei, deoarece mi s-a parut mult mai usor de implementat paradigma 
Map-Reduce, plus ca am mai lucrat cu C++ in trecut si mi s-a parut un limbaj destul de usor de folosit.

Acum, voi prezenta fiecare functie, structura si clasa implementata in cadrul temei:
class WordEntry: Aici retinem o intrare a unui cuvant in indexul inversat. Contine un string (cuvantul in sine) si un 
vector de tip int, care reprezinta id-urile documentelor in care apare cuvantul. De asemenea, aici gasim si functia 
addFile, care adauga un id de fisier in vectorul de id-uri al cuvantului. Prin asta, se evita duplicarea id-urilor in
vector.

class MapperResults: Aici se retin rezultatele mapper-ului. Contine un vector de WordEntry-uri, care reprezinta cuvintele 
procesate de mapper si id-urile documentelor in care acestea apar.

class SharedState: Aici vom retine starea partajata intre toate thread-urile. Avem un vector de stringuri, ce retine numele 
fisierelor de intrare, un vector cu vectori de stringuri, ce retine continutul fisierelor de intrare, un vector 
MapperResults, ce retine rezultatele tuturor mapper-ilor un mutex de tip pthread_mutex_t, ce va fi folosit
pentru sincronizarea accesului la rezultatele mapper-ilor si reducer-ilor, si o bariera de tip pthread_barrier_t, ce va fi
folosita pentru sincronizarea mapper-ilor si reducer-ilor. De asemenea, avem o functie explicita de initializare a
mutex-ului si a barierei, plus o functie de distrugere a mutex-ului si a barierei.

struct ThreadArg: Aici avem o structura ce paseaza argumentele catre thread-uri. Contine un pointer catre SharedState, un 
int ce reprezinta id-ul thread-ului si un int ce reprezinta numarul de thread-uri.

std::string cleanWord(const std::string& word): E o functie ce primeste un cuvant si il curata de caracterele nedorite, ca 
numerele, semnele de punctuatie, ghilimelele, etc. Dupa curatarea cuvantului, transforma cuvantul in litere mici si il 
returneaza.

void addWordtoResults(MapperResult& result, const std::string& word, int file_id): E o functie ce adauga un cuvant in 
rezultatele mapper-ului. Daca cuvantul exista deja in rezultate, adauga doar id-ul fisierului in vectorul de id-uri al 
cuvantului. Altfel, creeaza o noua intrare in rezultatele mapper-ului.

void* mapper(void* arg): Aceasta e una dintre functiile principale ale temei. Aici se implementeaza partea de Map a
paradigmei Map-Reduce. Luam un threadArg, de unde extragem SharedState-ul, id-ul thread-ului si numarul de thread-uri
ce ruleaza ca si mapperi. De asemenea, declaram un MapperResult, unde vom retine rezultatele locale ale mapper-ului.
Pentru distribuirea fisierelor catre mapperi, folosim o metoda de distribuire round-robin (fiecare mapper primeste cate un
fisier pe rand). Sa zicem ca avem mapperul M1 si va lua fisierul F1. Cum functioneaza un mapper e in felul urmator:
- M1 va lua linie cu linie din fisierul F1 si va procesa fiecare cuvant din linie, caracter cu caracter (pentru extragerea
eficienta a cuvintelor).
- Cuvintele extrase sunt curatate folosind functia cleanWord
- Daca cuvantul nu exista in rezultatele mapper-ului, il adaugam folosind functia addWordtoResults
- Acest lucru se repeta pentru fiecare mapper in parte, pana cand toate fisierele sunt procesate.
In final, folosind mutex-ul din SharedState, fiecare mapper va adauga rezultatele sale in vectorul de MapperResults din SharedState si folosind bariera, se asteapta ca toate thread-urile sa termine.

void* reducer(void* arg): Aceasta e cealalta functie principala a temei. Aici se implementeaza partea de Reduce a
paradigmei Map-Reduce. Luam un threadArg, de unde extragem SharedState-ul, id-ul thread-ului si numarul de thread-uri
ce ruleaza ca si reduceri. Fiecare reducer va procesa cate o litera din alfabet, tot in mod round-robin. Pentru fiecare
litera, reducerul va proceda astfel:
- va crea un fisier de forma "litera.txt" (ex: a.txt, b.txt, etc)
- va combina rezultatele tuturor mapper-ilor intr-un singur vector de WordEntry-uri (pentru litera curenta)
- va sorta vectorul dupa 2 factori: numarul de aparitii al cuvantului in ordine descrescatoare si alfabetic
- va scrie in fisierul creat cuvantul si id-urile documentelor in care apare cuvantul sub forma "cuvant :[id1, id2, ...]"
In final, fiecare reducer va scrie in fisierul sau rezultatele obtinute.

int main(int argc, char* argv[]): Aceasta e functia principala a temei, unde se ruleaza toata logica temei. Aici vom
scrie in argv[1] numarul de thread-uri ce ruleaza ca si mapperi, in argv[2] numarul de thread-uri ce ruleaza ca si reduceri
si in argv[3] numele fisierului de input. Vom initializa SharedState-ul cu numarul de mapperi si reduceri, apoi vom citi 
din fisierulde input numarul de fisiere de citit si fisierele ce trebuie citite linie cu linie. Apoi, vom crea un vector
de tip pthread_t, unde vom retine thread-urile mapperilor si reducerilor. Apoi, vom crea thread-urile mapperilor si
reducerilor, si asteptam sa se termine toate thread-urile. In final, returnam 0.