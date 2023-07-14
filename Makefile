RM =rm -f

OBJS = $(.o ,.exe)

FPSTerminal: fps.o
	g++ -g -o FPSTerminal fps.o

fps.o: TerminalFPS.cpp
	g++ -g -c 

clean: 
	$(RM) $(BOJS)