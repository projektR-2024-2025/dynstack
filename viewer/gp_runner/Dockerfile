# Build stage
FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    cmake \
    build-essential \
    git \
    pkg-config \
    protobuf-compiler \
    libprotobuf-dev \
    libzmq3-dev \
    && rm -rf /var/lib/apt/lists/*

# # Install cppzmq from source
RUN cd /tmp && git clone https://github.com/zeromq/cppzmq.git && \
    cd cppzmq && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local && \
    cmake --install .

# Clone and build ECF
RUN cd /tmp && git clone https://github.com/djakobovic/ECF.git && \
    cd ECF && \
    mkdir build && cd build && \
    cmake .. -DCMAKE_CXX_FLAGS="-fpermissive" && \
    cmake --build . --config Release --parallel && \
    mkdir -p /usr/local/lib /usr/local/include/ECF && \
    cp ./libECF.a /usr/local/lib/ && \
    cp -r ../ECF /usr/local/include/

WORKDIR /app

# Copy project files
COPY . .

RUN cd src && protoc ./hotstorage_model.proto --cpp_out=hotstorage

# Build
RUN cmake -S . -B ./build/ \
    -DCMAKE_CXX_FLAGS="-fpermissive" \
    -DCMAKE_BUILD_TYPE=Release && \
    cmake --build ./build/

# Runtime stage
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libzmq5 \
    libprotobuf23 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/stacking /usr/local/bin/

ENTRYPOINT ["stacking"]
CMD ["tcp://dynview:2222", "658f9b28-6686-40d2-8800-611bd8466215", "HS"]