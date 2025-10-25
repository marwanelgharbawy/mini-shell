## you have command.cc command.h tokenizer.cc tokenizer.h you need compile them and link
## them to one object file called  : "myshell" 
## so we should run make , that will create our object myshell ,
## ./myshell should run your application 

# Targests that are not files
.PHONY: all run clean

all: myshell

run: myshell
	@./myshell

test: myshell
	@./run_all_and_sum.sh

myshell: command.o tokenizer.o
	@g++ -g -Wall -std=c++11 -o myshell command.o tokenizer.o

command.o: command.cc
	@g++ -g -Wall -std=c++11 -c command.cc

tokenizer.o: tokenizer.cc
	@g++ -g -Wall -std=c++11 -c tokenizer.cc

clean:
	@rm -f myshell command.o tokenizer.o