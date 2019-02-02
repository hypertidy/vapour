system('R -d  "valgrind --track-origins=yes --leak-check=full" -f tests/grindy/test.R >&1 | tee valgrind')

vg <- readLines("tests/grindy/valgrind")
as.integer(gsub("\\)", "", unlist(lapply(strsplit(grep("vapour\\.cpp:", vg, value = TRUE), "\\.cpp:"), "[", 2)))) -> idx
tx <- readLines("src/vapour.cpp")
tx[idx]

