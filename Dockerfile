FROM ghcr.io/wiiu-env/devkitppc:20260225

RUN apt-get update && apt-get install -y python3 python3-pip \
    && rm -rf /usr/lib/python3.11/EXTERNALLY-MANAGED \
    && pip3 install pycryptodome \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

ENV DEVKITPRO=/opt/devkitpro
ENV DEVKITARM=${DEVKITPRO}/devkitARM
ENV PATH=${DEVKITPRO}/tools/bin:${DEVKITARM}/bin:${PATH}

WORKDIR /project

CMD ["make"]
