my_shell: my_shell.o
	g++ my_shell.o -o my_shell

my_shell.o: my_shell.cc
	g++ -c my_shell.cc

clean:
	rm my_shell.o my_shell
