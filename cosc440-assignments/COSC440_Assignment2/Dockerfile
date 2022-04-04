FROM ubuntu
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 \
    python3-pip \
    python3-dev \
    build-essential \
    gcc


# wheel & setuptools are used to build packages
RUN pip3 install wheel setuptools

# install tensorflow (comes with numpy 1.19)
RUN pip3 install tensorflow matplotlib

# update numpy to numpy 1.21 (needed for https://numpy.org/devdocs/reference/generated/numpy.lib.stride_tricks.sliding_window_view.html)
RUN pip3 install --upgrade numpy

WORKDIR /src
COPY ./src /src

ENTRYPOINT /bin/bash
