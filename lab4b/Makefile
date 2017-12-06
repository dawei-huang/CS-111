#NAME: Dawei Huang
#EMAiL: daweihuang@g.ucla.edu
#ID: 304792166

default:
	gcc -Wall -Wextra -lmraa -lm -g lab4b.c -o lab4b

check:	
	./lab4b -p4 << -EOF
	START
	STOP
	OFF
	EOF

clean:
	rm -f lab4b *.tar.gz *.txt

dist:
	tar -zczf lab4b-304792166.tar.gz Makefile README lab4b.c
