#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H

#include <iostream>

class bloomfilter
{
    private:
    char* ptr;
    unsigned int size;

    static unsigned long sdbm(std::string&);
    static unsigned long djb2(std::string&);
    static unsigned long hash_i(std::string &, unsigned int);

    public:
    bloomfilter(unsigned int);
    char* get_ptr(void){return ptr;}
    unsigned int get_size(void){return size;}
    void set_bits(unsigned long*);
    int val_exists(unsigned long*);

    static unsigned long* hash_values_of(std::string&);
    void print_bloomfilter(void);


    ~bloomfilter();
};

#endif
