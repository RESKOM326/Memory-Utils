# Compiler
CC				=	gcc

# Flags
CFLAGS			=	-Wall -Wextra -Wformat -g

# ==================================================

DIR_SRC 		= 	../src
DIR_TST 		= 	./testing
DIR_BLD 		= 	../bld

# ==================================================

DEPENDENCY 		=	$(DIR_BLD)/mu_utils.o $(DIR_BLD)/mu_diag.o $(DIR_BLD)/mu_memchunk.o $(DIR_BLD)/mu_io.o $(DIR_BLD)/mu_scanner.o
INCLUDEDIR		=	-I$(DIR_SRC)/inc

default:	scanner tests memscanlx cleanobj

scanner:
			$(CC) $(CFLAGS) $(INCLUDEDIR) -c -o $(DIR_BLD)/mu_utils.o $(DIR_SRC)/mu_utils.c
			$(CC) $(CFLAGS) $(INCLUDEDIR) -c -o $(DIR_BLD)/mu_diag.o $(DIR_SRC)/mu_diag.c
			$(CC) $(CFLAGS) $(INCLUDEDIR) -c -o $(DIR_BLD)/mu_memchunk.o $(DIR_SRC)/mu_memchunk.c
			$(CC) $(CFLAGS) $(INCLUDEDIR) -c -o $(DIR_BLD)/mu_io.o $(DIR_SRC)/mu_io.c
			$(CC) $(CFLAGS) $(INCLUDEDIR) -c -o $(DIR_BLD)/mu_scanner.o $(DIR_SRC)/mu_scanner.c

tests:
			$(CC) $(CFLAGS) $(INCLUDEDIR) -o $(DIR_BLD)/test1 $(DIR_TST)/test1.c $(DEPENDENCY)

simulator:
			$(CC) -o $(DIR_BLD)/simulator $(DIR_TST)/simulator.c 

memscanlx:	
			$(CC) $(CFLAGS) $(INCLUDEDIR) -o $(DIR_BLD)/mem_scan_linux $(DIR_SRC)/mem_scan_linux.c $(DEPENDENCY)

cleanobj:
			rm -rf $(DIR_BLD)/*.o

cleanall:
			rm -rf $(DIR_BLD)/*