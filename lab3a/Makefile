# NAMES: Dawei Huang and Nathan Smith 
# EMAILS: <Dawei's email> and nathan.smith@ucla.edu
# IDS: <Dawei's ID> and 704787554


default: build

build:
	gcc -Wall -Wextra -o lab3a lab3a.c

clean:
	rm -f lab3a lab3a-704787554.tar.gz

dist:
	tar -zcvf lab3a-704787554.tar.gz Makefile README lab3a.c

mount:
	sudo mount -o ro,loop ./EXT2_test.img fs
