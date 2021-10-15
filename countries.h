#ifndef COUNTRIES_H
#define COUNTRIES_H

#include <iostream>
#include <string>

class citizen;
class citizens;

class country
{
    private:
    std::string name;
    country* next;
    int population;
    int upto20_p;
    int upto40_p;
    int upto60_p;
    int above60_p;

    public:
    std::string get_name(void){return name;}
    country* get_next(void){return next;}
    int get_population(void){return population;}
    int get_upto20(void){return upto20_p;}
    int get_upto40(void){return upto40_p;}
    int get_upto60(void){return upto60_p;}
    int get_above60(void){return above60_p;}
    void set_next(country* NEXT){next=NEXT;}
    void inc_population(void){population++;}
    void inc_upto20(void){upto20_p++;}
    void inc_upto40(void){upto40_p++;}
    void inc_upto60(void){upto60_p++;}
    void inc_above60(void){above60_p++;}
    country(std::string);
};

class countries
{
    private:
    country* first_country;

    public:
    country* get_first_country(void){return first_country;}
    country* member(std::string);
    country* insert(std::string);
    void print_countries(void);
    countries(); 
    ~countries();
};

#endif
