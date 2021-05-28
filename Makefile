all: buidler testcases testcases_data

buidler:
	g++ -o debugger src/main.cc src/debugger_manager.cc src/breakpoint.cc \
		src/compile_unit.cc src/disassembler.cc src/parameter_extractor.cc \
		src/registers.cc src/memory.cc src/helpers.cc \
		-lcapstone -lelf++ -ldwarf++ -std=gnu++11 -ldl 

testcases:
	echo "done"
	mkdir -p examples/bin
	g++ -o examples/bin/test1 examples/test1.cc -no-pie -g
	g++ -o examples/bin/test2 examples/test2.cc -no-pie -g
	g++ -o examples/bin/test3 examples/test3.cc -no-pie -g
	g++ -o examples/bin/test4 examples/test4.cc -no-pie -g

testcases_data:
	objdump -dj .text /poc/examples/bin/test1 > /poc/examples/data/test1.asm
	objdump -s -j .text /poc/examples/bin/test1 > /poc/examples/data/test1.hex
	objdump -dj .text /poc/examples/bin/test2 > /poc/examples/data/test2.asm
	objdump -s -j .text /poc/examples/bin/test2 > /poc/examples/data/test2.hex
	objdump -dj .text /poc/examples/bin/test3 > /poc/examples/data/test3.asm
	objdump -s -j .text /poc/examples/bin/test3 > /poc/examples/data/test3.hex
	objdump -dj .text /poc/examples/bin/test4 > /poc/examples/data/test4.asm
	objdump -s -j .text /poc/examples/bin/test4 > /poc/examples/data/test4.hex

run_test1:
	/poc/debugger /poc/examples/bin/test1

run_test2:
	/poc/debugger /poc/examples/bin/test2

run_test3:
	/poc/debugger /poc/examples/bin/test3

run_test4:
	/poc/debugger /poc/examples/bin/test4

clean:
	rm -f debugger examples/bin/* examples/data/* obj/* 