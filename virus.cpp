#include "virus.h"

//-------------------------------- VIRUS ------------------------------------------//

virus::virus(std::string NAME, int BLFL_SIZE)
{
    name=NAME;
    blfl=new bloomfilter(BLFL_SIZE);
    vacc=new skiplist;
    unvacc=new skiplist;
    reqs=new requests;
}

virus::~virus()
{
    delete blfl;
    delete vacc;
    delete unvacc;
    delete reqs;
}


//-------------------------------- VIRUSES ------------------------------------------//

virus* viruses::member(std::string VIRUS)
{
    virus *dummyptr=first_virus;

    while(dummyptr!=nullptr)
    {
        if(dummyptr->get_name()==VIRUS)
        {
            return dummyptr;
        }
        dummyptr=dummyptr->get_next();
    }
    return nullptr;
}

virus* viruses::insert(std::string VIRUS, int BLFL_SIZE)
{
    virus *cnt_adress, *temp;
    
    cnt_adress=member(VIRUS);

    if(cnt_adress==nullptr)
    {
        temp=first_virus;
        first_virus= new virus(VIRUS, BLFL_SIZE);
        first_virus->set_next(temp);
        return first_virus;
    }
    else
        return cnt_adress;
}


viruses::viruses()
{
    first_virus=nullptr;
}

viruses::~viruses()
{
    virus *current, *next;

    current=first_virus;

    while(current!=nullptr)
    {
        next=current->get_next();
        delete current;
        current=next;
    }
}