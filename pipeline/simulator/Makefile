TARGET=make

$(TARGET):  main.o
	gcc main.o -lm  -o pipeline


main.o: 
	gcc -c  main.c -lm 

clean:
	rm -f *.o pipeline 


