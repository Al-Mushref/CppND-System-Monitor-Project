CXX=g++
CXX_FLAGS=-std=c++20 -Iinclude -gdwarf-4 -O0
SAN=-fsanitize=address,null -fno-omit-frame-pointer 

exec: bin/exec

bin/exec: ./src/main.cpp ./include/linux_parser.h ./src/linux_parser.cpp ./include/process.h ./src/process.cpp
	$(CXX) $(CXX_FLAGS) ./src/main.cpp ./src/linux_parser.cpp  ./src/process.cpp -o $@

.PHONY: all
all: format test build

.PHONY: format
format:
	clang-format src/* include/* -i

.PHONY: build
build:
	mkdir -p build
	cd build && \
	cmake .. && \
	make

.PHONY: debug
debug:
	mkdir -p build
	cd build && \
	cmake -DCMAKE_BUILD_TYPE=debug .. && \
	make

.PHONY: clean
clean:
	rm -rf build
