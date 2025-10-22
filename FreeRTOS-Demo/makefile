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
	mkdir -p build/$p/compare
	echo '# Comparing sizes for build $p'    > build/$p/compare/compare.md
	echo                                    >> build/$p/compare/compare.md
	echo '- Files looked at (in build/$p):' >> build/$p/compare/compare.md
	printf '  - %s\n' \
	    build/$p/FreeRTOS-Demo-*.map        >> build/$p/compare/compare.md
	uv run binutils-mapfile              \
	    build/$p/FreeRTOS-Demo-*.map     \
	    --svg  build/$p/compare/compare.svg \
	    --yaml build/$p/compare/compare.yaml
	cat build/$p/compare/compare.*-*.yaml >> build/$p/compare/compare.md
	echo ''                             > build/$p/compare/table.md
	python compare/update_table.py     >> build/$p/compare/table.md
	echo ''                            >> build/$p/compare/table.md
	echo '<!-- Table ends here -->'    >> build/$p/compare/table.md
	sed -i \
	    -e '/<!-- Table starts here -->/p' \
	    -e '/<!-- Table ends here -->/r build/$p/compare/table.md' \
	    -e '/<!-- Table starts here -->/,/<!-- Table ends here -->/d' \
	    ../README.md
	sed -i \
	    -e '/<!-- Table starts here -->/p' \
	    -e '/<!-- Table ends here -->/r build/$p/compare/table.md' \
	    -e '/<!-- Table starts here -->/,/<!-- Table ends here -->/d' \
	    README.md
