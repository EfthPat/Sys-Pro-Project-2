#include "text_files.h"

// COnstructor

text_file::text_file(std::string NAME) {name=NAME;}

// Getters
std::string text_file::get_name() {return name;}
text_file* text_file::get_next() {return next;}

// Setters
void text_file::set_name(std::string NAME) {name=NAME;}
void text_file::set_next(text_file* NEXT){next=NEXT;}

// Constructor
text_files::text_files() {first_file=nullptr;}

// Getter
text_file* text_files::get_first_file() {return first_file;}

// Setter
void text_files::set_first_file(text_file* FIRST_FILE) {first_file=FIRST_FILE;}

// Member

text_file* text_files::member(std::string FILE)
{
    text_file *dummyptr=first_file;

    while(dummyptr!=nullptr)
    {
        if(dummyptr->get_name()==FILE)
        {
            return dummyptr;
        }
        dummyptr=dummyptr->get_next();
    }
    return nullptr;
}

// Insert

text_file* text_files::insert(std::string FILE)
{
    text_file *file_adress, *temp;

    file_adress=member(FILE);

    if(file_adress==nullptr)
    {
        temp=first_file;
        first_file= new text_file(FILE);
        first_file->set_next(temp);
        return first_file;
    }
    else
        return file_adress;
}


void text_files::print_textfiles(void)
{
    text_file* current_file=get_first_file();

    while(current_file!=nullptr)
    {
        std::cout<<current_file->get_name()<<std::endl;
        current_file=current_file->get_next();
    }

}
