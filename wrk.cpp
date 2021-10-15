#include "wrk.h"

wrk* glob_ptr;

using namespace std;

// Τα arguments: WRK_EXEC, fifoname, process_number,

// Constructor
wrk::wrk(int argv, char** argc)
{
    glob_ptr=this;
    // Αρχικοποιησε το struct sigaction του εργατη
    init_sigaction();
    // Παρε τον καταληκτικο αριθμο του FIFO για να ξερεις απο ποια FIFOs να διαβασεις/γραψεις
    std::string pr_num(argc[1]);
    pr_number=std::stoi(pr_num);

    std::string fullname_p="fifo_P_"+pr_num;
    std::string fullname_c="fifo_C_"+pr_num;
    // Ανοιξε το FIFO για write με FLAG = RDWR και αποθηκευσε τον write descriptor
    write_fd=open(fullname_c.c_str(),O_RDWR);
    // Ανοιξε το FIFO για read και αποθηκευσε τον read descriptor
    read_fd=open(fullname_p.c_str(),O_RDONLY);
    add_fd_wr(write_fd);
    add_fd_rd(read_fd);


    // Θεσε το buffer size στο 1 byte *για το πρωτο μηνυμα και μονο*
    buffer_size=1;
    std::string tmp_msg=receive_message();
    std::string** strptr=comm_handler::tokenize(tmp_msg.c_str());

    total_requests=valid_requests=rejected_requests=0;

    // Υπολογισε τα ονοματα των αρχειων για ανοιγμα και αποθηκευσε τα: BUFFER_SIZE, BLFL_SIZE, INPUT_DIR_PATH, DIR_PTR, PR_NUM
    buffer_size=std::stoi(strptr[0][0]);
    blfl_size=std::stoi(strptr[1][0]);
    input_dir_path=strptr[2][0];
    dir_ptr=opendir(input_dir_path.c_str());

    comm_handler::delete_tokens(strptr);

    // DATA STRUCTURES:
    // TEXT_FILES:
    textfilesptr=new text_files;
    // COUNTRIES:
    countriesptr=new countries;
    // Διαβασε απο το FIFO τις χωρες που σου ανατεθηκαν και σπασ' τες σε επι μερους
    std::string countries=receive_message();
    cnts_ptr=comm_handler::tokenize(countries.c_str());
    // Κανε τις χωρες insert στη λιστα χωρων
    for(unsigned int i=0;cnts_ptr[i]!=nullptr;i++)
        countriesptr->insert(cnts_ptr[i][0]);
    // CITIZENS:
    citizensptr=new citizens;
    // VIRUSES:
    virusesptr=new viruses;
}

// Αρχικοποιει το struct sigaction του εργατη - καλειται απ' τον Constructor
void wrk::init_sigaction()
{
    memset(&worker_sig_set,0,sizeof(worker_sig_set));
    // Αρχικοποιησε τα flags
    worker_sig_set.sa_flags=SA_RESTART;
    // Βαλε ως signal handler την συναρτηση worker_sig_handler
    worker_sig_set.sa_handler=worker_sig_handler;
    // Βαλε για ποια σηματα θα καλειται ο signal handler
    sigaction(SIGINT,&worker_sig_set, nullptr);
    sigaction(SIGQUIT,&worker_sig_set, nullptr);
    sigaction(SIGUSR1,&worker_sig_set, nullptr);
    // Βαλε ολα τα σηματα να μπλοκαρονται κατα την κληση του signal handler
    sigfillset(&worker_sig_set.sa_mask);
}

// Getters
int wrk::get_write_fd() {return write_fd;}
int wrk::get_read_fd() {return read_fd;}
unsigned int wrk::get_buffer_size() {return buffer_size;}
unsigned int wrk::get_blfl_size() {return blfl_size;}
DIR* wrk::get_dir_ptr() {return dir_ptr;}
std::string** wrk::get_cnts_ptr() {return cnts_ptr;}
std::string wrk::get_input_dir_path() {return input_dir_path;}
unsigned int wrk::get_pr_number() {return pr_number;}
text_files* wrk::get_textfilesptr(void) {return textfilesptr;}
countries *wrk::get_countriesptr() {return countriesptr;}
citizens* wrk::get_citizensptr() {return citizensptr;}
viruses* wrk::get_virusesptr() {return virusesptr;}

// Setters

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// DIRECTORIES - TEXT FILES

void wrk::read_dir()
{
    // Για καθε χωρα στον πινακα χωρων
    for(unsigned int i=0;cnts_ptr[i]!=nullptr;i++)
    {
        // Υπολογισε το path της χωρας
        std::string country_path=input_dir_path+"/"+cnts_ptr[i][0];
        // Ανοιξε το directory της χωρας
        DIR* curr_dir_ptr=opendir(country_path.c_str());

        READ_FILE:
        // Πηγαινε στο text file
        dirent* entry=readdir(curr_dir_ptr);
        if(entry!=nullptr)
        {
            std::string text_file(entry->d_name);
            if(text_file!="." && text_file!="..")
            {
                // Βαλ' το στη λιστα αρχειων
                textfilesptr->insert(text_file);
                // Υπολογισε το ονομα του text file: country_path + text_file
                std::string curr_text_file=country_path+"/"+text_file;
                read_text_file(curr_text_file,cnts_ptr[i][0]);
            }
            goto READ_FILE;
        }
        closedir(curr_dir_ptr);
    }
}

void wrk::read_text_file(std::string filename, std::string country)
{

    std::ifstream mystream;
    std::string line;
    // Ανοιξε το αρχειο για διαβασμα
    mystream.open(filename);

    // Αν το αρχειο ανοιξε
    if(mystream.is_open())
    {
        // Παρε τη γραμμη
        while(getline(mystream,line))
        {
            // Ελεγξε αν η γραμμη ειναι λεκτικα εγκυρη. Αν δεν ειναι, τυπωσε σφαλμα, αλλιως προχωρα στη δευτερη φαση ελεγχου
            record_info record=comm_handler::val_line(line, country);
            if(record.valid==-1)
                cout<<"ERROR IN RECORD: "<<line<<"."<<endl;
            else
                test_record(record);
        }
        mystream.close();
    }
    else
        std::cout<<"ERROR: Failed to open text file."<<std::endl;
}

void wrk::test_record(record_info record)
{
    bool willinsert;
    virus* virusptr;
    citizen* citizenptr;

    int id=record.id;
    std::string firstname=record.firstname;
    std::string lastname=record.lastname;
    std::string country=record.country;
    int age=record.age;
    std::string virus=record.virus;
    std::string state=record.state;
    std::string date=record.date;

    virusptr=virusesptr->member(virus);
    citizenptr=citizensptr->member(id);

    if(citizenptr==nullptr)
        willinsert=true;
    else if(citizenptr->get_firstname()!=firstname || citizenptr->get_lastname()!=lastname || citizenptr->get_age()!=age || citizenptr->get_origin()!=country)
        willinsert=false;
    else
    {
        if(virusptr==nullptr)
            willinsert=true;
        else if(virusptr->get_vacc()->idexists(id) || virusptr->get_unvacc()->idexists(id))
            willinsert=false;
        else
            willinsert=true;
    }
    if(!willinsert)
    {
        std::cout<<"ERROR IN RECORD: "<<id<<" "<<firstname<<" "<<lastname<<" "<<country<<" "<<age<<" "<<virus<<" "<<state;
        if(state=="YES")
            std::cout<<" "<<date;
        std::cout<<"."<<std::endl;
    }
    else
    {
        insert_record(id,firstname,lastname,country,age,virus,state,date);
    }
}

void wrk::insert_record(int id, std::string fn, std::string ln, std::string country, int age, std::string disease, std::string state, std::string date)
{

    virus* virusptr;
    citizen* citizenptr;
    std::string id_string_value;
    unsigned long* ptr;

    // Βαλε την χωρα, τον πολιτη και τον ιο
    countriesptr->insert(country);
    citizenptr=citizensptr->insert(id, age, fn, ln, country, countriesptr);
    virusptr=virusesptr->insert(disease, blfl_size);

    id_string_value=std::to_string(id);

    // Αν ο πολιτης εμβολιαστηκε
    if(state=="YES")
    {
        ptr=bloomfilter::hash_values_of(id_string_value);
        virusptr->get_blfl()->set_bits(ptr);
        sl_node* curr=virusptr->get_vacc()->insert(citizenptr,date);
        delete[] ptr;
    }
    else
        virusptr->get_unvacc()->insert(citizenptr,"");

}

// Ελεγχει αν προστεθηκαν καινουρια text files σε ολες χωρες ενος εργατη
void wrk::test_subdirectories()
{
    // Φτιαξε ενα αρχειο της μορφης U<PID>_ στο τρεχον directory
    std::string filename="./U"+std::to_string(getpid())+"_";
    creat(filename.c_str(),0777);

    // Παρε τη λιστα των text files
    text_files* txt_ptr=get_textfilesptr();
    // Παρε τον πινακα χωρων
    std::string** cntsptr=get_cnts_ptr();

    // Για καθε χωρα που εισαι υπευθυνος
    for(unsigned int i=0;cntsptr[i]!=nullptr;i++)
    {
        // Υπολογισε το path της χωρας
        std::string country_path=input_dir_path+"/"+cntsptr[i][0];
        // Ανοιξε το directory της χωρας
        DIR* country_dir=opendir(country_path.c_str());

        CHECK_TEXTFILES:
        // Πηγαινε στο text_file
        dirent* textfile=readdir(country_dir);
        // Αν δεν εισαι στο τελος του directory
        if (textfile!=nullptr)
        {
            // Παρε το ονομα του text file
            std::string textfile_name(textfile->d_name);
            // Αν το ονομα δεν ειναι "." / ".." και δεν ειναι περασμενο στο database
            if(textfile_name!="." && textfile_name!=".." && txt_ptr->member(textfile_name)==nullptr)
            {
                // Βαλ' το στο database
                get_textfilesptr()->insert(textfile_name);
                // Υπολογισε το path του text file: country_path + text_file
                std::string curr_text_file=country_path+"/"+textfile_name;
                // Διαβασε το text file
                read_text_file(curr_text_file,cntsptr[i][0]);
            }
            goto CHECK_TEXTFILES;
        }
    }
    // Στειλε το σημα SIGUSR1 στον πατερα
    kill(getppid(),SIGUSR1);
    // Στειλε στον πατερα τα ανανεομενα bloomfilters
    send_bloomfilters();
}




void wrk::make_logfile()
{
    std::string pathname="./log_file.";
    std::string str_pid=std::to_string(getpid());

    pathname.append(str_pid);
    // Φτιαξε και ανοιξε το logfile
    ofstream logfile(pathname);
    country* current_country=get_countriesptr()->get_first_country();
    while(current_country!=nullptr)
    {
        logfile<<current_country->get_name()<<endl;
        current_country=current_country->get_next();
    }
    logfile<<"TOTAL TRAVEL REQUESTS "<<total_requests<<endl;
    logfile<<"ACCEPTED "<<valid_requests<<endl;
    logfile<<"REJECTED "<<rejected_requests<<endl;
    logfile.close();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ERRORS: "*R_F*" -> read() failed
std::string wrk::receive_message()
{

    // Δημιουργησε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);

    char temp_buff[1];
    bool found_end=false;
    int bytes_read;
    std::string rd_amt="0";

    do
    {
        bytes_read=read(read_fd,temp_buff,1);
        if(bytes_read==-1)
            return "*R_F*";
        if(temp_buff[0]=='_')
            found_end=true;
        else
            rd_amt.push_back(temp_buff[0]);
    }while(!found_end);


    int read_amount=std::stoi(rd_amt);
    unsigned int min_amount;

    std::string message;
    while(read_amount>0)
    {
        if(buffer_size>read_amount)
            min_amount=read_amount;
        else
            min_amount=buffer_size;

        bytes_read=read(read_fd,buffer,min_amount);
        if(bytes_read==-1)
            return "*R_F*";
        for(unsigned int i=0;i<bytes_read;i++)
            message.push_back(buffer[i]);
        read_amount-=bytes_read;
    }

    return message;
}

message wrk::read_message()
{

    // Δημιουργησε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);

    char temp_buff[1];
    bool found_end=false;
    int bytes_read;
    std::string rd_amt;

    do
    {
        bytes_read=read(read_fd,temp_buff,1);
        if(bytes_read==-1)
                return {nullptr,0};
        if(temp_buff[0]=='_')
            found_end=true;
        else
            rd_amt.push_back(temp_buff[0]);
    }while(!found_end);

    // Υπολογισε το μεγεθος που πρεπει να διαβασεις και φτιαξε ενα πινακα τετοιου μεγεθους
    unsigned int read_amount=std::stoi(rd_amt);
    char* message=new char[read_amount];
    int min_amount;
    unsigned int init_read_amount=read_amount;


    // Το j ειναι για την κινηση πανω στον πινακα message
    unsigned int j=0;
    while(read_amount>0)
    {
        if(buffer_size>read_amount)
            min_amount=read_amount;
        else
            min_amount=buffer_size;

        bytes_read=read(read_fd,buffer,min_amount);
        if(bytes_read==-1)
                return {message,0};
        for(unsigned int i=0;i<bytes_read;i++)
        {
            message[j]=buffer[i];
            j++;
        }
        read_amount-=bytes_read;
    }

    return {message,init_read_amount};
}

// ERRORS: 1 -> write() failed, ..
int wrk::send_message(std::string message)
{
    message.push_back('\0');

    // Υπολογισε το κωδικοποιημενο μηνυμα και το μηκος του
    unsigned int msg_size=message.size();
    std::string encoded=std::to_string(msg_size);
    encoded.push_back('_');
    unsigned int final_msg_size=encoded.size()+msg_size;
    encoded.append(message);

    // Ορισε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);
    // Κανε την μετατροπη τυπου απο string σε const char*
    const char* str_ptr=encoded.c_str();

    unsigned int j=0, k;

    CP_TO_BUFF:
    k=0;
    while(j<final_msg_size && k<buffer_size)
    {
        buffer[k]=str_ptr[j];
        k++;
        j++;
    }
    if(k==buffer_size && j!=final_msg_size)
    {
        int bytes_written=write(write_fd,buffer,buffer_size);
        if(bytes_written==-1)
            return 1;
        goto CP_TO_BUFF;
    }
    else
    {
        int bytes_written=write(write_fd,buffer,k);
        if(bytes_written==-1)
            return 1;
    }
    return 0;
}

// ERRORS: 1 -> write() failed, ..
int wrk::send_message(const char* message, unsigned int length)
{
    // Υπολογισε το κωδικοποιημενο μηνυμα και το μηκος του
    std::string encoded=std::to_string(length);
    encoded.push_back('_');
    unsigned int msg_size=encoded.size();

    // Ορισε τον buffer και αρχικοποιησε τον με το συμβολο BUFF_INIT
    char buffer[buffer_size];
    memset(buffer,BUFF_INIT,buffer_size);
    // Κανε την μετατροπη τυπου απο string σε const char* για το encoded
    const char* str_ptr=encoded.c_str();
    bool encoded_sent=false;

    unsigned int j=0, k;

    CP_TO_BUFF:
    k=0;
    while(j<msg_size && k<buffer_size)
    {
        if(!encoded_sent)
            buffer[k]=str_ptr[j];
        else
            buffer[k]=message[j];
        k++;
        j++;
    }

    if(k==buffer_size && j!=msg_size)
    {
        int bytes_written=write(write_fd,buffer,buffer_size);
        if(bytes_written==-1)
            return 1;
        goto CP_TO_BUFF;
    }

    int bytes_written=write(write_fd,buffer,k);
    if(bytes_written==-1)
        return 1;

    if(!encoded_sent)
    {
        encoded_sent=true;
        j=0;
        msg_size=length;
        goto CP_TO_BUFF;
    }

    return 0;
}


void wrk::read_from_fifo()
{
    CHECK_AVAILABILITY:
    unblock_all();
    serve_signals();
    fd_set temp_read_fds=read_fds;
    select(max_fd,&temp_read_fds, nullptr, nullptr,&timeout);
    if(FD_ISSET(read_fd,&temp_read_fds)==0)
        goto CHECK_AVAILABILITY;

    // Διαβασε το μηνυμα απ' τον read file descriptor
    message msg=read_message();
    // Κανε το αντιστοιχο action με βαση το μηνυμα που ελαβες
    analyze_message(msg);
    delete[] msg.message_ptr;
    
    goto CHECK_AVAILABILITY;
}




void wrk::analyze_message(message msg)
{
    // Σπασε το μηνυμα σε tokens
    std::string** str_ptr=comm_handler::tokenize(msg.message_ptr);

    // ACTION CODE A : ο πατερας θελει να δει αν υπαρχει ο πολιτης
    if(str_ptr[0][0]=="A")
    {
        // Παρε το id
        int id=std::stoi(str_ptr[1][0]);
        // Ελεγξε αν υπαρχει στο database
        citizen* citizenptr=citizensptr->member(id);
        // Αν οχι, στειλε μηνυμα οτι δεν υπαρχει
        if(citizenptr==nullptr)
            send_message("NO");
        else
        {
            send_message("YES");
        }
    }
    // ACTION CODE B : ο πατερας προωθησε το query /travelRequest στο monitor
    else if(str_ptr[0][0]=="B")
    {
        // Παρε τα id, date, countryfrom(_), countryto(_), virus
        int id=std::stoi(str_ptr[1][0]);
        std::string date=str_ptr[2][0];
        std::string countryfrom=str_ptr[3][0];
        std::string countryto=str_ptr[4][0];
        std::string disease=str_ptr[5][0];

        // Βρες τον ιο
        virus* virus_ptr=virusesptr->member(disease);


        // Πηγαινε στη vaccinated skip list του ιου
        skiplist* skiplist_ptr=virus_ptr->get_vacc();
        // Ελεγξε αν υπαρχει ο πολιτης στη vaccinated skip list
        sl_node* node=skiplist_ptr->search(id);

        // Αν ο πολιτης δεν υπαρχει, στειλε ΝΟ
        if(node==nullptr || node->get_previous()==nullptr || node->get_id()!=id)
            send_message("NO");
        // Αλλιως, στειλε YES με την ημερομηνια στο τελος
        else
        {
            std::string vacc_date=node->get_date();
            send_message("YES "+vacc_date);
        }
    }
    // ACTION CODE C : ο πατερας προωθησε το query /searchVaccinationStatus στο monitor
    else if(str_ptr[0][0]=="C")
    {
        // Παρε το id του πολιτη
        int id=std::stoi(str_ptr[1][0]);
        citizen* citizen_ptr=citizensptr->member(id);
        // Αν ο πολιτης δεν υπαρχει στο database, στειλε στον πατερα NO
        if(citizen_ptr==nullptr)
            send_message("NO");
        // Αν ομως ο πολιτης υπαρχει στο database
        else
        {
            std::string message;

            // Παρε τα στοιχεια του: ονομα, επωνυμο, χωρα καταγωγης, ηλικια
            std::string first_name=citizen_ptr->get_firstname();
            std::string last_name=citizen_ptr->get_lastname();
            std::string country=citizen_ptr->get_origin();
            std::string age=std::to_string(citizen_ptr->get_age());

            // Ενωσε τα ολα σε ενα μηνυμα
            message="YES "+first_name+" "+last_name+" "+country+" "+age;

            //Πηγαινε στους ιους
            virus* current_virus=virusesptr->get_first_virus();
            // Για καθε ιο
            while(current_virus!=nullptr)
            {
                // Παρε το ονομα του
                std::string virus_name=current_virus->get_name();
                // Πηγαινε στην vaccinated skip list του
                skiplist *vacc_sl=current_virus->get_vacc();
                // Ελεγξε αν ο πολιτης εχει εμβολιαστει
                sl_node *vacc_node=vacc_sl->search(id);
                std::string vacc_date;


                // Αν ο πολιτης δεν ανηκει στο vaccinated skip list, βαλε την ημερομηνια στο "N"
                if(vacc_node==nullptr || vacc_node->get_previous()==nullptr || vacc_node->get_id()!=id)
                    vacc_date="N";
                // Αλλιως παρε την ημερομηνια που εμβολιαστηκε
                else
                    vacc_date=vacc_node->get_date();

                // Φτιαξε το μηνυμα στη μορφη " vacc_date*virus"
                vacc_date.push_back('*');
                vacc_date.append(virus_name);
                std::string token=" ";
                token.append(vacc_date);
                // Ενωσε το με το αρχικο μηνυμα
                message.append(token);
                // Προχωρα στον επομενο ιο
                current_virus=current_virus->get_next();
            }
            // Στειλε το τελικο μηνυμα στον πατερα
            send_message(message);
        }
    }
    // ACTION CODE D : ο πατερας θελει να δει αν ο ιος υπαρχει στο database του παιδιου
    else if(str_ptr[0][0]=="D")
    {
        std::string disease=str_ptr[1][0];
        virus* virus_ptr=virusesptr->member(disease);
        if(virus_ptr==nullptr)
            send_message("NO");
        else
            send_message("YES");
    }
    // ACTION CODE E : το process αυξανει τα total requests και valid ή rejected αναλογα με το τι μηνυμα εστειλε ο πατερας
    else if(str_ptr[0][0]=="E")
    {
        total_requests++;
        if(str_ptr[1][0]=="V")
            valid_requests++;
        else
            rejected_requests++;
    }
    delete[] str_ptr;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void wrk::send_bloomfilter(virus* vr_ptr)
{
        unsigned int i=0, j=0;
        // Παρε το ονομα του ιου, το μηκος του και κανε την μετατροπη σε const char*
        std::string v_n=vr_ptr->get_name();
        unsigned int virus_length=v_n.size();
        const char* virus_name=v_n.c_str();

        // Παρε το bloom filter
        char* blfl=vr_ptr->get_blfl()->get_ptr();


        // Φτιαξε εναν πινακα μεγεθους: virus_length + 1 + bloomfilter_size
        unsigned int message_size=virus_length+1+blfl_size;
        char* message=new char[message_size];
        // Περνα το ονομα του ιου
        for(i=0;i<virus_length;i++)
            message[i]=virus_name[i];
        // Μετα το '_'
        message[i]='_';
        i++;
        // Και τελος το bloom filter
        for(j=0;j<blfl_size;j++)
        {
            message[i]=blfl[j];
            i++;
        }

        // Στειλε το μηνυμα στον πατερα
        send_message(message,message_size);
        // Κανε free την μνημη του πινακα
        delete[] message;
}


void wrk::send_bloomfilters()
{
    virus* vr_ptr=get_virusesptr()->get_first_virus();

    while(vr_ptr!=nullptr)
    {
        send_bloomfilter(vr_ptr);
        vr_ptr=vr_ptr->get_next();
    }
    // Ready
    char ready[1]={'R'};
    send_message(ready,1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Ο sighandler του εργατη
void worker_sig_handler(int signum)
{
    glob_ptr->set_pend_sig(signum);
}

// Συναρτηση που εξυπηρετει τα σηματα
void wrk::serve_signals()
{
    block_all();

    // Αν τουλαχιστον ενα απ' τα σηματα SIGINT ή SIGQUIT εκκρεμουν
    if(sig_pending(SIGINT)==1 || sig_pending(SIGQUIT)==1)
    {
        // Φτιαξε το logfile
        make_logfile();
        // Καθαρισε και τα 2 σηματα απ' τον πινακα σηματων
        clr_pend_sig(SIGINT);
        clr_pend_sig(SIGQUIT);
    }

    // Αν το σημα SIGUSR1 εκκρεμει
    if(sig_pending(SIGUSR1)==1)
    {
        // Ελεγξε τα subdirectories των χωρων σου για να βρεις τα καινουρια text files
        test_subdirectories();
        // Σβησ' το απ τον πινακα σηματων
        clr_pend_sig(SIGUSR1);
    }

}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void wrk::print_stats()
{
    std::cout<<"-pid: "<<getpid();
    std::cout<<" - pr_no:"<<pr_number;
    std::cout<<" - wr_fd: "<<write_fd;
    std::cout<<" - rd_fd: "<<read_fd;
    std::cout<<" - bf_sz: "<<buffer_size;
    std::cout<<" - bl_sz: "<<blfl_size;
    std::cout<<" - dr_ph: "<<input_dir_path;
    std::cout<<" - dr_pr: "<<dir_ptr;
    std::cout<<std::endl;
}

void wrk::print_viruses(void)
{
    virus* current_virus=virusesptr->get_first_virus();

    while(current_virus!=nullptr)
    {
        cout<<"virus_name:"<<current_virus->get_name()<<endl;
        skiplist* vacc_sl=current_virus->get_vacc();
        cout<<"vacc_skiplist_size:"<<vacc_sl->get_sl_size()<<endl;
        sl_node* record=vacc_sl->get_bottom();
        while(record!=nullptr)
        {
            if(record->get_citizen()!=nullptr)
            {
                cout<<"id:"<<record->get_id()<<" - date:"<<record->get_date()<<endl;
            }
            record=record->get_next();
        }
        current_virus=current_virus->get_next();
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

