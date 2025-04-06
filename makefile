#all: lineage

#lineage: lineage.c
#	gcc -g lineage.c -o lineage

#clean:
#	rm -f lineage

all: lineagep

lineagep: lineagep.c
	gcc -g lineagep.c -o lineagep

clean:
	rm -f lineagep