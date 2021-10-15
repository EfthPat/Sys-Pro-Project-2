#include "mst.h"

using namespace std;

// Command:
//rm fifo* && ./travelMonitor -m 5 -b 100000 -s 500 -i "./input_dir"


int main(int argv, char** argc)
{
    cout<<"hi"<<endl;
    sigset_t fullset;
    sigfillset(&fullset);
    sigprocmask(SIG_SETMASK,&fullset, nullptr);

    // Ελεγξε αν τα command line arguments ειναι εγκυρα
    commline_info cli=comm_handler::test_master_args(argv, argc);
    if(cli.error_sum==1)
    {
        std::cout<<"ERROR: Invalid amount of arguments."<<std::endl;
        return 1;
    }
    if(cli.error_sum==2)
    {
        std::cout<<"ERROR: Some flags are not in valid format."<<std::endl;
        return 2;
    }
    if(cli.error_sum==4)
    {
        std::cout<<"ERROR: Invalid numMonitors size."<<std::endl;
        return 3;
    }
    if(cli.error_sum==8)
    {
        std::cout<<"ERROR: Invalid buffer size."<<std::endl;
        return 4;
    }
    if(cli.error_sum==16)
    {
        std::cout<<"ERROR: Invalid bloom filter size."<<std::endl;
        return 5;
    }
    DIR *dir_ptr=opendir(argc[8]);
    if(dir_ptr==nullptr)
    {
        std::cout<<"ERROR: Failed to open directory."<<std::endl;
        return 6;
    }

    // Παρε τα values των arguments
    int workers=cli.numMonitors;
    int buffsize=cli.buffsize;
    int blflsize=cli.blflsize;
    std::string input_dir=argc[8];

    dirent* entry;
    unsigned int countries_found=0;
    while((entry=readdir(dir_ptr))!=nullptr)
    {
        if(entry->d_name[0]!='.')
            countries_found++;
    }
    closedir(dir_ptr);
    if(countries_found==0)
    {
        std::cout<<"ERROR: Input directory is empty."<<std::endl;
        return 7;
    }

    dir_ptr=opendir(argc[8]);
    if(workers>countries_found)
        workers=countries_found;

    // Δημιουργησε ενα αντικειμενο τυπου πατερα
    mst father(buffsize,blflsize,dir_ptr,fifoname,input_dir);

    // Δημιουργησε τους workers και τα FIFOs
    process_info packet=father.create_workers(workers,countries_found);
    father.set_mask(packet.old_mask);

    // Διαβασε τα bloom filters ολων των παιδιων
    father.read_bloomfilters();

    // Διαβασε για εντολη απ' το πληκτρολογιο (σε loop)
    father.read_command();

    return 0;
}
