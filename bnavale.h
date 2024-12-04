/**
  \file bnavale.h
  \author Luigi Arena
*/

/** tipo di ogni cella in sui e' suddiviso il mare (area di gioco)
  -- EMPTY, cella vuota, 
  -- ONE contenente una nave lunga uno, 
  -- TWO, THREE parte di una nave lunga 2 o parte di una nave lunga 3 */

#define EMPTY  '.'
#define ONE    '1'
#define TWO    '2'
#define THREE  '3'
#define EMPTY_COLPITA  'X'
#define ONE_COLPITA    'a'
#define TWO_COLPITA    'b'
#define THREE_COLPITA  'c'



typedef struct area {
  /* matrice che rappresenta l'area di gioco */
  char ** a;
  /* numero di righe */
  unsigned N;
  /* numero di colonne */
  unsigned M;
  /* numero di navi presenti */
  unsigned n_navi;
} area_t;

/** alloca l'area di gioco  nXm suddivisa in celle di uguali dimensioni vuote
    con condizioni periodiche di bordo 
    (n-1 adiacente a 0 in tutte le dimensioni) 
    \param n numero delle righe
    \param m numero delle colonne

    \retval p puntatore alla nuova area (Se tutto e' andato bene)
    \retval NULL se si e' verificato un errore
*/
area_t * crea_area (unsigned n, unsigned m) ;

/** dealloca l'area di gioco in tutte le sue parti
    \param sea area da deallocare
*/
void distruggi_area (area_t * sea) ;

/**
   inizializza l'area di gioco tutta a EMPTY 
   \param sea l'area di gioco da inizializzare
*/
void init (area_t * sea);


/**
   inserisce nell'area di gioco una nuova nave lunga 1 (posizionamento casuale)
   \param sea l'area di gioco

   \retval 0 se l'inserimento è andato a buon fine
   \retval -1 se si è verificato un errore (es. non c'e' posto)
*/
int put_one (area_t* sea);

/**
   inserisce nell'area di gioco una nuova nave lunga 2 (posizionamento casuale e orientamento casuale della nave)
   \param sea area di gioco

   \retval 0 se l'inserimento è andato a buon fine
   \retval -1 se si è verificato un errore (es. non c'e' posto)
*/
int put_two (area_t * sea);

/**
   inserisce nell'area di gioco una nuova nave lunga 3 (posizionamento casuale e orientamento casuale della nave)
   \param sea area di gioco

   \retval 0 se l'inserimento è andato a buon fine
   \retval -1 se si è verificato un errore (es. non c'e' posto)
*/
int put_three (area_t* sea);

/**
   stampa fomattata del mare su stdout (FORNITO DAI DOCENTI NON MODIFICARE -- file bnavale_docenti.c)
   \param sea area da stampare
*/
void print_area (area_t* sea);

/**
  conta le navi di tipo k in sea, incluse navi colpite o affondate (FORNITO DAI DOCENTI NON MODIFICARE -- file bnavale_docenti.c)
  \param k tipo di navi da contare ONE, TWO o THREE

  \retval n numero delle navi di tipo k, o -1 se k non è ONE, TWO o THREE
*/
int conta_navi (area_t* sea, char k);

/** esiti possibili della funzione esegui_mossa */
#define ERRORE          130
#define ACQUA            131
#define AFFONDATA_ONE    132
#define AFFONDATA_TWO   133
#define AFFONDATA_THREE 134
#define COLPITA_TWO  135
#define COLPITA_THREE  136

/**
   Controlla l'esito del colpo di coordinate (x,y) nell'area di gioco
   \param sea area di gioco
   \param x,y coordinate del colpo

   \retval e uno dei valori di esito definiti sopra 

 */
int esegui_mossa (area_t * sea, int x, int y);

/**
   stampa un messaggio di errore in base all'esito (fornita dai docenti nel file bnavale_docenti.c NON MODIFICARE  -- file bnavale_docenti.c)
   \param e uno dei valori di esito definiti sopra */
   void stampa_esito (int e);

/** funzione crea le due aree di gioco e gestisce una partita con il computer. Si puo' assumere che l'umano giochi per primo. 
    \param nXm ampiezza area di gioco
    \param n_navi1 numero navi da 1 inserite casulamente nelle due aree
    \param n_navi2 numero navi da 2 inserite casulamente nelle due aree
    \param n_navi3 numero navi da 3 inserite casulamente nelle due aree */
void gioca(unsigned n, unsigned m, unsigned n_navi1, unsigned n_navi2, unsigned n_navi3);

int bnavale_main (int argc, char ** argv);
