/*
  \file
  \authors informatica (CDS Fisica) 2018/19
  \brief A2: secondo file di test
-- test put_one put_two put three
 */
#include <stdio.h>
#include <stdlib.h>
#include "bnavale.h"

#define L 30

#define N1 6
#define N2 5
#define N3 3

#define SEED 42

int main (void) {
  int i,j;
  int num;
  area_t * sea;
  
  /* Test 2.1 */
  fprintf(stdout,"Test 2.1: creaiamo l'area e inseriamo varie navi \n");

  /* inizializzo il generatore dei numeri casuali */
  srand(SEED);

  /* creo e inizializzo l'area a vuoto */
  sea = crea_area(L,L);
  init(sea);
  print_area(sea);



  /* inserisco navi lunghe 1 */
  num = N1;
  for ( i = 0 ;i < num; i++) {
     if ( put_one(sea) == -1 ) return EXIT_FAILURE;
     
  }
  print_area(sea);
  /* inserite tutte ?*/
  if (conta_navi(sea,ONE) != num) return EXIT_FAILURE;


  /* Test 2.2 */
  fprintf(stdout,"Test 2.2: test navi lunghe 2\n");
  
  /* inserisco navi lunghe 2 */
  num = N2 ;
  fprintf(stderr,"Test 2.2: num %d\n",num);

  for ( i = 0 ;i < num; i++) {
    if (put_two(sea) != 0 ) return EXIT_FAILURE;
  }
  print_area(sea);
  
  /* inserite tutte ?*/
  if (conta_navi(sea,TWO) != num) return EXIT_FAILURE;

  /* Test 2.3 */
  fprintf(stdout,"Test 2.3: test navi lunghe 3\n");
  
  print_area(sea);

  /* inserisco navi lunghe 3 */
  num = N3 ;
  fprintf(stderr,"Test 1.4: num %d\n",num);

  for ( i = 0 ;i < num; i++) {
    if (put_three(sea) != 0 ) return EXIT_FAILURE;   
  }
  print_area(sea);
/* inserite tutte ?*/
  if (conta_navi(sea,THREE) != num) return EXIT_FAILURE;


  /* inserimento impossibile */
  fprintf(stderr,"Test 1.5: inserimento impossibile\n");
  init(sea);
  for (i=0;i<L;i++)
    for (j=0;j<L;j++)
      if (i%2==0 && j%2==0) {
	     sea->a[i][j] = ONE;
	     (sea->n_navi)++;
      }
  
  print_area(sea);

  /* no navi adiacenti ! */
  if (put_one(sea) != -1 ) return EXIT_FAILURE;   

  if (put_two(sea) != -1 ) return EXIT_FAILURE;   

  if (put_three(sea) != -1 ) return EXIT_FAILURE;   


return 0;

}

