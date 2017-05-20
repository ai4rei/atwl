gcc -DFORCE_CRT -DNO_INLINE_ASSEMBLY -I../snippets -I../zlibstatic gzio.c gzio-gcc-mem.c ../snippets/bvcrc32.c ../snippets/bvdata.cpp ../snippets/bvgzio.cpp -lz -o gzio
