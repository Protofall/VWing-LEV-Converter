
EXE = VWing-LEV-To-Png
EXT =
CFLAGS = -lpng -g
#Add a -g to the flags to get line number info for valgrind

CFILES = $(wildcard *.c)
OBJS = $(CFILES:%.c=%.o)

.PHONY: all clean run

all: clean $(EXE)

$(EXE): $(OBJS)
	gcc -o $(EXE) $(OBJS) $(CFLAGS) 

%.o: %.c
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o $(EXE)$(EXT)
	rm -f *.png

run:
	./$(EXE) vwing/LEVEL1.LEV level1.png
	#./$(EXE) vwing/LEVEL2.LEV level2.png
	#./$(EXE) vwing/LEVEL3.LEV level3.png
	#./$(EXE) vwing/LEVEL4.LEV level4.png
	#./$(EXE) vwing/LEVEL5.LEV level5.png
	./$(EXE) vwing/LEVEL6.LEV level6.png
	#./$(EXE) vwing/LEVEL7.LEV level7.png
	#./$(EXE) vwing/LEVEL8.LEV level8.png
	#./$(EXE) vwing/LEVEL9.LEV level9.png

