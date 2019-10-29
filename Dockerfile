FROM ubuntu:18.04

RUN apt-get update && apt-get install -y build-essential libtool autotools-dev automake pkg-config libssl-dev libevent-dev bsdmainutils python3 libminiupnpc-dev libzmq3-dev libboost-all-dev libdb-dev libdb++-dev

COPY . /app

RUN cd /app && ./autogen.sh
RUN mkdir /app/build
RUN cd /app/build && ../configure --without-gui
RUN cd /app/build && make -j5

FROM ubuntu:18.04

RUN apt-get update && apt-get install -y libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-test-dev libboost-thread-dev libdb-dev libdb++-dev libssl-dev libevent-dev libminiupnpc-dev libzmq3-dev

COPY --from=0 /app/build/src/bitcoind /bitcoind
ENTRYPOINT ["/bitcoind"]
