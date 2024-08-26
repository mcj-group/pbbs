INCREMENTAL = maximalIndependentSet/incrementalMIS

MQ =  maximalIndependentSet/mqMIS 

ALL = $(INCREMENTAL) $(MQ)

all : $(ALL)

$(ALL) : FORCE
	make -C $@

FORCE :

clean : FORCE
	for bench in $(ALL); do \
	  make cleanall -s -C benchmarks/$$bench ; \
	done
