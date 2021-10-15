#include "request.h"

// Constructor
request::request(std::string DATE, bool VALID, std::string COUNTRY)
{
    date=DATE;
    valid=VALID;
    country=COUNTRY;
    next=nullptr;
}

// Getters
std::string request::get_date()
{ return date; }

bool request::get_valid()
{ return valid; }

std::string request::get_country()
{ return country; }

request *request::get_next()
{ return next; }

// Setters
void request::set_date(std::string DATE)
{ date=DATE; }

void request::set_valid(bool VALID)
{ valid=VALID; }

void request::set_country(std::string COUNTRY)
{ country=COUNTRY; }

void request::set_next(request *NEXT)
{ next=NEXT; }

// Constructor
requests::requests()
{
    first_request=nullptr;
    total_requests=0;
    valid_requests=0;
    rejected_requests=0;
}

// Getters
unsigned int requests::get_total_requests()
{ return total_requests; }

unsigned int requests::get_valid_requests()
{ return valid_requests; }

unsigned int requests::get_rejected_requests()
{ return rejected_requests; }

// Operations

int requests::indates(date x, date y, date z)
{
    if(x.year<y.year || (x.year==y.year && x.month<y.month) || (x.year==y.year && x.month==y.month && x.day<y.day))
        return -1;
    else if(x.year>z.year || (x.year==z.year && x.month>z.month) || (x.year==z.year && x.month==z.month && x.day>z.day))
        return -1;
    else
        return 0;
}

request *requests::insert(std::string DATE, bool VALID, std::string COUNTRY)
{
    request *temp_req=first_request;
    first_request=new request(DATE, VALID, COUNTRY);
    first_request->set_next(temp_req);
    total_requests++;
    if(VALID)
        valid_requests++;
    else
        rejected_requests++;
    return first_request;
}

request_info requests::calc_requests(std::string DATE1, std::string DATE2, std::string COUNTRY)
{
    request *curr_req = first_request;
    unsigned int total_reqs = 0, valid_reqs = 0, rejected_reqs=0;
    date date1=comm_handler::stringtodate(DATE1);
    date date2=comm_handler::stringtodate(DATE2);
    date temp;

    while(curr_req!=nullptr)
    {
        if(curr_req->get_country()==COUNTRY)
        {
            temp=comm_handler::stringtodate(curr_req->get_date());
            if(indates(temp, date1, date2)==0)
            {
                total_reqs++;
                if(curr_req->get_valid())
                    valid_reqs++;
                else
                    rejected_reqs++;
            }
        }
        curr_req=curr_req->get_next();
    }
    return {total_reqs, valid_reqs, rejected_reqs};
}

// Destructor
requests::~requests()
{
    request *curr_req=first_request;
    while(curr_req!=nullptr)
    {
        request *next_req=curr_req->get_next();
        delete curr_req;
        curr_req=next_req;
    }
}
