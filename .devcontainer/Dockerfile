FROM ubuntu:22.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Add required repositories
RUN apt-get update && apt-get install -y software-properties-common && \
    apt-get update

# Install all required dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    make \
    cmake \
    git \
    curl \
    vim \
    nano \
    gdb \
    valgrind \
    # Required dependencies
    libev-dev \
    libjansson-dev \
    libmysqlclient-dev \
    libhttp-parser-dev \
    libcurl4-openssl-dev \
    libmpdec-dev \
    # Additional required dependencies
    liblz4-dev \
    libsqlite3-dev \
    libpq-dev \
    libldap2-dev \
    libdb-dev \
    libsasl2-dev \
    # Redis
    libhiredis-dev \
    # Kafka related
    librdkafka-dev \
    # Zookeeper
    libzookeeper-mt-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace
