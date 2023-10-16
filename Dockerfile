FROM amd64/ubuntu:latest

RUN apt-get update && apt-get install -y \
    curl \
    g++ \
    flex \
    bison \
    make \
    libwww-perl \
    git \
    patch \
    wget \
    unzip \
    zlib1g-dev \
    openjdk-11-jdk

WORKDIR /app

# Now, copy your local files
COPY . .

RUN make minisat2-download
RUN make
