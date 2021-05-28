# print-function-args-debugger
A refactored Proof-of-concept originally developed in 2017 to print all function calls with their arguments data types and values using Ptrace during program execution.

## Setup POC
```sh
git clone git@github.com:finixbit/print-function-args-debugger.git
cd print-function-args-debugger

# To rebuild this image you must use `docker-compose build`
docker-compose run poc bash

cd /poc
make all

# binary must be compiled with debug info
/poc/debugger /path/to/binary
```

## Testcases
### Running test case 1
commands
```sh
cd /poc
make run_test1
```
outputs
```sh
/poc/debugger /poc/examples/bin/test1
0x000000000040115b	(Set breakpoint for main)
0x0000000000401149	(Set breakpoint for _Z9testcase4iPcS_)
0x000000000040113b	(Set breakpoint for _Z9testcase3iPc)
0x000000000040112c	(Set breakpoint for _Z9testcase2ic)
0x0000000000401122	(Set breakpoint for _Z9testcase1i)

............................................
[*] 0x40115b
[*] main(
	int argc<1>,
	char **argv<0x7ffce4cd5a88 = '[ 0x7ffce4cd5ebb= ]'>)
[*] examples/test1.cc:18
............................................
[*] 0x401122
[*] testcase1()
[*] examples/test1.cc:6
............................................
[*] 0x40112c
[*] testcase2(
	int data1<555>,
	char data2<65>)
[*] examples/test1.cc:9
............................................
[*] 0x40113b
[*] testcase3(
	int data1<555>,
	char *data2<0x9cae70>)
[*] examples/test1.cc:12
............................................
[*] 0x401149
[*] testcase4(
	int data1<555>,
	char *data2<0x9cae90>,
	char *data3<0x9caeb0>)
[*] examples/test1.cc:15
```

### Running test case 2
commands
```sh
cd /poc
make run_test2
```
outputs
```sh
/poc/debugger /poc/examples/bin/test2
0x0000000000401167	(Set breakpoint for main)
0x000000000040114f	(Set breakpoint for _Z9testcase47data4_t)
0x0000000000401138	(Set breakpoint for _Z9testcase37data3_t)
0x000000000040112d	(Set breakpoint for _Z9testcase27data2_t)
0x0000000000401122	(Set breakpoint for _Z9testcase17data1_t)

............................................
[*] 0x401167
[*] main(
	int argc<1>,
	char **argv<0x7ffd55820928 = '[ 0x7ffd55821ebb= ]'>)
[*] examples/test2.cc:39
............................................
[*] 0x401122
[*] testcase1()
[*] examples/test2.cc:27
............................................
[*] 0x40112d
datatype_idx  = 1
complex_idx   = 0
fn_index      = 32
cu_index      = 0
member= name 1
member= value 4
[*] testcase2(data2_t param<...>)
[*] examples/test2.cc:30
............................................
[*] 0x401138
datatype_idx  = 1
complex_idx   = 0
fn_index      = 31
cu_index      = 0
member= value 4
member= name 1
[*] testcase3(data3_t param<...>)
[*] examples/test2.cc:33
............................................
[*] 0x40114f
datatype_idx  = 1
complex_idx   = 0
fn_index      = 30
cu_index      = 0
member= name 1
member= value 4
[*] testcase4(data4_t param<...>)
[*] examples/test2.cc:36
```

### Running test case 3
commands
```sh
cd /poc
make run_test3
```
outputs
```sh
/poc/debugger /poc/examples/bin/test3
0x0000000000401177	(Set breakpoint for main)
0x000000000040115a	(Set breakpoint for _Z9testcase47data4_ti)
0x000000000040113e	(Set breakpoint for _Z9testcase37data3_ti)
0x0000000000401130	(Set breakpoint for _Z9testcase27data2_ti)
0x0000000000401122	(Set breakpoint for _Z9testcase17data1_ti)

............................................
[*] 0x401177
[*] main(
	int argc<1>,
	char **argv<0x7ffe2a7895f8 = '[ 0x7ffe2a789ebb= ]'>)
[*] examples/test3.cc:38
............................................
[*] 0x401122
[*] testcase1()
[*] examples/test3.cc:26
............................................
[*] 0x401130
datatype_idx  = 1
complex_idx   = 0
fn_index      = 32
cu_index      = 0
member= name 1
member= value 4
[*] testcase2(
	data2_t param<...>,
	int param2<900>)
[*] examples/test3.cc:29
............................................
[*] 0x40113e
datatype_idx  = 1
complex_idx   = 0
fn_index      = 31
cu_index      = 0
member= value 4
member= name 1
[*] testcase3(
	data3_t param<...>,
	int param2<28335728>)
[*] examples/test3.cc:32
............................................
[*] 0x40115a
datatype_idx  = 1
complex_idx   = 0
fn_index      = 30
cu_index      = 0
member= name 1
member= value 4
[*] testcase4(
	data4_t param<...>,
	int param2<444>)
[*] examples/test3.cc:35
```

### Running test case 4
commands
```sh
cd /poc
make run_test4
```
outputs
```sh
/poc/debugger /poc/examples/bin/test4
0x000000000040114e	(Set breakpoint for main)
0x0000000000401143	(Set breakpoint for _Z9testcase4P7data4_t)
0x0000000000401138	(Set breakpoint for _Z9testcase3P7data3_t)
0x000000000040112d	(Set breakpoint for _Z9testcase2P7data2_t)
0x0000000000401122	(Set breakpoint for _Z9testcase1P7data1_t)

............................................
[*] 0x40114e
[*] main(
	int argc<1>,
	char **argv<0x7ffe78c63b78 = '[ 0x7ffe78c63ebb= ]'>)
[*] examples/test4.cc:40

............................................
[*] 0x401122
[*] testcase1(data1_t *param<0x7ffe78c63a78>)
[*] examples/test4.cc:27
............................................
[*] 0x40112d
[*] testcase2(data2_t *param<0x7ffe78c63a70>)
[*] examples/test4.cc:30
............................................
[*] 0x401138
[*] testcase3(data3_t *param<0x7ffe78c63a60>)
[*] examples/test4.cc:33
............................................
[*] 0x401143
[*] testcase4(data4_t *param<0x7ffe78c63a50>)
[*] examples/test4.cc:36
```