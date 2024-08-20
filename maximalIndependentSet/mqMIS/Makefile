CC = g++
CCFLAGS = -mcx16 -O3 -std=c++17 -DNDEBUG
CLFLAGS =
CFLAGS = -DPARLAY_SEQUENTIAL -pthread $(CCFLAGS)
LFLAGS = -lpthread $(CLFLAGS)
GITROOT := $(shell git rev-parse --show-toplevel)
CCFLAGS += -I$(GITROOT) -I$(GITROOT)/parlaylib/include

ifeq (, $(shell which ld.lld))
else
    CLFLAGS += -fuse-ld=lld
endif

BENCH = MIS
OBJS = MIS.o

CPS= MultiQueue.h MultiBucketQueue.h BucketStructs.h

TIME = $(BENCH)Time
CHECK = ../bench/$(BENCH)Check
INCLUDE =

# Make benchmark
$(BENCH) : $(TIME).o $(OBJS) $(CHECK)
	$(CC) -o $@ $(TIME).o $(OBJS) $(LFLAGS)

# Timing Code
$(TIME).o : ../bench/$(TIME).C $(CPS)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c ../bench/$(TIME).C
	
# The check code
$(CHECK) : $(CHECK).C
	make $(BENCH)Check -C ../bench

# object files
%.o : %.C $(REQUIRE) 
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

$(CPS):
	ln -s ../../../CPS/include/*.h .

clean :
	rm -f $(BENCH) *.o

cleanall : clean
	rm -f testInputs*;
	make clean -C ../bench
