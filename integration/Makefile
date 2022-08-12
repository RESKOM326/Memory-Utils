# Compiler
CC				=	gcc

# Flags
CFLAGS			=	-Wall -Wextra -Wformat -g

# ==================================================

DIR_SRC 		= 	../src
DIR_TST 		= 	./testing
DIR_BLD 		= 	../bld

# ==================================================

DEPENDENCY 		=	$(DIR_BLD)/mu_utils.o $(DIR_BLD)/mu_diag.o $(DIR_BLD)/mu_memchunk.o
INCLUDEDIR		=	-I$(DIR_SRC)/inc

default:	scanner tests simulator cleanobj

scanner:
			$(CC) $(CFLAGS) $(INCLUDEDIR) -c -o $(DIR_BLD)/mu_utils.o $(DIR_SRC)/mu_utils.c
			$(CC) $(CFLAGS) $(INCLUDEDIR) -c -o $(DIR_BLD)/mu_diag.o $(DIR_SRC)/mu_diag.c
			$(CC) $(CFLAGS) $(INCLUDEDIR) -c -o $(DIR_BLD)/mu_memchunk.o $(DIR_SRC)/mu_memchunk.c

tests:
			$(CC) $(CFLAGS) $(INCLUDEDIR) -o $(DIR_BLD)/test1 $(DIR_TST)/test1.c $(DEPENDENCY)

simulator:
			$(CC) -o $(DIR_BLD)/sim $(DIR_TST)/sim.c 

cleanobj:
			rm -rf $(DIR_BLD)/*.o

cleanall:
			rm -rf $(DIR_BLD)/*