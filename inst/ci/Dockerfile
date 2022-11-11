FROM tuxmake/clang-15

RUN apt-get update -y ; apt-get install -y curl

RUN curl -Ls https://github.com/r-lib/rig/releases/download/latest/rig-linux-latest.tar.gz | tar xz -C /usr/local

RUN rig add devel

RUN mkdir ~/.R

RUN apt-get install -y libc++abi-15-dev libc++-15-dev

RUN printf 'CC = clang\nCXX = clang++ -stdlib=libc++\nCXX11 = clang++ -stdlib=libc++\nCXX14 = clang++ -stdlib=libc++\nCXX17 = clang++ -stdlib=libc++\nCXX20 = clang++ -stdlib=libc++\n' > ~/.R/Makevars

RUN flags="-O2 -g -Wunneeded-internal-declaration -Winvalid-utf8 -Wformat -Wsizeof-pointer-div -Wliteral-conversion -Wempty-body -Wreturn-stack-address -Wnon-c-typedef-for-linkage -Wstrict-prototypes"; \
    echo "CFLAGS=$flags" >> ~/.R/Makevars; \
    echo "CXXFLAGS=$flags" >> ~/.R/Makevars; \
    echo "CXX11FLAGS=$flags" >> ~/.R/Makevars; \
    echo "CXX14FLAGS=$flags" >> ~/.R/Makevars; \
    echo "CXX17FLAGS=$flags" >> ~/.R/Makevars; \
    echo "CXX20FLAGS=$flags" >> ~/.R/Makevars

RUN apt-get install -y nano git libxml2-dev libfontconfig1-dev libharfbuzz-dev  libproj-dev libgeos-dev libgdal-dev

RUN export MAKEFLAGS=-j15

RUN R -e 'options(Ncpus = 15); install.packages("remotes"); remotes::install_cran(c("markdown", "spelling", "jsonlite"));'

#docker build . -t hypertidy/clang-wconversion
#docker run -it --rm -v $(pwd):/vapour  hypertidy/clang-wconversion

## now edit src/Makevars.in to put -Wconversion ito CPPFLAGS, and: 
#R CMD build vapour --no-build-vignettes
#R CMD INSTALL vapour_0.?.?.tar.gz > INSTALL.out 2>&1
