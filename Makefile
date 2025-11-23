
BUNDLER = z-core/zbundler.py
SRC = src/zlist.c
DIST = zlist.h

all: bundle

bundle:
	@echo "Bundling $(DIST)..."
	python3 $(BUNDLER) $(SRC) $(DIST)

init:
	git submodule update --init --recursive

