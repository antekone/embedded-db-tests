test: test.cpp
	g++ test.cpp -o test -std=c++11 -llmdb

.PHONY: run
run:
	@(rm -r testdb; mkdir testdb; ./test) || true
