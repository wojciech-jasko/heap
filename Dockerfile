FROM python:3.6.9-stretch

RUN apt update && \
    apt upgrade -y && \
    apt install -y \
    #--- Development files -----
    build-essential \
    checkinstall \
    ruby \
    cmake

ADD . /repo

WORKDIR /repo

CMD make
