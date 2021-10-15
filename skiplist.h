#ifndef SKIPLIST_H
#define SKIPLIST_H

#include<iostream>
#include <ctime>
#include "citizens.h"

class sl_node
{
    private:
    citizen *ctz;
    std::string date;
    sl_node *next;
    sl_node *previous;
    sl_node *lower;
    sl_node *upper;
    
    public:
    citizen* get_citizen(void){return ctz;}
    int get_id(void) {return ctz->get_id();}
    std::string get_date(void){return date;}
    sl_node* get_next(void) {return next;}
    sl_node* get_previous(void) {return previous;}
    sl_node* get_lower(void) {return lower;}
    sl_node* get_upper(void) {return upper;}
    void set_next(sl_node *NEXT) {next=NEXT;}
    void set_previous(sl_node *PREVIOUS) {previous=PREVIOUS;}
    void set_lower(sl_node *LOWER) {lower=LOWER;}
    void set_upper(sl_node *UPPER) {upper=UPPER;}
    sl_node* higher_adress(sl_node*);
    sl_node(citizen*, std::string);
};

class skiplist
{
    private:
    int sl_size;
    sl_node* bottom;
    sl_node* top;
    void delete_row(sl_node*);

    public:
    int get_sl_size(void){return sl_size;}
    sl_node* get_bottom(void){return bottom;}
    sl_node* get_top(void){return top;}
    void inc_sl_size(void){sl_size++;}
    void dec_sl_size(void){sl_size--;}
    void set_bottom(sl_node* BOTTOM){bottom=BOTTOM;}
    void set_top(sl_node* TOP){top=TOP;}
    sl_node* search(int);
    int idexists(int);
    sl_node* insert(citizen*, std::string DATE);
    int del(int);
    skiplist();
    ~skiplist();
};

#endif
