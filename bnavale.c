/**
  \file bnavale.c
  \author Luigi Arena
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "bnavale.h"
// bnavale_docenti.c - da includere al posto di bnavale.h per far girare il main
// #include "bnavale_docenti.c"

// Definizione dei colori
#define ANSI_COLOR_GREY     "\x1b[30;1m"
#define ANSI_COLOR_RED      "\x1b[31m"
#define ANSI_COLOR_GREEN    "\x1b[32m"
#define ANSI_COLOR_YELLOW   "\x1b[33m"
#define ANSI_COLOR_BLUE     "\x1b[34m"
#define ANSI_COLOR_MAGENTA  "\x1b[35m"
#define ANSI_COLOR_CYAN     "\x1b[36m"
#define ANSI_COLOR_RESET    "\x1b[0m"

// Definizione delle Strutture
/**
  le hidden_area_t sono strutture che vengono create in automatico associate a ciascun
  area_t, servono per permettere il riempimento casuale in maniera ottimizzata.
  La loro implementazione è nascosta dall'uso di bnavale.h
*/
typedef struct hidden_area {
  area_t * area;
  unsigned * sea_scape;
  unsigned edge_one;
  unsigned edge_two;
  unsigned edge_three;
  struct hidden_area * next;
} hidden_area_t;

/**
  coord è una struttura che contiene 2 gruppi di coordinate, servono per memorizzare
  due celle adiacenti nella ricerca di put_two e put_three
*/
typedef struct coord {
  unsigned x1;
  unsigned y1;
  unsigned x2;
  unsigned y2;
} coord_t;

/**
  cpu è la struttura che contiene i dati necessari al funzionamento della AI del computer

  @area_t * copy é una copia dell'area_t dell'avversario umano
  @memory è un sea_scape che viene utilizzato per la selezione delle celle nelle mosse del computer
  @edge è il limitatore che segna il confine tra le celle in memory selezionabili
  @target1, @target2 e @target3 sono variabili che indicano al computer se ha pescato una nave da 2
  oppure da 3 e come muoversi per selezionarle le celle più giuste
*/
struct {
  area_t * copy;
  unsigned * memory;
  unsigned edge;
  int target1;
  int target2;
  int target3;
} cpu;

/**
  world è una lista che contiene tutti gli hidden_area_t creati e quindi anche tutte le area_t
*/
hidden_area_t * world = NULL;

// Prototipi delle mie funzioni
void conta_world (void);
hidden_area_t * find_area (area_t * sea);
void init_cpu (area_t * sea_player);
int check_cell (area_t * sea, unsigned cell_r, unsigned cell_c, char cell_case);
coord_t * swap_vec (unsigned r_cell_r, unsigned r_cell_c, unsigned n, unsigned m);
void print_world_utf8 (hidden_area_t * world, unsigned n, unsigned m);
void segna_vuote (area_t * sea, unsigned cell_r, unsigned cell_c);
void update_memory (int cell);
int gioca_cpu();
void stampa_intestazione (void);

/**
    Crea e inizializza un area_t e la sua hidden_area_t associata, e le inizializza
*/
area_t * crea_area (unsigned n, unsigned m) {
  int i;
  area_t * sea = malloc(sizeof(area_t));
  char ** a = malloc(n*sizeof(char *));
  for (i=0; i<n; i++) {
    *(a+i) = malloc(m*sizeof(char));
  }
  sea->a = a;
  sea->N = n;
  sea->M = m;
  sea->n_navi = 0;
  // chiamo floater i puntatori alle hidden_area_t
  struct hidden_area * floater = malloc(sizeof(hidden_area_t));
  unsigned * sea_scape = malloc((n*m)*sizeof(unsigned));
  floater->area = sea;
  floater->sea_scape = sea_scape;
  floater->edge_one = n*m;
  floater->edge_two = n*m;
  floater->edge_three = n*m;
  floater->next = world;
  world = floater;
  init(sea);
  return sea;
}

/**
    Dealloca un area_t e la sua hidden_area_t associata
*/
void distruggi_area (area_t * sea) {
  if (sea == NULL) return;
  int i;
  hidden_area_t * floater = find_area(sea);
  for (i=0; i<sea->N; i++) {
    free(sea->a[i]);
  }
  free(sea->a);
  free(sea);
  free(floater->sea_scape);
  world = floater->next;
  free(floater);
}

/**
    Inzializza un'area_t e la sua hidden_area_t associata
*/
void init (area_t * sea) {
  if (sea == NULL) return;
  unsigned i, j;
  unsigned nm = sea->N*sea->M;
  hidden_area_t * floater = find_area(sea);
  for (i=0; i<sea->N; i++) {
    for (j=0; j<sea->M; j++) {
      sea->a[i][j] = EMPTY;
    }
  }
  sea->n_navi = 0;
  for (i=0; i<nm; i++) {
    floater->sea_scape[i] = i;
  }
  floater->edge_one = nm;
  floater->edge_two = nm;
  floater->edge_three = nm;
}

/**
    Inserisce nell'area di gioco una nuova nave lunga 1 (posizionamento casuale) ma
    con l'aiuto della hidden_area_t cerca di ottimizzare la selezione delle celle vuote
*/
int put_one (area_t* sea) {
  if (sea == NULL) return -1;
  int check = 0;
  unsigned m = sea->M;
  unsigned index, cell, cell_r, cell_c;
  hidden_area_t * floater = find_area(sea);

  while ((floater->edge_one>0) && (!check)) {
    index = rand()%floater->edge_one;
    cell = floater->sea_scape[index];
    cell_r = cell/m;
    cell_c = cell%m;
    if (check_cell(sea, cell_r, cell_c, EMPTY) == 1) {
      sea->a[cell_r][cell_c] = ONE;
      sea->n_navi++;
      check = 1;
    }
    floater->sea_scape[index] = floater->sea_scape[floater->edge_one-1];
    floater->sea_scape[floater->edge_one-1] = cell;
    floater->edge_one--;
    floater->edge_two--;
    floater->edge_three--;
  }

  if (check) return 0;
  else return -1;
}

/**
    Inserisce nell'area di gioco una nuova nave lunga 2 (posizionamento casuale) ma
    con l'aiuto della hidden_area_t cerca di ottimizzare la selezione delle celle vuote
    e adatte all'inserimento di una nave lunga 2
*/
int put_two (area_t * sea) {
  if (sea == NULL) return -1;
  int check = 0;
  unsigned n = sea->N, m = sea->M;
  unsigned index, cell, cell_r, cell_c;
  hidden_area_t * floater = find_area(sea);

  while ((floater->edge_two>0) && (!check)) {
    index = rand()%floater->edge_two;
    cell = floater->sea_scape[index];
    cell_r = cell/m;
    cell_c = cell%m;
    if (check_cell(sea, cell_r, cell_c, EMPTY) == 1) {
      coord_t * vec = swap_vec(cell_r, cell_c, n, m);
      for (int i=0; (i<4) && (!check); i++) {
        if (check_cell(sea, vec[i].x1, vec[i].y1, EMPTY) == 1) {
          sea->a[cell_r][cell_c] = TWO;
          sea->a[vec[i].x1][vec[i].y1] = TWO;
          sea->n_navi++;
          check = 1;
        }
      }
    }
    floater->sea_scape[index] = floater->sea_scape[floater->edge_two-1];
    floater->sea_scape[floater->edge_two-1] = cell;
    if (check || sea->a[cell_r][cell_c] != EMPTY){
      floater->sea_scape[floater->edge_two-1] = floater->sea_scape[floater->edge_one-1];
      floater->sea_scape[floater->edge_one-1] = cell;
      floater->edge_one--;
    }
    floater->edge_two--;
    floater->edge_three--;
  }

  if (check) return 0;
  else return -1;
}

/**
    Inserisce nell'area di gioco una nuova nave lunga 3 (posizionamento casuale) ma
    con l'aiuto della hidden_area_t cerca di ottimizzare la selezione delle celle vuote
    e adatte all'inserimento di una nave lunga 3
*/
int put_three (area_t* sea) {
  if (sea == NULL) return -1;
  int check = 0;
  unsigned n = sea->N, m = sea->M;
  unsigned index, cell, cell_r, cell_c;
  hidden_area_t * floater = find_area(sea);

  while ((floater->edge_three>0) && (!check)) {
    index = rand()%floater->edge_three;
    cell = floater->sea_scape[index];
    cell_r = cell/m;
    cell_c = cell%m;
    if (check_cell(sea, cell_r, cell_c, EMPTY) == 1) {
      coord_t * vec = swap_vec(cell_r, cell_c, n, m);
      for (int i=0; (i<4) && (!check); i++) {
        if (check_cell(sea, vec[i].x1, vec[i].y1, EMPTY) == 1 &&
            check_cell(sea, vec[i].x2, vec[i].y2, EMPTY) == 1) {
          sea->a[cell_r][cell_c] = THREE;
          sea->a[vec[i].x1][vec[i].y1] = THREE;
          sea->a[vec[i].x2][vec[i].y2] = THREE;
          sea->n_navi++;
          check = 1;
        }
      }
    }
    floater->sea_scape[index] = floater->sea_scape[floater->edge_three-1];
    floater->sea_scape[floater->edge_three-1] = cell;
    if (check || sea->a[cell_r][cell_c] != EMPTY){
      floater->sea_scape[floater->edge_three-1] = floater->sea_scape[floater->edge_one-1];
      floater->sea_scape[floater->edge_one-1] = cell;
      floater->edge_one--;
      floater->edge_two--;
    }
    floater->edge_three--;
  }

  if (check) return 0;
  else return -1;
}

/**
   Controlla l'esito del colpo di coordinate (x,y) nell'area di gioco
*/
int esegui_mossa (area_t * sea, int x, int y) {
  int e;

  switch (sea->a[x][y]) {
    case EMPTY:
      sea->a[x][y] = EMPTY_COLPITA;
      e = ACQUA;
      break;
    case ONE:
      sea->a[x][y] = ONE_COLPITA;
      e = AFFONDATA_ONE;
      sea->n_navi--;
      break;
    case TWO:
      if (check_cell(sea, x, y, TWO) == 1) {
        sea->a[x][y] = TWO_COLPITA;
        e = COLPITA_TWO;
      } else {
        sea->a[x][y] = TWO_COLPITA;
        e = AFFONDATA_TWO;
        sea->n_navi--;
      }
      break;
    case THREE:
      if (check_cell(sea, x, y, THREE) == 1) {
        sea->a[x][y] = THREE_COLPITA;
        e = COLPITA_THREE;
      } else {
        sea->a[x][y] = THREE_COLPITA;
        e = AFFONDATA_THREE;
        sea->n_navi--;
      }
      break;
    default:
      e = ERRORE;
      break;
  }

  return e;
}

/**
    Crea le due aree di gioco e gestisce una partita con il computer
    Sfrutta la struttura 'cpu' e le funzioni associate per simulare un'AI
    Viene usata dalla funzione main per lanciare le partite
*/
void gioca (unsigned n, unsigned m, unsigned n_navi1, unsigned n_navi2, unsigned n_navi3) {
  int i, x_player, x_cpu, cell, mossa_player=0, mossa_cpu=0;
  char y_player, y_cpu, buffer[5];

  // Creo le aree di gioco, una per il giocatore umano e una per CPU
  area_t * sea_player = crea_area(n, m);
  area_t * sea_cpu = crea_area(n, m);

  // Provo a inserire il numero giusto di navi per al massimo 10 volte, esco se ci riesco
  int count = 0;
  while (count<=10 && (conta_navi(sea_player, ONE) != n_navi1 || conta_navi(sea_cpu, ONE) != n_navi1 ||
      conta_navi(sea_player, TWO) != n_navi2 || conta_navi(sea_cpu, TWO) != n_navi2 ||
      conta_navi(sea_player, THREE) != n_navi3 || conta_navi(sea_cpu, THREE) != n_navi3))
  {
    // Inizializzo le aree di gioco
    init(sea_player);
    init(sea_cpu);
    // Inserisco le navi
    for(i=0; i<n_navi1; i++) {
      put_one(sea_player);
      put_one(sea_cpu);
    }
    for(i=0; i<n_navi2; i++) {
      put_two(sea_player);
      put_two(sea_cpu);
    }
    for(i=0; i<n_navi3; i++) {
      put_three(sea_player);
      put_three(sea_cpu);
    }
    count++;
  }

  // Controllo se l'inserimento delle navi è avvenuto con successo altrimenti esco da gioca
  if (conta_navi(sea_player, ONE) != n_navi1 || conta_navi(sea_cpu, ONE) != n_navi1 ||
      conta_navi(sea_player, TWO) != n_navi2 || conta_navi(sea_cpu, TWO) != n_navi2 ||
      conta_navi(sea_player, THREE) != n_navi3 || conta_navi(sea_cpu, THREE) != n_navi3)
  {
    printf("Tentativo %d\n", i+1);
    fprintf(stdout, "\nMi dispiace, non sono riuscito ad inserire tutte le navi...\n");
    fprintf(stdout, "Premi " ANSI_COLOR_BLUE "INVIO" ANSI_COLOR_RESET " per continuare\n\n");
    while(getchar()!='\n');
    return;
  }

  // Inizializzo la memoria del giocatore CPU
  init_cpu(sea_player);

  // Inizia la partita
  while (sea_cpu->n_navi > 0 && sea_player->n_navi > 0) {
    // Turno PLAYER
    system("clear");
    print_world_utf8(world, n, m);
    mossa_player = 0; x_player=0; y_player=0;
    // Ripeto la richiesta di una casella finché il valore non è valido
    while(!mossa_player) {
      fprintf(stdout, ANSI_COLOR_GREEN "PLAYER" ANSI_COLOR_RESET ", fai una mossa: ");
      fgets(buffer, 5, stdin);
      // Posso uscire digitando 'esci'
      if (strcmp(buffer, "esci")==0) {
        system("clear");
        print_world_utf8(world, n, m);
        printf("Arrivederci!  (\u0298\u203f\u0298)\u256f\n\n");
        exit(0);
      }
      sscanf(buffer, " %c%d", &y_player, &x_player);
      if (y_player>='a' && y_player<'z') y_player = y_player-'a'+'A';
      if ((y_player>='A' && y_player<='A'+m) && (x_player>=1 && x_player<=n)) {
        x_player = x_player-1;
        y_player = y_player-'A';
        mossa_player = esegui_mossa(sea_cpu, x_player, y_player);
      } else {
        fprintf(stdout, ANSI_COLOR_RED "Mossa non valida!\n" ANSI_COLOR_RESET);
        fprintf(stdout, "Premi " ANSI_COLOR_BLUE "INVIO" ANSI_COLOR_RESET " per continuare");
        while(getchar()!='\n');
        system("clear");
        print_world_utf8(world, n, m);
      }
    }
    system("clear");
    print_world_utf8(world, n, m);
    fprintf(stdout, "Risultato: "); stampa_esito(mossa_player); fprintf(stdout, "\n");
    // Turno CPU
    cell = gioca_cpu();
    x_cpu = cell/m;
    y_cpu=cell%m;
    mossa_cpu = esegui_mossa(sea_player, x_cpu, y_cpu);
    // Cancello e ristampo tutto per aggiorare il campo con le mosse effettuate
    system("clear");
    print_world_utf8(world, n, m);
    fprintf(stdout, ANSI_COLOR_GREEN "PLAYER" ANSI_COLOR_RESET " sceglie");
    fprintf(stdout, " %c%d\n", y_player+'A', x_player+1);
    fprintf(stdout, "Risultato: "); stampa_esito(mossa_player); fprintf(stdout, "\n");
    fprintf(stdout, ANSI_COLOR_GREEN "CPU" ANSI_COLOR_RESET " sceglie");
    fprintf(stdout, " %c%d\n", y_cpu+'A', x_cpu+1);
    fprintf(stdout, "Risultato: "); stampa_esito(mossa_cpu); fprintf(stdout, "\n");
    // Aspetto un input per andare al prossimo turno
    printf("Premi " ANSI_COLOR_BLUE "INVIO" ANSI_COLOR_RESET " per continuare\n");
    while(getchar()!='\n');
  }
  system("clear");
  print_world_utf8(world, n, m);
  /** Quando le navi finisco stampo il vincitore oppure un errore se la selezione manuale é
    stata scorretta
  */
  if (sea_cpu->n_navi == 0 && sea_player->n_navi == 0) fprintf(stdout, "Qualcosa e' andato storto.   (0 _ 0)\n\n");
  else if (sea_cpu->n_navi == 0) fprintf(stdout, "Complimenti, hai vinto!   *\\(^ o ^)/*\n\n");
  else if (sea_player->n_navi == 0) fprintf(stdout, "Peccato, hai perso.   (\u2565 \ufe4f \u2565)\n\n");

  // Aspetto un input per uscire fuori dalla funzione 'gioca' e tornare alla schermata principale
  printf("Premi " ANSI_COLOR_BLUE "INVIO" ANSI_COLOR_RESET " per continuare\n");
  getchar();
  return;
}

/**
   Cerca in World l'hidden_area_t associata all'area_t data come argomento
   \param sea un'area di gioco

   \retval p puntatore alla hidden_area_t di sea
*/
hidden_area_t * find_area (area_t * sea) {
  hidden_area_t * p = world;
  int found = 0;
  while (found == 0 && p != NULL) {
    if(p->area == sea) found = 1;
    else p = p->next;
  }
  return p;
}

/**
   Funzione di aiuto per l'inserimento delle navi e per la selezione delle cell_case
   da parte di CPU durante una partita
   \param sea area di gioco
   \param cell_r,cell_c riga e colonna della cella
   \param cell_case caso che si vuole analizzare

   \retval check se cell_case é EMPTY allora check vale 1 se tutte le celle adiacenti
          sono EMPTY, 0 altrimenti
          se cell_case vale TWO o THREE allora check vale 1 se esiste almeno una cella
          intorno che vale come cell_case, ovvero se le navi sono state colpite ma
          non affondate
*/
int check_cell (area_t * sea, unsigned cell_r, unsigned cell_c, char cell_case) {
  int check = 0;
  unsigned n = sea->N, m = sea->M;
  // Controllo la cella centrale
  if (sea->a[cell_r][cell_c] != cell_case) return check;
  switch(cell_case) {
    case EMPTY:
    // Controllo le celle agli angoli e ai lati
    if (sea->a[(cell_r+n-1)%n][(cell_c+m-1)%m] == cell_case &&
        sea->a[(cell_r+n-1)%n][(cell_c+m+1)%m] == cell_case &&
        sea->a[(cell_r+n+1)%n][(cell_c+m-1)%m] == cell_case &&
        sea->a[(cell_r+n+1)%n][(cell_c+m+1)%m] == cell_case &&
        sea->a[(cell_r+n-1)%n][(cell_c+m)%m] == cell_case &&
        sea->a[(cell_r+n)%n][(cell_c+m-1)%m] == cell_case &&
        sea->a[(cell_r+n)%n][(cell_c+m+1)%m] == cell_case &&
        sea->a[(cell_r+n+1)%n][(cell_c+m)%m] == cell_case
    ) check = 1;
    break;
  case TWO:
    // Controllo le celle ai lati
    if (sea->a[(cell_r+n-1)%n][(cell_c+m)%m] == cell_case ||
        sea->a[(cell_r+n)%n][(cell_c+m-1)%m] == cell_case ||
        sea->a[(cell_r+n)%n][(cell_c+m+1)%m] == cell_case ||
        sea->a[(cell_r+n+1)%n][(cell_c+m)%m] == cell_case
    ) check = 1;
    break;
  case THREE:
    // Controllo le celle ai lati
    if (sea->a[(cell_r+n-1)%n][(cell_c+m)%m] == cell_case ||
        sea->a[(cell_r+n)%n][(cell_c+m-1)%m] == cell_case ||
        sea->a[(cell_r+n)%n][(cell_c+m+1)%m] == cell_case ||
        sea->a[(cell_r+n+1)%n][(cell_c+m)%m] == cell_case
    ) check = 1;
    else {
      // Se esiste un 'c' adiacente controllo che non esista un THREE vicino
        if (sea->a[(cell_r+n-1)%n][(cell_c+m)%m] == 'c' &&
            sea->a[(cell_r+n-2)%n][(cell_c+m)%m] == cell_case
        ) check = 1;
        else if (sea->a[(cell_r+n)%n][(cell_c+m-1)%m] == 'c' &&
            sea->a[(cell_r+n)%n][(cell_c+m-2)%m] == cell_case
        ) check = 1;
        else if (sea->a[(cell_r+n)%n][(cell_c+m+1)%m] == 'c' &&
            sea->a[(cell_r+n)%n][(cell_c+m+2)%m] == cell_case
        ) check = 1;
        else if (sea->a[(cell_r+n+1)%n][(cell_c+m)%m] == 'c' &&
            sea->a[(cell_r+n+2)%n][(cell_c+m)%m] == cell_case
        ) check = 1;
    }
    break;
  }
  return check;
}

/**
   Per una cella di coordinate cell_r e cell_c restituisce un vettore di 8 celle associate
   per i possibili inserimenti di navi da TWO e da THREE
   \param cell_r,cell_c riga e colonna della cella
   \param n,m numero di righe e colonne dell'area di gioco

   \retval vec vettore delle possibili celle per TWO e THREE
*/
coord_t * swap_vec (unsigned cell_r, unsigned cell_c, unsigned n, unsigned m) {
    int n_rand;
    coord_t temp;
    coord_t * vec = malloc(4*sizeof(coord_t));

    // Celle in alto
    vec[0].x1 = (cell_r+n-1)%n; vec[0].y1 = (cell_c+m)%m;
    vec[0].x2 = (cell_r+n-2)%n; vec[0].y2 = (cell_c+m)%m;
    // Celle in basso
    vec[1].x1 = (cell_r+n+1)%n; vec[1].y1 = (cell_c+m)%m;
    vec[1].x2 = (cell_r+n+2)%n; vec[1].y2 = (cell_c+m)%m;
    // Celle a sinistra
    vec[2].x1 = (cell_r+n)%n; vec[2].y1 =(cell_c+m-1)%m;
    vec[2].x2 = (cell_r+n)%n; vec[2].y2 =(cell_c+m-2)%m;
    // Celle a destra
    vec[3].x1 = (cell_r+n)%n; vec[3].y1 = (cell_c+m+1)%m;
    vec[3].x2 = (cell_r+n)%n; vec[3].y2 = (cell_c+m+2)%m;

    // Mescolo gli elementi del vettore per la selezione casuale
    for (int i=4; i>0; i--) {
      n_rand = rand()%i;
      temp = vec[n_rand];
      vec[n_rand] = vec[i-1];
      vec[i-1] = temp;
    }
    return vec;
}

/**
   Inizializza la memoria di CPU
   \param sea_player area di gioco dell'umano
*/
void init_cpu (area_t * sea_player) {
  int i, j;
  int n = sea_player->N, m = sea_player->M;
  cpu.edge = n*m;
  // I target valgono -1 quando non utlizzati
  cpu.target1 = -1;
  cpu.target2 = -1;
  cpu.target3 = -1;
  area_t * copy = malloc(sizeof(area_t));
  copy->N = n; copy->M = m; copy->n_navi = sea_player->n_navi;
  char ** a = malloc(n*sizeof(unsigned *));
  for (i=0; i<n; i++) {
    a[i] = malloc(m*sizeof(unsigned));
    for (j=0; j<m; j++) a[i][j] = sea_player->a[i][j];
  }
  copy->a = a;
  unsigned * memory = malloc(cpu.edge*sizeof(unsigned *));
  for (i=0; i<cpu.edge; i++) {
    memory[i] = i;
  }
  /** Associo a CPU una copia dell'area del giocatore umano, in questo modo il computer
    potrà fare previsioni senza fare mosse a caso
  */
  cpu.copy = copy;
  cpu.memory = memory;
  return;
}

/**
   Questa funzione viene chiamata dalla funzione 'gioca' quando è il turno di CPU
   Serve per far selezionare a CPU in ogni turno la cella migliore in base alle mosse
   precedenti

   \retval cell cella selezionata da CPU o -1 se non ci sono più celle selezionabili
*/
int gioca_cpu() {
  if(cpu.edge < 1) return -1;
  int index, cell, cell_r, cell_c, mossa, value = EMPTY_COLPITA;
  unsigned n = cpu.copy->N, m = cpu.copy->M;
  int h[4][2] = {{n-1,m},{n,m-1},{n,m+1},{n+1,m}};

  // Inizia il calcolo
  while (value==EMPTY_COLPITA && cpu.edge>0) {
    if (cpu.target1 == -1) {
      // Non ho nessun target, scelgo a caso dalla memoria (celle selezionabili)
      index = rand()%cpu.edge;
      cell = cpu.memory[index];
    } else if (cpu.target2 == -1) {
      /** Ho un target1 quindi cerco una casella adiacene a target1, sia nel caso
        TWO che nel caso THREE
      */
      index = rand()%4;
      cell = (((cpu.target1/m)+h[index][0])%m)*n + ((cpu.target1%m)+h[index][1])%m;
    } else if (cpu.target3 == -1) {
      // Ho un target2 quindi sono nel caso THREE e cerco una terza cella
      /** Avendo selezionato già due celle di tre, ho due possibilità, quindi selezionato
        due celle, una la provo subito e l'altra nel caso al prossimo Turno
      */
      if (cpu.target1/m == cpu.target2/m) {
        // Caso THREE orizzontale
        if (cpu.target2 == ((cpu.target1/m)*n + (cpu.target1%m+m-1)%m)) {
          cell = (cpu.target1/m)*n + (cpu.target1%m+m+1)%m;
          cpu.target3 = (cpu.target2/m)*n + (cpu.target2%m+m-1)%m;
        } else {
          cell = (cpu.target1/m)*n + (cpu.target1%m+m-1)%m;
          cpu.target3 = (cpu.target2/m)*n + (cpu.target2%m+m+1)%m;
        }
      } else {
        // Caso THREE verticale
        if (cpu.target2 == (((cpu.target1/m+n-1)%m)*n + cpu.target1%m)) {
          cell = ((cpu.target1/m+n+1)%m)*n + cpu.target1%m;
          cpu.target3 = ((cpu.target2/m+n-1)%m)*n + cpu.target2%m;
        } else {
          cell = ((cpu.target1/m+n-1)%m)*n + cpu.target1%m;
          cpu.target3 = ((cpu.target2/m+n+1)%m)*n + cpu.target2%m;
        }
      }
    } else {
      /** Ho un target3 quindi nella selezione precedente ho preso una cella sbagliata
        e provo con il secondo tentativo
      */
      cell = cpu.target3;
    }
    cell_r = cell/m; cell_c = cell%m;
    value = cpu.copy->a[cell_r][cell_c];
    // Aggiorno lo schema di CPU con la nuova cella selezionata
    update_memory(cell);
  }

  /** Eseguo la mossa sulla copia dell'area dell'umano, così CPU può capire che cosa succederà
    e quali celle può scartare per i prossimi turni in base al caso
  */
  mossa = esegui_mossa(cpu.copy, cell_r, cell_c);
  switch (mossa) {
    case AFFONDATA_ONE:
      segna_vuote(cpu.copy, cell_r, cell_c);
      break;
    case AFFONDATA_TWO:
      segna_vuote(cpu.copy, cpu.target1/m, cpu.target1%m);
      segna_vuote(cpu.copy, cell_r, cell_c);
      cpu.target1 = -1;
      break;
    case AFFONDATA_THREE:
      segna_vuote(cpu.copy, cpu.target2/m, cpu.target2%m);
      segna_vuote(cpu.copy, cpu.target1/m, cpu.target1%m);
      segna_vuote(cpu.copy, cell_r, cell_c);
      cpu.target1 = -1; cpu.target2 = -1; cpu.target3 = -1;
      break;
    case COLPITA_TWO: case COLPITA_THREE:
      if (cpu.target1 == -1) cpu.target1 = cell;
      else cpu.target2 = cell;
      break;
  }
  return cell;
}

/**
   Accede alla struttura cpu, cerca la posizione di una cella dentro memory e se non era
   già stata selezionata la sposta nell'area di quelle già selezionate, dopo 'edge'
   \param cell cella da eliminare dalla lista delle selezioni
*/
void update_memory (int cell) {
  int index = -1;
  for (int i=0; i<cpu.edge; i++) {
    if (cpu.memory[i] == cell) index = i;
  }
  if (index != -1) {
    cpu.memory[index] = cpu.memory[cpu.edge-1];
    cpu.memory[cpu.edge-1] = cell;
    cpu.edge--;
  }
  return;
}

/**
   Data una cella in sea di coordinate cell_r e cell_c, segna come colpite tutte le caselle
   adiacenti. Quando la CPU trova una nave usa quest funzione per rendere non selezionabili
   le caselle adiacenti a quelle della nave
   \param sea area di gioco
   \param cell_r,cell_c riga e colonna della cella
*/
void segna_vuote (area_t * sea, unsigned cell_r, unsigned cell_c) {
  unsigned N = sea->N, M = sea->M;
  unsigned n1 = (cell_r+N-1)%M, n2 = (cell_r+N)%M, n3 = (cell_r+N+1)%M;
  unsigned m1 = (cell_c+M-1)%M, m2 = (cell_c+M)%M, m3 = (cell_c+M+1)%M;

  if (sea->a[n1][m1] == EMPTY) sea->a[n1][m1] = EMPTY_COLPITA;
  if (sea->a[n1][m2] == EMPTY) sea->a[n1][m2] = EMPTY_COLPITA;
  if (sea->a[n1][m3] == EMPTY) sea->a[n1][m3] = EMPTY_COLPITA;
  if (sea->a[n2][m1] == EMPTY) sea->a[n2][m1] = EMPTY_COLPITA;
  if (sea->a[n2][m3] == EMPTY) sea->a[n2][m3] = EMPTY_COLPITA;
  if (sea->a[n3][m1] == EMPTY) sea->a[n3][m1] = EMPTY_COLPITA;
  if (sea->a[n3][m2] == EMPTY) sea->a[n3][m2] = EMPTY_COLPITA;
  if (sea->a[n3][m3] == EMPTY) sea->a[n3][m3] = EMPTY_COLPITA;

  return;
}

/**
    funzione di test, conta tutte le hidden_area_t/area_t create e stampa il risultato
*/
void conta_world (void) {
  hidden_area_t * p = world;
  unsigned count = 0;
  while (p != NULL) {
    count = count+1;
    p = p->next;
  }
  printf("World counts %d seas.\n", count);
}

/**
    stampa fomattata dell'area di gioco su stdout in standard UNICODE
*/
void print_world_utf8 (hidden_area_t * world, unsigned n, unsigned m) {
    int i, j;
    char cell;
    hidden_area_t * floater_cpu = world;
    hidden_area_t * floater_player = world->next;
    area_t * sea_cpu = floater_cpu->area;
    area_t * sea_player = floater_player->area;

    // Stampa l'intestazione
    fprintf(stdout, "\n");
    for (i=0; i<m+1; i++) printf(" ");
    fprintf(stdout, ANSI_COLOR_BLUE "PLAYER" ANSI_COLOR_RESET);
    for (i=0; i<m; i++) fprintf(stdout, " ");
    fprintf(stdout, "%6c", ' ');
    for (i=0; i<m-1; i++) fprintf(stdout, " ");
    fprintf(stdout, ANSI_COLOR_BLUE "CPU" ANSI_COLOR_RESET);
    for (i=0; i<m; i++) fprintf(stdout, " ");
    fprintf(stdout, "\n");

    // stampa riga delle colonne
    fprintf(stdout, "%3c\u256D\u2574%c", ' ', 65);
    for (i=1; i<m; i++) fprintf(stdout, "-%c", 65+i);
    fprintf(stdout, "\u2576\u256E");
    fprintf(stdout, "%6c\u256D\u2574%c", ' ', 65);
    for (i=1; i<m; i++) fprintf(stdout, "-%c", 65+i);
    fprintf(stdout, "\u2576\u256E%3c", ' ');
    fprintf(stdout, "\n");

    // stampa righe con il proprio numero all'inizio
    for (i=0; i<n; i++) {
      fprintf(stdout, "%2c%02d ", ' ', i+1);
      for (j=0; j<m; j++) {
        cell = sea_player->a[i][j];
        switch (cell) {
          case EMPTY:
            fprintf(stdout, ANSI_COLOR_GREY "\u00B7 " ANSI_COLOR_RESET); break;
          case EMPTY_COLPITA:
            fprintf(stdout, ANSI_COLOR_GREY "X " ANSI_COLOR_RESET); break;
          case ONE: case ONE_COLPITA:
            fprintf(stdout, ANSI_COLOR_CYAN "%c " ANSI_COLOR_RESET, cell); break;
          case TWO: case TWO_COLPITA:
            fprintf(stdout, ANSI_COLOR_RED "%c " ANSI_COLOR_RESET, cell); break;
          case THREE: case THREE_COLPITA:
            fprintf(stdout, ANSI_COLOR_YELLOW "%c " ANSI_COLOR_RESET, cell); break;
        }
      }
      fprintf(stdout, "\u2502%5c%02d ", ' ', i+1);
      for (j=0; j<m; j++) {
        cell = sea_cpu->a[i][j];
        switch (cell) {
          case EMPTY: case ONE: case TWO: case THREE:
            fprintf(stdout, ANSI_COLOR_GREY "\u00B7 " ANSI_COLOR_RESET); break;
          case EMPTY_COLPITA:
            fprintf(stdout, ANSI_COLOR_GREY "X " ANSI_COLOR_RESET); break;
          case ONE_COLPITA:
            fprintf(stdout, ANSI_COLOR_CYAN "%c " ANSI_COLOR_RESET, cell); break;
          case TWO_COLPITA:
            fprintf(stdout, ANSI_COLOR_RED "%c " ANSI_COLOR_RESET, cell); break;
          case THREE_COLPITA:
            fprintf(stdout, ANSI_COLOR_YELLOW "%c " ANSI_COLOR_RESET, cell); break;
        }
      }
      fprintf(stdout, "\u2502%2c", ' ');
      fprintf(stdout, "\n");
    }

    // stampa la riga di chiusura
    fprintf(stdout, "%3c\u2570", ' ');
    for (i=0; i<m; i++) {
      fprintf(stdout, "\u2500\u2500");
    }
    fprintf(stdout, "\u2500\u256F");
    fprintf(stdout, "%6c\u2570", ' ');
    for (i=0; i<m; i++) {
      fprintf(stdout, "\u2500\u2500");
    }
    fprintf(stdout, "\u2500\u256F%3c", ' ');
    fprintf(stdout, "\n");

    // Stampa il numero di navi rimanenti
    fprintf(stdout, ANSI_COLOR_BLUE "%3cNavi:" ANSI_COLOR_RESET " %03d", ' ', sea_player->n_navi);
    for (i=0; i<m-4; i++) fprintf(stdout, " ");
    fprintf(stdout, "%6c", ' ');
    for (i=0; i<m-5; i++) fprintf(stdout, " ");
    fprintf(stdout, ANSI_COLOR_BLUE "%3cNavi:" ANSI_COLOR_RESET " %03d", ' ', sea_cpu->n_navi);
    fprintf(stdout, "\n\n");

    // Stampa le opzioni di gioco
    fprintf(stdout, ANSI_COLOR_BLUE "OPZIONI: " ANSI_COLOR_RESET);
    fprintf(stdout, "indica una casella nel formato \'LetteraNumero\' \n\toppure digita \'esci\' per terminare.\n\n");
}

/**
    stampa una intestazione con il nome del gioco per il menù del main
*/
void stampa_intestazione (void) {
  int i;
  fprintf(stdout, ANSI_COLOR_BLUE "\n");
  for (i=0; i<60; i++) {fprintf(stdout, "\u2248");} fprintf(stdout, "\n");
  for (i=0; i<20; i++) {fprintf(stdout, "\u2248");}
  fprintf(stdout, ANSI_COLOR_RESET "  BATTAGLIA NAVALE  " ANSI_COLOR_BLUE);
  for (i=0; i<20; i++) {fprintf(stdout, "\u2248");} fprintf(stdout, "\n");
  for (i=0; i<60; i++) {fprintf(stdout, "\u2248");} fprintf(stdout, "\n");
  fprintf(stdout, "\n" ANSI_COLOR_RESET);
}

/**
    funzione main
*/

int bnavale_main (int argc, char ** argv) {
  int guardia, uscita, opt, manual_flag = 0;
  char buffer[5];
  unsigned n = 5, n_navi1 = 0, n_navi2 = 0, n_navi3 = 0;

  // Controllo se il programma é stato avviato con argomenti e nel caso attivo il manual_flag
  if (argc>1) {
    while ((opt = getopt(argc, argv, "m:")) != -1) {
      switch (opt) {
        case 'm':
            manual_flag = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-m N n_one n_two n_three]\n", argv[0]);
            exit(EXIT_FAILURE);
      }
    }
    // Esco se non ci sono abbastanza argomenti
    if (manual_flag == 1 && argc-optind < 3) {
      fprintf(stderr, "Usage: %s [-m N n_one n_two n_three]\n", argv[0]);
      fprintf(stderr, "Too few arguments for \'-m\' option\n");
      exit(EXIT_FAILURE);
    }
    // Esco se non ci sono troppi argomenti
    if (manual_flag == 1 && argc-optind > 3) {
      fprintf(stderr, "Usage: %s [-m N n_one n_two n_three]\n", argv[0]);
      fprintf(stderr, "Too many arguments for \'-m\' option\n");
      exit(EXIT_FAILURE);
    }
    // Associo i valori degli argomenti alle variabili necessarie al gioco
    n = atoi(argv[optind-1]); if((int) n<5 || (int) n>26) n=10;
    n_navi1 = atoi(argv[optind]); if((int) n_navi1<0 || (int) n_navi1>1000) n_navi1=0;
    n_navi2 = atoi(argv[optind+1]); if((int) n_navi2<0 || (int) n_navi2>1000) n_navi2=0;
    n_navi3 = atoi(argv[optind+2]); if((int) n_navi3<0 || (int) n_navi3>1000) n_navi3=0;
  }

  do {
    guardia = 0; uscita = 0;
    // Genero il seme per le funzioni di estrazione casuale
    srand(time(NULL));

    // Salto l'inserimento dei dati se è stato attivato il flag m
    if (!manual_flag) {
      // Pulisco il terminale
      system("clear");

      // Stampo l'intestazione
      stampa_intestazione();

      // Raccolgo i dati di inizializzazione del gioco
      fprintf(stdout, "Benvenuto a Battaglia Navale\n\n");
      do {
        fprintf(stdout, "Per favore inserisci la dimensione della griglia (5-26): ");
        fscanf(stdin, "%u", &n);
      } while (n<5 || n>26);
      do {fprintf(stdout, "Quante navi da 1 (positivo): "); fscanf(stdin, "%u", &n_navi1);} while ((int) n_navi1<0);
      do {fprintf(stdout, "Quante navi da 2 (positivo): "); fscanf(stdin, "%u", &n_navi2);} while ((int) n_navi2<0);
      do {fprintf(stdout, "Quante navi da 3 (positivo): "); fscanf(stdin, "%u", &n_navi3);} while ((int) n_navi3<0);
      while(getchar()!='\n');
    }
    manual_flag = 0;

    // Inizio la partita
    gioca(n, n, n_navi1, n_navi2, n_navi3);

    // Richiesta per una nuova partita
    system("clear");
    stampa_intestazione();
    do {
      fprintf(stdout, "Vuoi fare una nuova partita? (Si/No)\n");
      fgets(buffer, 5, stdin);
      if (strncmp(buffer, "No", 2)==0 || strncmp(buffer, "NO", 2)==0 || strncmp(buffer, "no", 2)==0 ||
          strncmp(buffer, "N", 1)==0 || strncmp(buffer, "n", 1)==0) {
        guardia = 1;
        uscita = 1;
      }
      if (strncmp(buffer, "Si", 2)==0 || strncmp(buffer, "SI", 2)==0 || strncmp(buffer, "si", 2)==0 ||
          strncmp(buffer, "S", 1)==0 || strncmp(buffer, "s", 1)==0) {
        guardia = 1;
      }
    } while (!guardia);
  } while (!uscita);

  // Saluto ed esco
  system("clear");
  stampa_intestazione();
  printf("Arrivederci!  (\u0298\u203f\u0298)\u256f\n\n");

  exit(EXIT_SUCCESS);
}

