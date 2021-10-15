#! /bin/bash

# Ελεγξε τον αριθμο των ορισματων που δωθηκαν
if [ $# -ne 3 ]; then
   echo "ERROR: Invalid amount of arguments."
   exit 1
fi

# Τα ορισματα 1 ως 3, απ' τη γραμμη εντολων
inputfile=$1
inputdir=$2
numfiles=$3

# Regular expression - Κανει match αν τουλαχιστον ενας χαρακτηρας ενος token *δεν* ειναι στο range [0-9]
not_in_range='[^0-9]'

# Ελεγξε αν υπαρχει αρχειο με ονομα <inputfile>
if [ ! -f "$inputfile" ]; then
   echo "ERROR: File <$inputfile> does not exist."
   exit 2
fi

# Ελεγξε αν το τριτο argument ειναι αριθμος
if [[ $numfiles =~ $not_in_range ]]; then
   echo "ERROR: Third argument <$numfiles> is not a number."
   exit 3
fi

# Ελεγξε αν το τριτο argument ειναι θετικος αριθμος
if [[ $numfiles -eq 0 ]]; then
   echo "ERROR: Third argument <$numfiles> can't be 0."
   exit 4
fi

# Ελεγξε αν υπαρχει ηδη directory με ονομα <inputdir>
if [ -d "$inputdir" ]; then
   echo "ERROR: Directory <$inputdir> already exists."
   exit 5 
else
   mkdir "$inputdir"
fi

########################################################################################################

# Ορισε εναν πινακα για τα tokens των records και τις χωρες
declare -a token
declare -a present_countries
declare -a country_info

countries_counter=0

# Απ' το αρχειο <inputfile>
while read record;
do
   # Παρε το record και αποθηκευσε καθε λεξη ως διαφορετικο στοιχειο στον πινακα token
   IFS=' ' read -r -a token <<< "$record"
   # Παρε την χωρα απ' τον πινακα token
   country=${token[3]}
   # Αν δεν υπαρχει directory με το ονομα αυτης της χωρας
   if [ ! -d "$inputdir/$country" ]; then
      # Φτιαξε ενα directory με ονομα <country>
      mkdir "$inputdir/$country"
      # Φτιαξε τα text αρχεια απο 1 ως <numfiles>
      for ((i = 1 ; i <= $numfiles ; i++)); do
         touch "$inputdir/$country/$country-$i.txt"
      done
      # Προσθεσε τη χωρα στον πινακα χωρων στη μορφη "$country 1" και αυξησε τον μετρητη του πινακα χωρων κατα 1
      present_countries[$countries_counter]="$country 1"
      countries_counter=$((countries_counter+1))
      # Βαλε το record στο πρωτο text αρχειο που εφτιαξες
      if [ ${token[6]}="YES" ];then
         echo ${token[0]} ${token[1]} ${token[2]} ${token[4]} ${token[5]} ${token[6]} ${token[7]}>>"$inputdir/$country/$country-1.txt"
      else
         echo ${token[0]} ${token[1]} ${token[2]} ${token[4]} ${token[5]} ${token[6]}>>"$inputdir/$country/$country-1.txt"
      fi
   # Αν ομως υπαρχει ηδη το directory της χωρας
   else
      # Πηγαινε στον πινακα χωρων και διαβασε την καθε χωρα ξεχωριστα
      for ((j = 0 ; j < $countries_counter ; j++)); do      
           IFS=' ' read -r -a country_info <<< ${present_countries[$j]}
           # Αν η τρεχουσα χωρα του πινακα ταιριαζει με αυτην που ψαχνουμε
           if [ ${country_info[0]} = $country ]; then
              next_file=$((1+${country_info[1]}))
              # Αυξησε τον μετρητη κατα 1. Αν ξεπερασει το οριο <numfiles>, καν' τον reset σε 1.
              if [ $next_file -gt $numfiles ];then
                 next_file=1
              fi
              # Ανανεωσε το κουτακι της χωρας
              present_countries[$j]="$country $next_file"
              # Βαλε το record στο επομενο text αρχειο
              if [ ${token[6]}="YES" ];then
                 echo ${token[0]} ${token[1]} ${token[2]} ${token[4]} ${token[5]} ${token[6]} ${token[7]}>>"$inputdir/$country/$country-$next_file.txt"
              else
                 echo ${token[0]} ${token[1]} ${token[2]} ${token[4]} ${token[5]} ${token[6]}>>"$inputdir/$country/$country-$next_file.txt"
              fi      
              
              break
           fi
      done
   fi
done<$inputfile

exit 0

