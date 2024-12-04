/*
  \file
  \authors informatica (CDS Fisica) 2018/19
  \brief A2: quarto file di test
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "bnavale.h"

#define N 10

int main (void) {

  /* Test 3.1 */
  fprintf(stdout,"Test 3.1: giochiamo (premi un tasto qualsiasi per proseguire) \n");
  getchar();
  
  /* inizializzo il generatore dei numeri casuali */
  srand(time(NULL));

  /* avvio il gioco interattivo */
  gioca(N,N,2,3,4);

  return 0;

}

