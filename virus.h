#ifndef VIRUS_H
#define VIRUS_H

#include <iostream>
#include <string>
#include "skiplist.h"
#include "bloomfilter.h"
#include "request.h"

class virus
{
    private:
    std::string name;
    requests* reqs;
    bloomfilter* blfl;
    skiplist* vacc;
    skiplist* unvacc;
    virus* next;

    public:
    std:: string get_name(void){return name;}
    requests* get_reqs(void){return reqs;}
    bloomfilter* get_blfl(void){return blfl;}
    skiplist* get_vacc(void){return vacc;}
    skiplist* get_unvacc(void){return unvacc;}
    virus* get_next(void){return next;}
    void set_next(virus* NEXT){next=NEXT;}
    virus(std::string, int);
    ~virus();
};

class viruses
{
    private:
    virus* first_virus;

    public:
    virus* get_first_virus(void){return first_virus;}
    virus* member(std::string);
    virus* insert(std::string, int);
    viruses();
    ~viruses();
};

#endif
