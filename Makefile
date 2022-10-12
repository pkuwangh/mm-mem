default:
	cmake -B build/release -DCMAKE_INSTALL_PREFIX=$(shell pwd) -DCMAKE_BUILD_TYPE=Release ./
	make install -j${nproc} -C build/release

debug:
	cmake -B build/debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON ./
	make -j${nproc} -C build/debug

dbg: debug

clean:
	if [ -d "build/release" ]; then make -C build/release clean; fi
	if [ -d "build/debug" ]; then make -C build/debug clean; fi

clobber:
	rm -rf build bin
