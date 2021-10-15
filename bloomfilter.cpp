#include<string>
#include "bloomfilter.h"

bloomfilter::bloomfilter(unsigned int bytes) // Φτιαχνει ενα πινακα χαρακτηρων μεγεθους <bytes> και θετει το size=<bytes>
{
    ptr = new char[bytes]; 
    for(int i=0;i<bytes;i++)
        ptr[i]=0;
    size=bytes;
}


bloomfilter::~bloomfilter(){delete[] ptr;}


void bloomfilter::set_bits(unsigned long* valptr) // Θετει συγκεκριμενα bits στο bloom filter σε 1 - Δεχεται ως παραμετρο ενα δεικτη σε πινακα u.l. [16]
{
    int i, offset, steps;
    unsigned long sizeinbits, norm;

    sizeinbits=(unsigned long)8*get_size(); // Μετατρεψε το μεγεθος του bloom filter σε bits
    
    for(i=0;i<16;i++)
    {
        norm=valptr[i]%sizeinbits; // Κανονικοποιησε το i-οστό hash value ετσι ωστε να 'χωραει' στα bits του bloom filter : norm in [0,bits-1]
        offset=norm/8; 
        steps=norm%8;
        ptr[offset] |= (char) 1<<(7-steps);        
    }
}

int bloomfilter::val_exists(unsigned long* valptr) // Ελεγχει αν τα bits σε συγκεκριμενες θεσεις στο bloom filter ειναι 1
{
    int i, offset, shiftamount;
    char temp;
    unsigned long sizeinbits, norm;

    sizeinbits=(unsigned long)8*get_size();

    for(i=0;i<16;i++)
    {
        norm=valptr[i]%sizeinbits;
        offset=norm/8;
        shiftamount=norm%8;
        temp=ptr[offset]<<shiftamount;
        if(temp>=0)
        return 0;
    }
    return 1; // 1 υπαρχει - 0 δεν υπαρχει
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long bloomfilter::hash_i(std::string &str, unsigned int i) // Καλει τις 2 hash functions
{
    return djb2(str) + i*sdbm(str) + i*i;
}

unsigned long* bloomfilter::hash_values_of(std::string& str) // καλει την hash_i() 16 φορες
{
    unsigned long* ptr;

    ptr=new unsigned long[16];
    for(int i=0;i<16;i++)
        ptr[i]=hash_i(str,i);

    return ptr;
}

unsigned long bloomfilter::sdbm(std::string& str) // Πρωτη hash function
{
    unsigned long hash = 0;
    int c, i;

    for (i=0;i<str.length();i++)
    {
        c = str[i];
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

unsigned long bloomfilter::djb2(std::string& str) // Δευτερη hash function
{
    unsigned long hash = 5381;
    int c, i;

    for (i=0;i<str.length();i++)
    {
        c = str[i];
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

void bloomfilter::print_bloomfilter()
{
    for(unsigned int i=0;i<size;i++)
        std::cout<<(int)ptr[i]<<" ";
    std::cout<<std::endl;
}



















