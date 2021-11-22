
syn_ack: 
	gcc syn_ack.c  -Wall -W -pedantic -o syn_ack

recv6_ll:
	gcc recv6_ll.c -Wall -W -pedantic -o recv6_ll

clean:
	rm -rf syn_ack recv6_ll
