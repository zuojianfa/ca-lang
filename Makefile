CLANG_CONFIG=llvm-config
CLANGPP=clang++
CLANG=clang
CFLAGS=-g

CA_FLAGS=-g

all: ca

################################################################################
# main area
################################################################################

# bison -d ca.y will generate ca.tab.h and ca.tab.c
# yacc -d ca.y will generate y.tab.h and y.tab.c

lex.yy.c: ca.l
	flex ca.l

ca.l: ca.tab.h

ca.tab.h ca.tab.c: ca.y
	bison -Wcounterexamples -d  $^

ca.tab.o: ca.tab.c
	gcc ${CFLAGS} -c -o $@ $^

.c.o:
	gcc ${CFLAGS} -c -o $@ $^

################################################################################
# llvm area
################################################################################

LLVM_IR = lex.yy.o ca.tab.o symtable_cpp.o utils.o llvm/IR_generator.o \
	llvm/ir1.o llvm/dwarf_debug.o ca.o

ca: llvm/ir1.h ${LLVM_IR}
	$(CLANGPP) ${CFLAGS} `${CLANG_CONFIG} --ldflags --system-libs --libs core orcjit native` ${LLVM_IR} -o $@

llvm/IR_generator.o: llvm/ir1.h

.cpp.o:
	$(CLANGPP) ${CFLAGS} -I. `${CLANG_CONFIG} --cxxflags` -c $< -o $@

################################################################################
# test area
################################################################################

llvm-test-to-o:
	./ca -ll ${CA_FLAGS} test/goto.ca test/goto.ll && llc -filetype=obj -o test/goto.o test/goto.ll
	./ca -ll ${CA_FLAGS} test/goto2.ca test/goto2.ll && llc -filetype=obj -o test/goto2.o test/goto2.ll
	./ca -ll ${CA_FLAGS} test/goto_simplest.ca test/goto_simplest.ll && llc -filetype=obj -o test/goto_simplest.o test/goto_simplest.ll
	./ca -ll ${CA_FLAGS} test/if.ca test/if.ll && llc -filetype=obj -o test/if.o test/if.ll
	./ca -ll ${CA_FLAGS} test/fib.ca test/fib.ll && llc -filetype=obj -o test/fib.o test/fib.ll
	./ca -ll ${CA_FLAGS} test/fib2.ca test/fib2.ll && llc -filetype=obj -o test/fib2.o test/fib2.ll
	./ca -ll ${CA_FLAGS} test/fib_mvar.ca test/fib_mvar.ll && llc -filetype=obj -o test/fib_mvar.o test/fib_mvar.ll
	./ca -ll ${CA_FLAGS} test/print.ca test/print.ll && llc -filetype=obj -o test/print.o test/print.ll
	./ca -ll ${CA_FLAGS} test/while.ca test/while.ll && llc -filetype=obj -o test/while.o test/while.ll
	./ca -ll ${CA_FLAGS} test/polynomial.ca test/polynomial.ll && llc -filetype=obj -o test/polynomial.o test/polynomial.ll


llvm-test-clang: llvm-test-to-o
	clang ${CA_FLAGS} -o test/goto test/goto.o
	clang ${CA_FLAGS} -o test/goto2 test/goto2.o
	clang ${CA_FLAGS} -o test/goto_simplest test/goto_simplest.o
	clang ${CA_FLAGS} -o test/if test/if.o
	clang ${CA_FLAGS} -o test/fib test/fib.o
	clang ${CA_FLAGS} -o test/fib2 test/fib2.o
	clang ${CA_FLAGS} -o test/fib_mvar test/fib_mvar.o
	clang ${CA_FLAGS} -o test/print test/print.o
	clang ${CA_FLAGS} -o test/while test/while.o
	clang ${CA_FLAGS} -o test/polynomial test/polynomial.o

llvm-test-to-s:
	./ca -ll ${CA_FLAGS} test/goto.ca test/goto.ll && llc -filetype=asm -o test/goto.s test/goto.ll
	./ca -ll ${CA_FLAGS} test/goto2.ca test/goto2.ll && llc -filetype=asm -o test/goto2.s test/goto2.ll
	./ca -ll ${CA_FLAGS} test/goto_simplest.ca test/goto_simplest.ll && llc -filetype=asm -o test/goto_simplest.s test/goto_simplest.ll
	./ca -ll ${CA_FLAGS} test/if.ca test/if.ll && llc -filetype=asm -o test/if.s test/if.ll
	./ca -ll ${CA_FLAGS} test/fib.ca test/fib.ll && llc -filetype=asm -o test/fib.s test/fib.ll
	./ca -ll ${CA_FLAGS} test/fib2.ca test/fib2.ll && llc -filetype=asm -o test/fib2.s test/fib2.ll
	./ca -ll ${CA_FLAGS} test/fib_mvar.ca test/fib_mvar.ll && llc -filetype=asm -o test/fib_mvar.s test/fib_mvar.ll
	./ca -ll ${CA_FLAGS} test/print.ca test/print.ll && llc -filetype=asm -o test/print.s test/print.ll
	./ca -ll ${CA_FLAGS} test/while.ca test/while.ll && llc -filetype=asm -o test/while.s test/while.ll
	./ca -ll ${CA_FLAGS} test/polynomial.ca test/polynomial.ll && llc -filetype=asm -o test/polynomial.s test/polynomial.ll

# FIXME: when using as ld to make executable from .s, after running the executable it will crash when in last retq
llvm-test-as-ld2: llvm-test-to-s
	as test/goto.s     -o test/goto.o     && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/goto     test/goto.o     -lc
	as test/goto2.s    -o test/goto2.o    && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/goto2    test/goto2.o    -lc
	as test/goto_simplest.s    -o test/goto_simplest.o    && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/goto_simplest    test/goto_simplest.o    -lc
	as test/if.s       -o test/if.o       && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/if       test/if.o       -lc
	as test/fib.s      -o test/fib.o      && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/fib      test/fib.o      -lc
	as test/fib2.s     -o test/fib2.o     && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/fib2     test/fib2.o     -lc
	as test/fib_mvar.s -o test/fib_mvar.o && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/fib_mvar test/fib_mvar.o -lc
	as test/print.s    -o test/print.o    && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/print    test/print.o    -lc
	as test/while.s    -o test/while.o    && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/while    test/while.o    -lc
	as test/polynomial.s    -o test/polynomial.o    && ld --entry=main -dynamic-linker /lib64/ld-linux-x86-64.so.2 -o test/polynomial    test/polynomial.o    -lc

# fixed
llvm-test-as-ld: llvm-test-to-s
	as test/goto.s     -o test/goto.o     && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/goto     test/goto.o     -lc
	as test/goto2.s    -o test/goto2.o    && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/goto2    test/goto2.o    -lc
	as test/goto_simplest.s    -o test/goto_simplest.o    && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/goto_simplest    test/goto_simplest.o    -lc
	as test/if.s       -o test/if.o       && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/if       test/if.o       -lc
	as test/fib.s      -o test/fib.o      && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/fib      test/fib.o      -lc
	as test/fib2.s     -o test/fib2.o     && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/fib2     test/fib2.o     -lc
	as test/fib_mvar.s -o test/fib_mvar.o && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/fib_mvar test/fib_mvar.o -lc
	as test/print.s    -o test/print.o    && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/print    test/print.o    -lc
	as test/while.s    -o test/while.o    && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/while    test/while.o    -lc
	as test/polynomial.s    -o test/polynomial.o    && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/polynomial    test/polynomial.o    -lc

llvm-test-o:
	./ca -c test/goto.ca test/goto.o              && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/goto         test/goto.o       -lc
	./ca -c test/goto2.ca test/goto2.o	           && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/goto2        test/goto2.o      -lc
	./ca -c test/goto_simplest.ca test/goto_simplest.o	           && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/goto_simplest        test/goto_simplest.o      -lc
	./ca -c test/if.ca test/if.o                  && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/if           test/if.o         -lc
	./ca -c test/fib.ca test/fib.o		   && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/fib          test/fib.o        -lc
	./ca -c test/fib2.ca test/fib2.o		   && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/fib2         test/fib2.o       -lc
	./ca -c test/fib_mvar.ca test/fib_mvar.o	   && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/fib_mvar     test/fib_mvar.o   -lc
	./ca -c test/print.ca test/print.o		   && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/print        test/print.o      -lc
	./ca -c test/while.ca test/while.o		   && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/while        test/while.o      -lc
	./ca -c test/polynomial.ca test/polynomial.o  && ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 cruntime/*.o -o test/polynomial   test/polynomial.o -lc

clean:
	rm -f *.o *.e test/*.o ca.tab.h ca.tab.c lex.yy.c a.out ca llvm/*.o test/fib test/fib2 test/fib_mvar test/goto test/goto2 test/if test/print test/while


