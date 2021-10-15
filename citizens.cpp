#include <string>
#include "citizens.h"

citizen::citizen(int ID, int AGE, std::string FIRSTNAME, std::string LASTNAME, std::string ORIGIN, countries* cnts_ptr)
{
    country* cnt_ptr;

    id=ID;
    age=AGE;
    firstname=FIRSTNAME;
    lastname=LASTNAME;
    next=nullptr;

    cnt_ptr=cnts_ptr->insert(ORIGIN);
    origin=cnt_ptr;
    cnt_ptr->inc_population();

    if(age>0 && age<=20)
        cnt_ptr->inc_upto20();
    else if(age>20 && age<=40)
        cnt_ptr->inc_upto40();
    else if(age>40 && age<=60)
        cnt_ptr->inc_upto60();
    else
        cnt_ptr->inc_above60();
}

void citizen::print_citizen()
{
    std::cout<<"FN:"<<get_firstname()<<" LN:"<<get_lastname()<<" AGE:"<<get_age()<<" CNT:"<<get_origin()<<std::endl;
}


citizen* citizens::member(int ID)
{
    citizen *dummyptr=first_citizen;

    while(dummyptr!=nullptr)
    {
        if(dummyptr->get_id()==ID)
        {
            return dummyptr;
        }
        dummyptr=dummyptr->get_next();
    }
    return nullptr;
}

citizen* citizens::insert(int ID, int AGE, std::string FIRSTNAME, std::string LASTNAME, std::string ORIGIN, countries* cnts_ptr)
{
    citizen *civilian_adress, *temp;
    
    civilian_adress=member(ID);

    if(civilian_adress==nullptr)
    {
        temp=first_citizen;
        first_citizen= new citizen(ID, AGE, FIRSTNAME, LASTNAME, ORIGIN, cnts_ptr);
        first_citizen->set_next(temp);
        return first_citizen;
    }
    else
    {
        return civilian_adress;
    }
}

citizens::citizens()
{
    first_citizen=nullptr;
}


citizens::~citizens()
{
    citizen *current, *next;
    current=first_citizen;

    while (current!=nullptr)
    {
        next=current->get_next();
        delete current;
        current=next;
    }
}
