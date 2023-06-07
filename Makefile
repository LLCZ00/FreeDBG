.PHONY: clean


SOURCES = ./src/main.cpp ./src/logging.cpp ./src/arghandler.cpp ./src/debugger.cpp ./src/interface.cpp


freedbg:
	clang++ $(SOURCES) -o $@ 

clean:
	rm -f freedbg