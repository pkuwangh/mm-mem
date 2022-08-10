default:
	cmake3 -B build -DCMAKE_BUILD_TYPE=Release ../
	make -j${nproc} -C build

debug:
	cmake3 -B build -DCMAKE_BUILD_TYPE=Debug ../
	make -j${nproc} -C build

dbg: debug

clean:
	make -C build clean

clobber:
	rm -rf build
