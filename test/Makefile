## ********************************************************************
# 	Makefile for Mini-Othello
#
#	Type "make" for using GCC or "make bcc" for using Borland C++.
#	(Assuming the compiler(s) have been properly installed)
#
#	GCC 3.4.x for Linux or Cygwin, or 4.2 (MinGW) for Windows/DOS
#       is recommended. (MinGW GCC 3.4.x may also work, not tested
#       though.)
#
#	Yunpeng Li 
# *********************************************************************

CC = g++

# GCC-2.95:
#CPPFLAGS = -Wall -O2 -ffast-math -finline-functions -fomit-frame-pointer

# GCC-3.4 and 4.2:
CPPFLAGS = -Wall -O3 -ffast-math -fomit-frame-pointer -funroll-loops \
-foptimize-sibling-calls -frename-registers -march=pentium4 -mfpmath=sse

PROGRAM_NAME = othello
OBJ = othello.o game.o board.o minimax.o mobility.o liberty.o


$(PROGRAM_NAME): $(OBJ)
	$(CC) $(CPPFLAGS) -o $(PROGRAM_NAME) $(OBJ)


othello.o: othello.cpp othello.hpp
game.o: game.cpp game.hpp
board.o: board.cpp board.hpp
minimax.o: minimax.cpp minimax.hpp mob_table.hpp adj_table.hpp
mobility.o: mobility.cpp mobility.hpp
liberty.o: liberty.cpp liberty.hpp

endgamecx.o: endgamecx.c endgamecx.h

clean:
	rm -f *.o *.obj *~ core $(PROGRAM_NAME).tds

ca:
	rm -f *.o *.obj *~ $(PROGRAM_NAME) $(PROGRAM_NAME).exe $(PROGRAM_NAME).tds

cls: 
	del /q *.o *.obj *~ $(PROGRAM_NAME).tds

cleangcc:
	rm -f *.o *~ core

upx:
	upx -9 -q $(PROGRAM_NAME).exe

gccupx:
	make
	upx -9 -q $(PROGRAM_NAME).exe



## ------- For compile using Borland C++ Compiler ----------

### Options for Borland C++ Compiler 
BCC = bcc32
# -w-xxxx for suppress some not too useful warnings
BCC_FLAGS = -O2 -w-8008 -w-8066 -w-8057 -w-8004 -w-8027


bcc: othello.obj game.obj board.obj minimax.obj
	$(BCC) $(BCC_FLAGS) -e$(PROGRAM_NAME) othello.obj game.obj board.obj minimax.obj

othello.obj: othello.cpp othello.hpp
	$(BCC) $(BCC_FLAGS) -c -oothello.obj othello.cpp
game.obj: game.cpp game.hpp
	$(BCC) $(BCC_FLAGS) -c -ogame.obj game.cpp
board.obj: board.cpp board.hpp
	$(BCC) $(BCC_FLAGS) -c -oboard.obj board.cpp
minimax.obj: minimax.cpp minimax.hpp
	$(BCC) $(BCC_FLAGS) -c -ominimax.obj minimax.cpp

bccupx:
	make bcc
	upx -9 -q $(PROGRAM_NAME).exe

cleanbcc:
	rm -f *.obj *~ $(PROGRAM_NAME).tds
