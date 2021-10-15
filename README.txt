------------------------------------------------------------
WARNING: PLEASE OPEN ANY SUBSEQUENT FILES USING CAT COMMAND
INSTEAD OF NANO, OTHERWISE GREEK CHARACTERS WON'T APPEAR!
FOR INSTANCE: cat README.txt
------------------------------------------------------------


Α.Μ: 1115201300141
Ον/νυμο: Ευθύμιος Πατέλης


Πρωτοκολλο ανταλλαγης μηνυματων μεσω named pipes:

Απ' το parent process προς τα child processes, τα μηνυματα ειναι της μορφης <BytesToRead>_<ActionCode> <Packet-1> <Packet-n>, οπου:
<BytesToRead> ειναι το μεγεθος του μηνυματος σε bytes, μετα το συμβολο _
<ActionCode> ενα γραμμα με βαση τo οποιο το child process θα γνωριζει που αρκιβως αναφερονται τα υπολοιπα data
<Packet-1> <Packet-n> τα δεδομενα προς αναγνωση, χωρισμενα με space 

Απ' τα child processes προς το parent process, τα μηνυματα ειναι της μορφης <BytesToRead>_<Packet-1> <Packet-n>
δηλαδη χωρις το <ActionCode>, αφου ο πατερας με το που στειλει ενα μηνυμα στα παιδια, γνωριζει ακριβως τι 'σημαινουν' τα data που θα λαβει ως απαντηση



Queries:

Η υλοποιηση των queries εγινε ακριβως με τον τροπο που περιγραφεται στην εκφωνηση. Οσον αφορα το error checking:

/travelRequest citizenID Date CountryFrom CountryTo Virus

Επιστρεφει σφαλμα αν:
Το citizenID ή το Virus δεν υπαρχουν στο database του child process που διαχειριζεται τη χωρα CountryFrom, ή
Ενα τουλαχιστον απο τα Virus, CountryFrom ή CountryTo δεν υπαρχoυν στο database του parent process


/travelStats virusName date1 date2 [country]
 
Αν date1 > date2, τοτε το διαστημα [date1, date2] ειναι κενο, συνεπως επιστρεφονται 0 total/accepted/rejected requests
Επιστρεφει σφαλμα αν:
Ο ιος ή η χωρα δεν υπαρχουν στο database του parent process


/addVaccinationRecords country

Επιστρεφει σφαλμα αν:
Δεν υπαρχει χωρα με ονομα <country> στο directory των χωρων


/searchVaccinationStatus citizenID

Επιστρεφει σφαλμα αν:
Δεν υπαρχει πολιτης με αυτο το ID στα databases των παιδιων, ή αν υπαρχει αλλα το ID ειναι duplicated (δηλαδη αν 2 τουλαχιστον processes μοιραζονται μια εγγραφη με το ιδιο ID) 


/exit

Σε ενα loop, ο πατερας τερματιζει την λειτουργια καθε child process μεσω SIGKILL ακολουθουμενο απο ενα wait(NULL).
Επειτα, σβηνει τα FIFOs απ' το τρεχον directory, τυπωνει το δικο του log_file, ελευθερωνει τη μνημη και τερματιζει μεσω της exit()   


Signal Handling και Block/Serve/Unblock rotation:

Καθε process κληρονομει (απ' το class proc_mask) ενα πινακα μεγεθους 64 bits (ενα bit για καθε σημα). Ο signal handler, αναλογα με το σημα που κληθηκε,
θετει το αναλογο bit σε 1. Επειτα, η συναρτηση serve_signals(), ελεγχει ποια σηματα εκκρεμουν, τα εξυπηρετει, και επειτα θετει το αντιστοιχο bit του σηματος σε 0.
Μονο τα σηματα της εκφωνησης εχουν 'συνδεθει' μεσω της sigaction() με τους signal handlers. Τα υπολοιπα εχουν αφεθει στο default action.


Το parent process κανει ολα τα signals unblock, καθως περιμενει να διαβασει την εντολη απ' το πληκτρολογιο.
Με το που δωθει η εντολη, τα σηματα που δωθηκαν κατα την αναμονη εξυπηρετουνται. Ο,τι αλλο σημα δωθει κατα την εκτελεση της εντολης γινεται queued.
Επειτα, αφου η εντολη εκτελεσθει, τα σηματα που δωθηκαν κατα τη διαρκεια εκτελεσης της, εξυπηρετουνται επισης.

Τα child processes (σε ενα loop) κανουν ολα τα signals unblock και τα εξυπηρετουν. Επειτα, ελεγχουν αν εχει γραφτει κατι στο FIFO τους.
Αν οχι, ξανακανουν ελεγχο και εξυπηρετηση pending σηματων. Αν ναι, κανουν queue ο,τι αλλο σημα δωθει μεχρι να εκτελεσουν τη λειτουργια τους και επιστρεφουν
στον ελεγχο και την εξυπηρετηση των pending σηματων που δωθηκαν καθως τα processes ηταν απασχολημενα. 


Data Structures / Classes:

Απ' την πρωτη ασκηση, χρησιμοποιηθηκαν τα bloom filter, skip lists και linked lists, για τους ιους, τους πολιτες και τις χωρες. Αλλες κλασεις που υλοποιηθηκαν ειναι οι:

class proc_mask:
Κληρονομειται απ' το parent process (class mst) και child process (class wrk). Περιεχει εναν 64bit-ο πινακα για την αποθηκευση των states των σηματων, καθως και την τρεχουσα μασκα του process
σε μια μεταβλητη τυπου sigset_t

class filedes_set:
Κληρονομειται επισης απ' το parent και child process. Χρησιμοποιειται για την αποθηκευση των read και write file descriptors του καθε process, καθως και του τοπικα μεγιστου file descriptor

class requests:
Ενα request περιγραφεται απο μια χωρα, μια ημερομηνια και ενα state(accepted/rejected). Καθε ιος αποθηκευει τα δικα του requests σε μια linked list, μεσω του query /travelRequest
και τα τυπωνει μεσω του query /travelStats


class comm_handler:
Το class αυτο περιεχει διαφορες static συναρτησεις για τον ελεγχο εγκυροτητας των records, την αναλυση ενος string σε επιμερους tokens, τον ελεγχο των ορισματων απ' τη γραμμη εντολων, κλπ.


Λεπτομερειες συμπεριφορας προγραμματος:

1.) Το προγραμμα θα τερματισει αν το input_dir που δωθει ως ορισμα ειναι αδειο
2.) Αν οι χωρες ειναι λιγοτερες απ' τα child processes, τοτε θα δημιουργηθουν τοσα child processes οσα και οι χωρες
3.) Αν τα child processes δεχτουν SIGINT ή SIGQUIT, φτιαχνουν το log_file χωρις να τερματισουν
4.) Για να τερματισουν τα child processes, πρεπει ο πατερας να δεχτει SIGINT ή SIGQUIT, ειτε την εντολη /exit, οπου και στις 2 περιπτωσεις θα στειλει SIGKILL στα παιδια 



Bash Script:

Πρωτα ελεγχεται ο αριθμος ορισματων, το αν υπαρχει ηδη input directory με το ιδιο ονομα, αν υπαρχει το αρχειο εγγραφων και αν το τελευταιο ορισμα ειναι θετικος αριθμος.


Επειτα, δημιουργειται το input directory και το inputFile διαβαζεται ανα γραμμη. Αν δεν υπαρχει υπο-φακελος με το ονομα της χωρας που βρεθηκε στο record, τοτε δημιουργειται
ενας και μεσα σε αυτον τα αντοιστιχα text files. Επειτα, στο πρωτο text file εκχωρειται η εγγραφη και η χωρα αποθηκευεται σε εναν πινακα στη μορφη (country 1).
Αν υπαρχει ηδη ομως χωρα, τοτε αναζητειται η χωρα απ' τον πινακα χωρων. Βρισκεται η κωδικοποιηση της (country x), και το χ αυξανεται κατα 1. Επειτα στο text file country-x.txt
εκχωρειται η εγγραφη. Αν το χ γινει μεγαλυτερο του <numFiles>, τοτε γινεται reset σε 1.








