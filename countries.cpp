#include <string>
#include "countries.h"

country::country(std::string NAME)
{
    name=NAME;
    next=nullptr;
    population=upto20_p=upto40_p=upto60_p=above60_p=0;
}

country* countries::member(std::string COUNTRY)
{
    country *dummyptr=first_country;

    while(dummyptr!=nullptr)
    {
        if(dummyptr->get_name()==COUNTRY)
        {
            return dummyptr;
        }
        dummyptr=dummyptr->get_next();
    }
    return nullptr;
}

country* countries::insert(std::string COUNTRY)
{
    country *cnt_adress, *temp;
    
    cnt_adress=member(COUNTRY);

    if(cnt_adress==nullptr)
    {
        temp=first_country;
        first_country= new country(COUNTRY);
        first_country->set_next(temp);
        return first_country;
    }
    else
        return cnt_adress;
}

countries::countries()
{
    first_country=nullptr;
}

countries::~countries()
{
    country *dummyptr=first_country, *next;

    while(dummyptr!=nullptr)
    {
        next=dummyptr->get_next();
        delete dummyptr;
        dummyptr=next;
    }
}

void countries::print_countries()
{
    country* current_country=first_country;
    while(current_country!=nullptr)
    {
        std::cout<<current_country->get_name()<<std::endl;
        current_country=current_country->get_next();
    }
}

