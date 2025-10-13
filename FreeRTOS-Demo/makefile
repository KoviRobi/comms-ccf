p ?= minsizerel
.PHONY: all run compare
all:
	cmake --preset $p
	cmake --build --preset $p

run:
	cmake --preset $p
	cmake --build --preset $p \
	--target run

compare:
	cmake --preset $p
	cmake --build --preset $p \
	--target FreeRTOS-Demo-measure
	mkdir -p misc
	echo '# Comparing sizes for build $p'    > misc/compare.md
	echo                                    >> misc/compare.md
	echo '- Files looked at (in build/$p):' >> misc/compare.md
	printf '  - %s\n' \
	    build/$p/FreeRTOS-Demo-*.map        >> misc/compare.md
	python ../python/binutils_mapfile \
	    build/$p/FreeRTOS-Demo-*.map  \
	    --svg misc/compare.svg        \
	    --yaml misc/compare.yaml
	cat misc/compare.*-*.yaml               >> misc/compare.md
