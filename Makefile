se: se.c
	gcc -Wall -Wextra -O3 -o se se.c -lpthread

clean:
	rm -f se
