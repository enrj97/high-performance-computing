FROM ubuntu

RUN apt-get update && apt-get install -y --no-install-recommends openmpi-bin build-essential g++ make libopenmpi-dev
COPY graph500-graph500-3.0.0 /opt/graph500/
WORKDIR /opt/graph500/src
RUN make all

