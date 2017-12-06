#NAME: Dawei Huang
#EMAiL: daweihuang@g.ucla.edu
#ID: 304792166

default:
	gcc -Wall -Wextra -lmraa -lm -g lab4c_tcp.c -o lab4c_tcp
	#gcc -Wall -Wextra -lm -lmraa -lssl -lcrypto -g lab4c_tls.c -o lab4c_tls

clean:
	rm -f lab4c_tcp lab4c_tls *.tar.gz *.txt

dist:
	tar -zczf lab4c-304792166.tar.gz Makefile README lab4c.c
