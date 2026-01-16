# Use Manylinux 2.28 x86_64 (matches your cibuildwheel target)
FROM quay.io/pypa/manylinux_2_28_x86_64

# Install Python tools
RUN /opt/python/cp310-cp310/bin/pip install --upgrade pip wheel auditwheel

RUN dnf install -y epel-release
RUN dnf install -y netcdf-devel netcdf-fortran-devel
RUN dnf install -y tree wget zip lapack-devel
RUN dnf config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel8/x86_64/cuda-rhel8.repo
RUN dnf install --setopt=obsoletes=0 -y \
    cuda-nvcc-12-8 \
    cuda-cudart-devel-12-8 \
    libcurand-devel-12-8 \
    libcublas-devel-12-8 
RUN ln -sf cuda-12.8 /usr/local/cuda

# Create a working directory
WORKDIR /io

# Copy your wheel and repair script into the container
COPY wheelhouse/*.whl /io/

ENV PATH="/opt/python/cp310-cp310/bin:${PATH}"

CMD ["chmod", "+x", "python/tools/repair_wheel_gpu.sh"]
