all: lineagedp

lineagedp: lineagedp.c
	gcc -g lineagedp.c -o lineagedp

clean:
	rm -f lineagedp
