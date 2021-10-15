COMP=g++

FLAGS= -c -std=c++0x

# Φτιαξε ενα object file για καθε source file και τα δυο εκτελεσιμα
all: request.o bloomfilter.o countries.o comm_handler.o filedes_set.o travelMonitor.o mst.o Monitor.o wrk.o child.o proc_mask.o travelMonitor Monitor

# Συνδεσε τα object files σε ενα exe
travelMonitor: common.h request.cpp bloomfilter.cpp countries.cpp citizens.cpp skiplist.cpp virus.cpp comm_handler.cpp filedes_set.cpp proc_mask.cpp child.cpp mst.cpp travelMonitor.cpp
	$(COMP) text_files.cpp request.cpp bloomfilter.cpp countries.cpp citizens.cpp skiplist.cpp virus.cpp comm_handler.cpp filedes_set.cpp proc_mask.cpp child.cpp mst.cpp travelMonitor.cpp -o travelMonitor

Monitor: common.h filedes_set.cpp request.cpp text_files.cpp bloomfilter.cpp countries.cpp citizens.cpp skiplist.cpp virus.cpp comm_handler.cpp proc_mask.cpp wrk.cpp Monitor.cpp
	$(COMP) filedes_set.cpp request.cpp text_files.cpp bloomfilter.cpp countries.cpp citizens.cpp skiplist.cpp virus.cpp comm_handler.cpp proc_mask.cpp wrk.cpp Monitor.cpp -o Monitor

# Φτιαξε το object file μονο για το συγκεκριμενο source file

bloomfilter.o: bloomfilter.h bloomfilter.cpp
	$(COMP) $(FLAGS) bloomfilter.cpp

countries.o: countries.h countries.cpp
	$(COMP) $(FLAGS) countries.cpp

citizens.o: citizens.h citizens.cpp
	$(COMP) $(FLAGS) citizens.cpp

skiplist.o: skiplist.h skiplist.cpp
	$(COMP) $(FLAGS) skiplist.cpp

virus.o: virus.h virus.cpp
	$(COMP) $(FLAGS) virus.cpp

child.o: child.h child.cpp
	$(COMP) $(FLAGS) child.cpp

filedes_set.o: filedes_set.h filedes_set.cpp
	$(COMP) $(FLAGS) filedes_set.cpp

proc_mask.o: proc_mask.cpp
	$(COMP) $(FLAGS) proc_mask.cpp

travelMonitor.o: travelMonitor.cpp mst.cpp
	$(COMP) $(FLAGS) travelMonitor.cpp

mst.o: mst.h mst.cpp
	$(COMP) $(FLAGS) mst.cpp

Monitor.o: Monitor.cpp wrk.h wrk.cpp
	$(COMP) $(FLAGS) Monitor.cpp

wrk.o: wrk.h wrk.cpp
	$(COMP) $(FLAGS) wrk.cpp

comm_handler.o: comm_handler.h comm_handler.cpp
	$(COMP) $(FLAGS) comm_handler.cpp

text_files.o: text_files.h text_files.cpp
	$(COMP) $(FLAGS) text_files.cpp

request.o: request.h request.cpp
	$(COMP) $(FLAGS) request.cpp


# Σβησε ολα τα αρχεια που κανουν pattern matching με .ο (δηλαδη ολα τα object files) και το τα παλια εκτελεσιμα
clean:
	rm *.o travelMonitor Monitor
