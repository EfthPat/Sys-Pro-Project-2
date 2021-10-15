#ifndef CITIZENS_H
#define CITIZENS_H

#include <string>
#include "countries.h"


class citizen
{
    private:

    int id;
    int age;
    std::string firstname;
    std::string lastname;
    country* origin;
    citizen* next;

    public:

    int get_id(void){return id;}
    int get_age(void){return age;}
    std::string get_firstname(void){return firstname;}
    std::string get_lastname(void){return lastname;}
    citizen* get_next(void){return next;}
    std::string get_origin(void){return origin->get_name();}
    void set_next(citizen* NEXT){next=NEXT;}
    citizen(int, int, std::string, std::string, std::string, countries*);

    void print_citizen(void);
};

class citizens
{
    private:
    citizen* first_citizen;

    public:
    citizen* member(int);
    citizen* insert(int, int, std::string, std::string, std::string, countries*);
    citizens();
    ~citizens();
};

#endif
