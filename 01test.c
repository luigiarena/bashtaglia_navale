/*
  \file
  \authors informatica (CDS Fisica) 2018/19
  \brief A1: primo file di test 
-- allocazione/deallocazione/inizializzazione area di gioco
 */
#include <stdio.h>
#include <stdlib.h>
#include "bnavale.h"
#define N 10
#define M 20


int main (void) {
  area_t * sea;
  
  /* Test 1.1 */
  fprintf(stdout,"Test 1.1: test allocazione \n");
  sea = crea_area (N, M);
  print_area(sea);


  /* Test 1.2 */
  fprintf(stdout,"Test 1.2: test deallocazione \n");
  distruggi_area(sea);


  /* Test 1.3 */
  fprintf(stdout,"Test 1.3: test init \n");
  sea = crea_area(N,N);
  sea->a[0][0] = ONE;
  init(sea);
  print_area(sea);

  distruggi_area(sea);
  
  return 0;

}

