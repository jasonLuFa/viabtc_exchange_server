FROM ubuntu:22.04

# Avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install all required dependencies in a single RUN
RUN apt-get clean && \
    apt-get update && \
    apt-get install -y \
    netcat \
    software-properties-common \
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
    # Process management tools
    psmisc \
    # Python and pip for testing
    python3 \
    python3-pip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Python WebSocket testing dependencies
RUN pip3 install websockets

# Set working directory
WORKDIR /workspace
