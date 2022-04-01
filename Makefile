all: myShell
myShell: myShell.c
	gcc -o myShell myShell.c -Wall -Werror