all:	
	gcc -O2 -D__BSTEA_MAIN_ -o bstea_test bstea_test.c bstea.c

clean:
	rm -f bstea_test 
