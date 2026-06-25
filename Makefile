BINARY=tinyfetch
CXX=g++
CXXFLAGS=-std=c++17 -Wall -Wextra -O3
SRC_DIR=src
SRCS=$(SRC_DIR)/main.cpp $(SRC_DIR)/sysinfo.cpp $(SRC_DIR)/render.cpp $(SRC_DIR)/export.cpp

.PHONY: build install clean test

build: $(BINARY)

$(BINARY): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(BINARY) $(SRCS)
	@echo "Built $(BINARY) (C++ version)"

install: build
	@if [ -d "ascii" ]; then \
		mkdir -p /usr/local/share/tinyfetch/ascii; \
		cp -r ascii/* /usr/local/share/tinyfetch/ascii/; \
		echo "Installed ASCII assets to /usr/local/share/tinyfetch/ascii/"; \
	fi
	@if [ -f "$(BINARY)" ]; then \
		install -m 0755 $(BINARY) /usr/local/bin/$(BINARY); \
		echo "Installed built $(BINARY) binary to /usr/local/bin/$(BINARY)"; \
	elif [ -f "scripts/tinyfetch.sh" ]; then \
		install -m 0755 scripts/tinyfetch.sh /usr/local/bin/$(BINARY); \
		echo "Installed scripts/tinyfetch.sh as /usr/local/bin/$(BINARY)"; \
	else \
		echo "Nothing to install"; exit 1; \
	fi

test: build
	./tests/test.sh

clean:
	rm -f $(BINARY) $(SRC_DIR)/*.o
