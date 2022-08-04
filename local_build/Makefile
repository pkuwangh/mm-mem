default:
	cmake3 -B build -DCMAKE_BUILD_TYPE=Release ../
	make -j16 -C build

debug:
	cmake3 -B build -DCMAKE_BUILD_TYPE=Debug ../
	make -j16 -C build

dbg: debug

clean:
	make -C build clean

clobber:
	rm -rf build
