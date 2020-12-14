FROM debian:stable-slim

RUN apt-get update && apt-get install -y --no-install-recommends openmpi-bin build-essential g++ make libopenmpi-dev
COPY graph500 /opt/graph500/
WORKDIR /opt/graph500/src
RUN make all

