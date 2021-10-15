#ifndef COMMON_H
#define COMMON_H

// Οι κοινες βιβλιοθηκες μεταξυ των mst.h και wrk.h
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <fstream>
#include <sys/wait.h>


#include "process_info.h"
#include "proc_mask.h"
#include "comm_handler.h"
#include "text_files.h"
#include "bloomfilter.h"
#include "countries.h"
#include "citizens.h"
#include "skiplist.h"
#include "virus.h"
#include "filedes_set.h"



// Τα δικαιωματα των FIFOs σε οκταδικο
#define FIFO_P_PERMS 0777
#define FIFO_C_PERMS 0777

// Τα suffixes των FIFO: _P_ γραφει ο πατερας και διαβαζει το παιδι, _C_ γραφει το παιδι και διαβαζει ο πατερας
#define FIFO_P_SUFFIX "_P_"
#define FIFO_C_SUFFIX "_C_"

// Το τερματικο συμβολο ενος μηνυματος που ανταλλασεται μεταξυ master - worker
#define MSG_END '^'

// Το συμβολο αρχικοποιησης των buffers
#define BUFF_INIT '*'
#endif
