CMAKE ?= cmake
JOBS ?= 2

default:
	$(CMAKE) -B build/release -DCMAKE_INSTALL_PREFIX="$(CURDIR)" -DCMAKE_BUILD_TYPE=Release ./
	$(MAKE) install -j$(JOBS) -C build/release

debug:
	$(CMAKE) -B build/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON ./
	$(MAKE) -j$(JOBS) -C build/debug

dbg: debug

clean:
	if [ -d "build/release" ]; then make -C build/release clean; fi
	if [ -d "build/debug" ]; then make -C build/debug clean; fi

clobber:
	rm -rf build bin
