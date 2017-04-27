all :
	gcc -o reduction -Wall Projet.c -lpthread

run :
	./reduction

clean :
	rm ./reduction
