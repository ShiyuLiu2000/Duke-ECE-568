# Use the latest ubuntu base image
FROM ubuntu:latest

# 
RUN apt-get -y update && apt-get install -y
# Use the GCC base image
FROM gcc:latest

# Update package list and install necessary dependencies
RUN apt-get update && \
    apt-get install -y \
    g++ \
    libboost-all-dev \
    # Other dependencies you may need
# Copy source code into the image

RUN mkdir /var/log/erss

ADD . /var/log/erss/

WORKDIR /var/log/erss

ENTRYPOINT ["./run.sh"]
# Set working directory
#WORKDIR /usr/src/erss_http_proxy

# Compile the source code
#RUN g++ -o request request.hpp

# Set execute permissions
RUN chmod +x request

# Define the default command to run when the container starts
##CMD ["./request"]
