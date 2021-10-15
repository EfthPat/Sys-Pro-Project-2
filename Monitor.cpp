#include "wrk.h"

using namespace std;

// Τα arguments: PATH, fifoname, process_number, buffer_size, ...


int main(int argv, char** argc)
{
    // Φτιαξε ενα αντικειμενο τυπου εργατη
    wrk worker(argv,argc);
    // Διαβασε τα directories που σου ανατεθηκαν
    worker.read_dir();
    // Στειλε τα bloom filters στον πατερα
    worker.send_bloomfilters();
    // Διαβαζε απ' το FIFO σου
    worker.read_from_fifo();

    return 0;
}













