#include "comm_handler.h"

unsigned int comm_handler::count_tokens(const char* str)
{
    unsigned int tokens=0;
    char prev_state=' ';

    for(unsigned int i=0;str[i]!='\n' && str[i]!='\0';i++)
    {
        if(str[i]!=' ' && str[i]!='\t' && (prev_state==' ' || prev_state=='\t'))
            tokens++;
        prev_state=str[i];
    }
    return tokens;
}

std::string** comm_handler::tokenize(const char* str)
{
    unsigned int tokens=count_tokens(str);
    if(tokens==0)
        return nullptr;
    std::string** token_ptr=new std::string*[tokens+1];
    unsigned int i=0, j=0;
    while(str[i]!='\n' && str[i]!='\0')
    {
        if(str[i]==' ' || str[i]=='\t')
            i++;
        else
        {
            std::string word;
            do
            {
                word.push_back(str[i]);
                i++;
            }while(str[i]!=' ' && str[i]!='\t' && str[i]!='\n' && str[i]!='\0');
            token_ptr[j]=new std::string(word);
            j++;
        }
    }
    token_ptr[tokens]=nullptr;

    return token_ptr;
}

string_info comm_handler::tokenize_s(const char* str)
{
    unsigned int tokens=count_tokens(str);
    std::string** str_ptr=tokenize(str);
    return {tokens,str_ptr};
}

// ERRORS:
// 1 -> invalid amount of arguments, 2 -> invalid flags, 4 -> invalid numMonitors, 8 -> invalid buffer size
// 16 -> invalid bloom filter size, ..

commline_info comm_handler::test_master_args(int argv, char **argc)
{
    if(argv!=9)
        return {1, 0, 0, 0};
    std::string first_flag(argc[1]), second_flag(argc[3]), third_flag(argc[5]), fourth_flag(argc[7]);
    if(first_flag!="-m" || second_flag!="-b" || third_flag!="-s" || fourth_flag!="-i")
        return {2,0,0,0};
    int numMonitors=eval_number(argc[2]);
    if(numMonitors<1)
        return {4,0,0,0};
    int buffsize=eval_number(argc[4]);
    if(buffsize<1)
        return {8,0,0,0};
    int blflsize=eval_number(argc[6]);
    if(blflsize<1)
        return {16,0,0,0};

    return {0,numMonitors,buffsize,blflsize};
}

int comm_handler::eval_number(const char* ptr)
{
    int sum=0;
    for(int i=0;ptr[i]!='\0';i++)
    {
        if(ptr[i]<'0' || ptr[i]>'9')
            return -1;
        else
            sum=10*sum+ptr[i]-'0';
    }
    return sum;
}

// 1 = SUCCESS / -1 = FAIL
int comm_handler::val_id(std::string id)
{
    if(id=="")
        return -1;
    for(int i=0;i<id.length();i++)
    {
        if(id[i]<'0' || id[i]>'9')
            return -1;
    }
    return stoi(id);
}

int comm_handler::val_flc(std::string name)
{
    if(name=="")
        return -1;
    for(int i=0;i<name.length();i++)
    {
        if((name[i]<'A' || name[i]>'Z') && (name[i]<'a' || name[i]>'z'))
            return -1;
    }
    return 1;
}

int comm_handler::val_age(std::string age)
{
    int value;
    if(age=="")
        return -1;
    for(int i=0;i<age.length();i++)
    {
        if(age[i]<'0' || age[i]>'9')
            return -1;
    }
    value=stoi(age);
    if(value>0 && value<=120)
        return value;
    else
        return -1;
}

int comm_handler::val_virus(std::string virus)
{
    int dashes=0;
    if(virus=="")
        return -1;
    for(int i=0;i<virus.length();i++)
    {
        if((virus[i]<'A' || virus[i]>'Z') && (virus[i]<'a' || virus[i]>'z') && (virus[i]<'0' || virus[i]>'9') && virus[i]!='-')
            return -1;
        else if(virus[i]=='-')
        {
            if (dashes>0)
                return -1;
            else
                dashes++;
        }
    }
    return 1;
}

int comm_handler::val_state(std::string state)
{
    if (state=="YES" || state=="NO")
        return 1;
    else
        return -1;
}

int comm_handler::val_date(std::string date)
{
    int i=0, sum=0, dashes=0;

    for(i=0;i<date.length();i++)
    {
        if(date[i]=='-')
            dashes++;
    }
    if(dashes!=2)
        return -1;
    i=0;
    while(date[i]!='-')
    {
        if(date[i]<'0' || date[i]>'9')
            return -1;
        sum=10*sum+date[i++]-'0';
    }
    if(sum<1 ||sum>30)
        return -1;
    sum=0;
    i++;
    while(date[i]!='-')
    {
        if(date[i]<'0' || date[i]>'9')
            return -1;
        sum=10*sum+date[i++]-'0';
    }
    if(sum<1 || sum>12)
        return -1;
    sum=0;
    i++;
    while(date[i]!='\0')
    {
        if(date[i]<'0' || date[i]>'9')
            return -1;
        sum=10*sum+date[i++]-'0';
    }
    if(sum<1)
        return -1;
    return 1;
}


record_info comm_handler::val_line(std::string line, std::string country)
{
    string_info str=tokenize_s(line.c_str());
    std::string** str_ptr=str.str_ptr;
    record_info record;

    cout<<"ok"<<endl;

    if(str.tokens<7 || str.tokens>8)
        goto FAIL;
    if(val_id(str_ptr[0][0])==-1)
        goto FAIL;
    if(val_flc(str_ptr[1][0])==-1)
        goto FAIL;
    if(val_flc(str_ptr[2][0])==-1)
        goto FAIL;
    if(val_flc(str_ptr[3][0])==-1)
        goto FAIL;
    if(val_age(str_ptr[4][0])==-1)
        goto FAIL;
    if(val_virus(str_ptr[5][0])==-1)
        goto FAIL;
    if(val_state(str_ptr[6][0])==-1)
        goto FAIL;
    if(str.tokens==7)
    {
        if(str_ptr[6][0]=="YES")
            goto FAIL;
        goto SUCCESS;
    }
    if(val_date(str_ptr[7][0])==-1)
        goto FAIL;
    if(str_ptr[6][0]=="NO")
        goto FAIL;

    SUCCESS:
    record.valid=1;
    record.id=std::stoi(str_ptr[0][0]);
    record.firstname=str_ptr[1][0];
    record.lastname=str_ptr[2][0];
    record.country=country;
    record.age=std::stoi(str_ptr[3][0]);
    record.virus=str_ptr[4][0];
    record.state=str_ptr[5][0];
    if(str.tokens==6)
        record.date="#";
    else
        record.date=str_ptr[6][0];
    delete[] str_ptr;
    return record;

    FAIL:
    record.valid=-1;
    delete[] str_ptr;
    return record;
}

date comm_handler::stringtodate(std::string dt)
{
    int i=0, dashes=0, sum=0;
    date result;

    while(dt[i]!='\0')
    {
        if(dt[i]!='-')
            sum=10*sum+dt[i]-'0';
        else if(dashes++==0)
        {
            result.day=sum;
            sum=0;
        }
        else
        {
            result.month=sum;
            sum=0;
        }
        i++;
    }
    result.year=sum;

    return result;
}

// vacc date, travel date
int comm_handler::date_gap(std::string date1, std::string date2)
{
    date vacc_date=stringtodate(date1);
    date travel_date=stringtodate(date2);

    int total_days_1=(vacc_date.year-1)*12*30+(vacc_date.month-1)*30+vacc_date.day-1;
    int total_days_2=(travel_date.year-1)*12*30+(travel_date.month-1)*30+travel_date.day-1;
    int total_day_diff=total_days_2-total_days_1;

    // Cannot go back in time
    if(total_day_diff<0)
        return -1;
    if(total_day_diff>180)
        return 0;
    return 1;
}

int comm_handler::date_greater(std::string dt1, std::string dt2)
{
    date date1=stringtodate(dt1);
    date date2=stringtodate(dt2);

    if(date1.year<date2.year)
        return 1;
    else if(date1.year>date2.year)
        return 0;

    if(date1.month<date2.month)
        return 1;
    else if(date1.month>date2.month)
        return 0;

    if(date1.day>date2.day)
        return 0;
    else
        return 1;
}

void comm_handler::an_str(std::string str)
{
    std::string date, virus;
    bool split_found=false;

    for(unsigned int i=0;i<str.size();i++)
    {
        if(!split_found)
        {
            if(str[i]=='*')
                split_found=true;
            else
                date.push_back(str[i]);
        }
        else
            virus.push_back(str[i]);
    }

    std::cout<<virus<<" ";
    if(date=="N")
        std::cout<<"NOT YET VACCINATED"<<std::endl;
    else
        std::cout<<"VACCINATED ON "<<date<<std::endl;
}

int comm_handler::delete_tokens(std::string **str_ptr)
{
    if(str_ptr==nullptr)
        return -1;
    for(unsigned int i=0;str_ptr[i]!=nullptr;i++)
        delete str_ptr[i];
    delete[] str_ptr;
    return 0;
}


