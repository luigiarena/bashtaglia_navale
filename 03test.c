/*
  \file
  \authors informatica (CDS Fisica) 2018/19
  \brief A2: terzo file di test
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "bnavale.h"

/* lato area di gara */
#define L 10

/* tipo coordinate */
typedef struct {
  int x;
  int y;
} coord_t;

/* coodinate navi da 1 */
#define N1 4
coord_t navi1 [N1] = {{0,0}, {7,9}, {3,3}, {4,7}};

/* coodinate navi da 2 */
#define N2 6
coord_t navi2 [N2] = {{3,0}, {3,9}, {0,2}, {1,2},{7,3},{8,3}};

/* coodinate navi da 3 */
#define N3 6
coord_t navi3 [N3] = {{0,7}, {1,7}, {9,7}, {5,1}, {5,2}, {5,3}};

int main (void) {
  int i;
  int x,y,e;
  
  area_t * sea;
  

  /* Test 3.1 */
  fprintf(stdout,"Test 3.1: test esecuzione singola mossa \n");

  /* costruiamo l'area */
  sea = crea_area(L,L);
  init(sea);
  
  
  /* posizioniamo le navi da uno */
  for (i=0;i<N1; i++) {
    sea->a[navi1[i].x][navi1[i].y] = ONE;
  }
  sea->n_navi = N1;
  
  /*... da due */
  for (i=0;i<N2; i++) {
    sea->a[navi2[i].x][navi2[i].y] = TWO;
  }
  sea->n_navi += N2/2;
  
  /* ... da tre! */
  for (i=0;i<N3; i++) {
    sea->a[navi3[i].x][navi3[i].y] = THREE;
  }
  sea->n_navi += N3/3;
  
  print_area(sea);

  /* eseguiamo le mosse richieste da stdin ...*/
  while ( !feof(stdin) ) {
    printf("Inserisci una mossa ( x y INVIO)\n");
    scanf("%d%d",&x,&y);
    e = esegui_mossa(sea,x,y);
    printf("Colpo in ( %d, %d ):",x,y);
    stampa_esito (e);
    print_area(sea);

  }

  return 0;

}

