#ifndef TEXT_FILES_H
#define TEXT_FILES_H

#include <iostream>
#include <string>

class text_file
{
    private:
    std::string name;
    text_file* next;

    public:
    text_file(std::string);
    std::string get_name(void);
    text_file* get_next(void);
    void set_name(std::string);
    void set_next(text_file*);
};


class text_files
{
    private:
    text_file* first_file;

    public:
    text_files();
    text_file* get_first_file(void);
    void set_first_file(text_file*);
    text_file* member(std::string);
    text_file* insert(std::string);

    void print_textfiles(void);
};


#endif