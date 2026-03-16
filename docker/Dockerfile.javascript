FROM emscripten/emsdk:4.0.22

WORKDIR /workspace

# Copy repository into image
COPY . /workspace

# Install CMake >= 3.24 (prebuilt binary from Kitware) and handle CPU arch
RUN set -eux; \
		apt-get update; apt-get install -y --no-install-recommends ca-certificates wget; \
		arch="$(uname -m)"; \
		case "$arch" in \
			x86_64|amd64) pkg="cmake-3.24.0-linux-x86_64.sh" ;; \
			aarch64|arm64) pkg="cmake-3.24.0-linux-aarch64.sh" ;; \
			*) echo "Unsupported architecture: $arch"; exit 1 ;; \
		esac; \
		url="https://github.com/Kitware/CMake/releases/download/v3.24.0/${pkg}"; \
		wget -O /tmp/cmake.sh "$url"; \
		mkdir -p /opt/cmake; sh /tmp/cmake.sh --skip-license --prefix=/opt/cmake; \
		ln -s /opt/cmake/bin/* /usr/local/bin/; rm /tmp/cmake.sh; \
		apt-get remove -y wget; apt-get -y autoremove; rm -rf /var/lib/apt/lists/*

# Install Node dependencies
RUN npm ci

RUN npm run build