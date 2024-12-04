/**
  \file
  Funzione di stampa secondo assegnamento (non modificare)
*/
#include <stdio.h>
#include  "bnavale.h"


void print_area (area_t * sea){
  int i, j;

  if ( sea == NULL ) return;
  
  for (i=0; i<sea->N; i++) {
   for (j=0; j<sea->M; j++) 	
      printf("%c ", sea->a[i][j]);
   printf("\n");
   }
  
  putchar('\n');
  printf("#navi: %d", sea->n_navi);
  putchar('\n');
  putchar('\n');

  return; 
}


int conta_navi (area_t* sea, char k) {
  int i, j;
  int c = 0;
  if ( k < EMPTY || k > THREE ) return -1;
  
  for (i=0; i<sea->N; i++)
    for (j=0; j<sea->M; j++){
      if (sea->a[i][j] == k) c++;
      else if (sea->a[i][j] == k+131) c++;
      else if (k>1 && sea->a[i][j] == k+133) c++;
    }

  if ( k == EMPTY || k == ONE )  return c;
  return c/(k-'0');
}

void stampa_esito (int e) {
  switch (e) {
  case ERRORE: 	printf("Errore!\n"); break;
  case ACQUA: 	        printf("Acqua\n"); break;
  case AFFONDATA_ONE: 	printf("Affondata nave ONE\n"); break;
  case AFFONDATA_TWO: 	printf("Affondata nave TWO\n"); break;
  case AFFONDATA_THREE: 	printf("Affondata nave THREE\n"); break;
  case COLPITA_TWO: 	printf("Colpita nave TWO\n"); break;
  case COLPITA_THREE: 	printf("Colpita nave THREE\n"); break;
  default: printf("Errore stampa esito\n"); break;
  }
}
