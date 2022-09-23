default:
	cmake -B release -DCMAKE_INSTALL_PREFIX=$(shell pwd) -DCMAKE_BUILD_TYPE=Release ../
	make install -j${nproc} -C release

debug:
	cmake -B debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE=ON ../
	make -j${nproc} -C debug

dbg: debug

clean:
	if [ -d "release" ]; then make -C release clean; fi
	if [ -d "debug" ]; then make -C debug clean; fi

clobber:
	rm -rf release debug bin lib
