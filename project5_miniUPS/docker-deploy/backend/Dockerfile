FROM ubuntu:20.04
RUN ln -fs /usr/share/zoneinfo/UTC /etc/localtime
RUN mkdir /code
WORKDIR /code
ADD . /code/
RUN apt-get update 
RUN apt-get install -y make gcc g++
RUN apt-get install -y libprotobuf-dev
ENV LD_LIBRARY_PATH="/usr/local/lib:/usr/lib:/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}"
RUN apt-get install -y protobuf-compiler
RUN apt-get install -y postgresql postgresql-contrib
RUN apt-get install -y build-essential apt-utils libpqxx-dev libpq-dev bc libpugixml-dev
RUN protoc --cpp_out=. *.proto
RUN ldconfig
RUN make clean && make