FROM ubuntu:20.04

# set time to UTC
RUN ln -fs /usr/share/zoneinfo/UTC /etc/localtime

RUN apt-get update 
RUN apt-get install -y make gcc g++
RUN apt-get install -y postgresql postgresql-contrib
RUN apt-get install -y build-essential apt-utils libpqxx-dev libpq-dev bc libpugixml-dev

RUN mkdir /code
ADD . /code/
WORKDIR /code
RUN chmod u+x ./run.sh
RUN make clean && make