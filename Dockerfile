FROM condaforge/miniforge3

SHELL ["/bin/bash", "-c"]

# Clone SpiceQL repo instead of copying?
#git clone git@github.com:DOI-USGS/SpiceQL.git /repo

# RUN git clone https://github.com/DOI-USGS/SpiceQL.git /repo --recursive 
# RUN echo $(ls /repo)
# RUN chmod -R 755 /repo

RUN mkdir /repo    
COPY . /repo
RUN echo $(ls /repo)
RUN chmod -R 755 /repo

# Set repo root env
ENV SPICEQL_REPO_ROOT /repo

# Need to mount ISIS data area
ENV SSPICE_DEBUG=TRUE
ENV SPICEROOT=/mnt/isis_data
ENV SPICEQL_LOG_LEVEL=TRACE

RUN apt-get update && apt-get install build-essential -y

# CMD ["/bin/bash"]
# RUN /bin/bash -c "source activate spiceql"
RUN mamba env create -f ${SPICEQL_REPO_ROOT}/environment.yml -n spiceql && \
    source /opt/conda/etc/profile.d/conda.sh && \
    conda init && \
    conda activate spiceql && \
    # conda install -c conda-forge spiceql && \
    echo "source activate spiceql" > ~/.bashrc && \ 
    conda activate spiceql && \ 
    cd $SPICEQL_REPO_ROOT && mkdir -p build && cd build && \ 
    cmake .. -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX -DSPICEQL_BUILD_TESTS=OFF && \ 
    make install 

RUN cd ${SPICEQL_REPO_ROOT}/fastapi
ENV PATH /opt/conda/envs/spiceql/bin:$PATH

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update
RUN apt-get install nginx curl -y 
COPY fastapi/config/nginx.conf /etc/nginx/nginx.conf

WORKDIR ${SPICEQL_REPO_ROOT}/fastapi

EXPOSE 8080

CMD nginx && uvicorn app.main:app --reload --port 8080
