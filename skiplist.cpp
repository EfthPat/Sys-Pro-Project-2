#include <string>
#include <iostream>
#include "skiplist.h"

sl_node::sl_node(citizen* CTZ, std::string DATE)
{
    ctz=CTZ;
    date=DATE;
    next=previous=lower=upper=nullptr;
}

sl_node* sl_node::higher_adress(sl_node *current)
{
    sl_node *higher, *lmost;
    do
    {
        if (current == nullptr)
            return nullptr;
        else
        {
            higher = current->get_upper();
            if (higher != nullptr)
                return higher;
            else
            {
                lmost = current->get_previous();
                if (lmost != nullptr)
                    current = lmost;
                else
                    return nullptr;
            }
        }
    } while (1);
}

sl_node* skiplist::search(int ID)
{
    sl_node *current=get_top();

    if(current==nullptr)
        return nullptr;
    do
    {
        if(current->get_next()!=nullptr && current->get_next()->get_id()<=ID)
            current=current->get_next();
        else if(current->get_lower()!=nullptr)
            current=current->get_lower();
        else
            return current;
    } while (1!=0);
}

int skiplist::idexists(int ID)
{
    sl_node *pos=search(ID);
    
    if(pos->get_previous()==nullptr || pos->get_id()!=ID)
        return 0;
    else
        return 1;
}

sl_node* skiplist::insert(citizen* civilian, std::string DATE) // Γυρναει NULL αν υπαρχει ηδη εγγραφη με το ιδιο ονομα
{
    int civilian_ID, flag=0;
    sl_node *pos, *next_pos, *current, *higher_pos, *base=nullptr, *min_int_node, *temp;
    
    civilian_ID=civilian->get_id();
    pos=search(civilian_ID);

    if(pos==nullptr || (pos->get_previous()!=nullptr && pos->get_id()==civilian_ID))
        return nullptr;
    else
    {
        do
        {
            next_pos=pos->get_next();
            if(flag==0)
            {
                current = new sl_node(civilian,DATE);
                flag=1;
            }
            else
                current = new sl_node(civilian,"");
            current->set_previous(pos);            
            current->set_next(next_pos);
            current->set_lower(base);
            pos->set_next(current);
            if(next_pos!=nullptr)
                next_pos->set_previous(current);
            if(base!=nullptr)
                base->set_upper(current);
            base=current;
            if(rand()%2!=0)
            {
                higher_pos=current->higher_adress(current);
                if(higher_pos==nullptr)
                {
                    min_int_node=new sl_node(nullptr,"");
                    temp=get_top();
                    set_top(min_int_node);
                    min_int_node->set_lower(temp);
                    temp->set_upper(min_int_node);
                    pos=get_top();
                }
                else
                    pos=higher_pos;
            }
            else
                break;
        } while(1!=0);
        inc_sl_size();
        return current;
    }

return nullptr;
}

int skiplist::del(int ID)
{
    sl_node *pos, *next_pos, *prev_pos, *upper_pos, *temp;
    int flag=0;

    pos=search(ID);
    if(pos!=nullptr)
        prev_pos=pos->get_previous();
    else
        return 0;

    if(prev_pos==nullptr || pos->get_id()!=ID) // Αν προσπαθησει να σβησει το MIN_INT column ή ενα ID που δεν υπαρχει, FAIL
        return 0;
    else
    {
        do
        {
            prev_pos=pos->get_previous();
            next_pos=pos->get_next();
            upper_pos=pos->get_upper();
            delete pos;
            prev_pos->set_next(next_pos);
            if(next_pos!=nullptr)
                next_pos->set_previous(prev_pos);   
            pos=upper_pos;
        } while(pos!=nullptr);

        while(top!=bottom && top->get_next()==nullptr)
        {
            temp=top->get_lower();
            delete top;
            top=temp;
        }
        top->set_upper(nullptr);
        dec_sl_size();
        return 1;
    }
}

skiplist::skiplist()
{
    sl_size=0;
    bottom=new sl_node(nullptr,"");
    top=bottom;
}

skiplist::~skiplist()
{
    sl_node *current, *upper;

    current=bottom;

    while(current!=nullptr)
    {
        upper=current->get_upper();
        delete_row(current);
        current=upper;
    }
}

void skiplist::delete_row(sl_node* current)
{
    sl_node *next;

    while(current!=nullptr)
    {
        next=current->get_next();
        delete current;
        current=next;
    }
}
