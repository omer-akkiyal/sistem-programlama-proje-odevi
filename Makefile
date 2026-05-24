all: tarsau

tarsau: main.c tarsau.c
	gcc -o tarsau main.c tarsau.c -I.

clean:
	rm -f tarsau