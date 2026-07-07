# ----------------------------
# Builder stage
# ----------------------------
FROM ubuntu:26.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive 

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ninja-build \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*
    
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && /opt/vcpkg/bootstrap-vcpkg.sh
    
ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

WORKDIR /app

COPY CMakeLists.txt .
COPY CMakePresets.json .
COPY vcpkg-configuration.json .
COPY vcpkg.json .
COPY Toaster ./Toaster

ARG BUILD_PRESET=release

RUN cmake --preset ${BUILD_PRESET} -DENABLE_POST_BUILD=OFF
RUN cmake --build . --preset ${BUILD_PRESET}-build --parallel

# ----------------------------
# Runtime stage
# ----------------------------
FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

ARG BUILD_PRESET=release

COPY --from=builder /app/build/${BUILD_PRESET}/ToasterBot .
COPY --from=builder /app/build/${BUILD_PRESET}/_deps/dpp-build/library/libdpp.so.10.1.5 .

ENV LD_LIBRARY_PATH=/app

COPY LICENSE .

ENTRYPOINT ["./ToasterBot"]
CMD []