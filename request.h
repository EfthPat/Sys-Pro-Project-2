#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include <string>
#include "comm_handler.h"

struct request_info
{
    unsigned int total;
    unsigned int valid;
    unsigned int rejected;
};

class request
{
    private:
    std::string date;
    bool valid;
    std::string country;
    request* next;

    public:
    // Constructor
    request(std::string,bool,std::string);

    // Getters
    std::string get_date(void);
    bool get_valid(void);
    std::string get_country(void);
    request* get_next(void);

    // Setters
    void set_date(std::string);
    void set_valid(bool);
    void set_country(std::string);
    void set_next(request*);
};

class requests
{
    private:
    request* first_request;
    unsigned int total_requests;
    unsigned int valid_requests;
    unsigned int rejected_requests;

    public:
    // Constructor
    requests();
    // Getters
    unsigned int get_total_requests(void);
    unsigned int get_valid_requests(void);
    unsigned int get_rejected_requests(void);
    // Operations
    int indates(date,date,date);
    request* insert(std::string,bool,std::string);
    request_info calc_requests(std::string, std::string, std::string);
    // Destructor
    ~requests();
};



#endif