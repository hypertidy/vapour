FROM r-base:latest

# dev requirements for devtools
RUN apt-get update && \
  apt-get -y install libxml2-dev libcurl4-openssl-dev libssl-dev \
  libgit2-dev \
  libfontconfig1-dev libfreetype6-dev libharfbuzz-dev libfribidi-dev libpng-dev libtiff5-dev libjpeg-dev
RUN Rscript -e 'install.packages("devtools", dependencies = TRUE)'

# install compiler and dev/debug tools
RUN apt-get update && \
  apt-get -y install clang lldb valgrind git

RUN apt-get install -y nano git libxml2-dev libfontconfig1-dev libharfbuzz-dev  libproj-dev libgeos-dev libgdal-dev

RUN export MAKEFLAGS=-j15

RUN R -e 'options(Ncpus = 15); install.packages("remotes"); remotes::install_cran(c("markdown", "spelling", "jsonlite", "devtools"));'
# use clang and -O0 so that we can use lldb
RUN mkdir ~/.R && \
  echo 'CC=clang' >> ~/.R/Makevars && \
  echo 'CXX=clang++' >> ~/.R/Makevars && \
  echo 'CXX11=clang++' >> ~/.R/Makevars && \
  echo 'CFLAGS+=-g -O0' >> ~/.R/Makevars && \
  echo 'CXXFLAGS+=-g -O0' >> ~/.R/Makevars && \
  echo 'CXX11FLAGS+=-g -O0' >> ~/.R/Makevars

# install debugit at /pkg
RUN git clone https://github.com/DavisVaughan/debugit.git /pkg
WORKDIR /pkg
ENTRYPOINT [ ]
CMD [ "/bin/bash" ]
