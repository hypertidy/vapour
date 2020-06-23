FROM osgeo/gdal:ubuntu-full-latest

MAINTAINER "mdsumner" mdsumner@gmail.com

RUN apt-get update && apt-get install -y software-properties-common

RUN echo "deb http://cran.rstudio.com/bin/linux/ubuntu bionic-cran35/  " >> /etc/apt/sources.list
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E298A3A825C0D65DFD57CBB651716619E084DAB9

RUN apt-get update
RUN apt-get upgrade -y

RUN export DEBIAN_FRONTEND=noninteractive; apt-get -y update \
  && apt-get install -y \
	git \
	libcurl4-openssl-dev \
	libssl-dev \
	libxml2-dev \
	make \
	qpdf \
	r-base-dev \
	emacs

RUN Rscript -e 'install.packages("remotes")'
RUN Rscript -e 'options(Ncpus = parallel::detectCores() - 1); remotes::install_cran(c("Rcpp", "knitr", "covr", "dplyr", "geojsonsf", "testthat", "rbenchmark", "rmarkdown", "spelling"))'

#RUN git config --global user.email "mdsumner@gmail.com"
RUN git clone  https://github.com/hypertidy/vapour.git
RUN vers=$(grep ^Version vapour/DESCRIPTION | sed 's/Version: //')
RUN R CMD build --no-manual vapour
RUN R CMD INSTALL "vapour_$vers.tar.gz"
RUN R CMD check --no-manual "vapour_$vers.tar.gz"

CMD ["/bin/bash"]
