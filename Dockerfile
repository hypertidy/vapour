## Emacs, make this -*- mode: sh; -*-

FROM rhub/rocker-gcc-san

MAINTAINER "hypertidy admin" mdsumner@gmail.com



RUN apt-get update \
    && apt-get install -y --no-install-recommends software-properties-common dirmngr  \
    && wget -qO- https://cloud.r-project.org/bin/linux/ubuntu/marutter_pubkey.asc | tee -a /etc/apt/trusted.gpg.d/cran_ubuntu_key.asc \
    %% add-apt-repository "deb https://cloud.r-project.org/bin/linux/ubuntu $(lsb_release -cs)-cran40/" \
    && add-apt-repository -y ppa:c2d4u.team/c2d4u4.0+ \
    && apt-get update && apt-get upgrade -y \ 
    && apt-get install -y r-cran-devtools r-cran-markdown r-cran-spelling 
    && apt-get install -y  git g++ libproj-dev cmake pkg-config make  libnetcdf-dev  \
                 libgeos-dev python3-dev libhdf4-dev libhdf5-dev \
                 && git clone https://github.com/osgeo/gdal \
                 && cd gdal \
                 && mkdir build \
                 && cd build \
                 && cmake .. \
                 && cmake --build . --parallel 28 \
                 && cmake --build . --target install \
                 && ldconfig 
                 
ENV HYPERTIDY_PLATFORM rocker-gcc-san-gdal
