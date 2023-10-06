CC=gcc
CFLAGS=-O0 -Werror=vla -std=gnu11 -g -fsanitize=address -pthread -lrt -lm
PERFFLAGS=-O0 -march=native -Werror=vla -std=gnu11 -pthread -lrt -lm
TESTFLAGS=-O0 -Werror=vla -std=gnu11 -g -fprofile-arcs -ftest-coverage -fsanitize=address -pthread -lrt -lm
NAME=btreestore
OBJECT=lib$(NAME).o
LIBRARY=lib$(NAME).a

correctness: btreestore.c
	$(CC) -c $(CFLAGS) $^ -o $(OBJECT)
	ar rcs $(LIBRARY) $(OBJECT)

performance: btreestore.c
	$(CC) -c $(PERFFLAGS) $^ -o $(OBJECT)
	ar rcs $(LIBRARY) $(OBJECT)

tests: btreestore.c
	$(CC) -c $(TESTFLAGS) $^ -o $(OBJECT)
	ar rcs $(LIBRARY) $(OBJECT)

	gcc unit_tests.c -o unit_test  -L"." -lcmocka-static -lbtreestore -lm -g $(TESTFLAGS)
	gcc tests.c -o e2e_test  -L"." -lcmocka-static -lbtreestore -lm -g $(TESTFLAGS)


buildTests :
	gcc unit_tests.c -o unit_test  -L"." -lcmocka-static -lbtreestore -lm -g $(TESTFLAGS)
	gcc tests.c -o e2e_test  -L"." -lcmocka-static -lbtreestore -lm -g $(TESTFLAGS)

runTest:
	./unit_test
	./e2e_test diff_tests
	./run_diff_tests.sh