################################
#
#     Makefile SECONDO ASSEGNAMENTO AA 2018/19
#    
################################


# file da consegnare
# modificare se si aggiungono file
FILE_DA_CONSEGNARE=bnavale.c

# preferenze compilatore e flag di compilazione
CC=gcc
# compilazione 
CFLAGS= -Wall -pedantic -g

obj = bnavale.o bnavale_docenti.o



# phony targets
.PHONY: cleanall test1 test2 test3 consegna 


bat1 : 01test.o $(obj)
	gcc $(obj) 01test.o -o bat1 -lm

bat2 : 02test.o $(obj)
	gcc $(obj) 02test.o -o bat2 -lm

bat3 : 03test.o $(obj)
	gcc $(obj) 03test.o -o bat3 -lm

bat4 : 04test.o $(obj)
	gcc $(obj) 04test.o -o bat4 -lm




bnavale.o: bnavale.c bnavale.h  
bmain.o: bmain.c bnavale.h  
01test.o: 01test.c bnavale.h   
02test.o: 02test.c bnavale.h   
03test.o: 03test.c bnavale.h   
04test.o: 04test.c bnavale.h   

battaglia : $(obj) bmain.o
	gcc $(obj) bmain.o -o battaglia -lm


cleanall: 
	-rm -f *.o *~ ./core bat? battaglia


test1:
	rm -f ./out1.check
	cp DATA/out1.check .
	make cleanall
	make bat1
	./bat1 | tee out1.txt
	diff -s out1.txt out1.check # (controlla che l'output sia uguale a DATA/out1.check)
	@echo "********** Test1 superato!"

test2: 
	make cleanall
	make bat2
	./bat2 
	@echo "********** Test2 superato!"

test3: 
	rm -f ./out3.check
	cp DATA/out3.check .
	cp DATA/mosse.txt .
	make cleanall
	make bat3 
	./bat3 < mosse.txt > out3.txt
	diff -s out3.txt out3.check # (controlla che l'output sia uguale a DATA/out3.check)
	@echo "********** Test3 superato!"


all:
	make cleanall
	make test1
	make test2
	make test3
	@echo "********** tutti i file sono stati creati e i test correttamente superati"
