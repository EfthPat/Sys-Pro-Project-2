#ifndef COMM_HANDLER_H
#define COMM_HANDLER_H

#include <iostream>
#include <string>

struct string_info
{
    unsigned int tokens;
    std::string** str_ptr;
};

struct commline_info
{
    int error_sum;
    int numMonitors;
    int buffsize;
    int blflsize;
};

struct record_info
{
    int valid;
    int id;
    std::string firstname;
    std::string lastname;
    std::string country;
    int age;
    std::string virus;
    std::string state;
    std::string date;
};

struct date
{
    int day;
    int month;
    int year;
};



class comm_handler
{
    private:

    public:

    static int eval_number(const char*);
    static unsigned int count_tokens(const char*);
    static std::string** tokenize(const char*);
    static int delete_tokens(std::string**);
    static string_info tokenize_s(const char*);
    static commline_info test_master_args(int argv, char** argc);

    static int val_id(std::string);
    static int val_flc(std::string);
    static int val_age(std::string);
    static int val_virus(std::string);
    static int val_state(std::string);
    static int val_date(std::string);
    static record_info val_line(std::string, std::string);

    static date stringtodate(std::string);
    static int date_gap(std::string,std::string);
    static int date_greater(std::string,std::string);


    static void an_str(std::string);
};


#endif