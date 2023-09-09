#!/bin/bash

# Configuration
SYSROOT_PATH=$(realpath ./sysroot)
PATCHES_PATH=$(realpath ./patches)
TOOLCHAIN_PATH=$(realpath ./toolchain)

# Check for required tools
check_dependencies() {
    local dependencies=("git" "wget" "tar" "patch" "meson" "ninja")
    for dep in "${dependencies[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            echo "Error: $dep is not installed. Please install it and try again."
            exit 1
        fi
    done
}

# Create toolchain directory
create_toolchain_directory() {
    if [ ! -d "$TOOLCHAIN_PATH" ]; then
        mkdir -p "$TOOLCHAIN_PATH"
    fi
    cd "$TOOLCHAIN_PATH"
}

# Download and build mlibc
build_mlibc() {
    echo "Downloading mlibc..."
    git clone https://github.com/managarm/mlibc -b release-4.0
    echo "Building mlibc headers..."
    cd mlibc
    patch -N -p1 < "$PATCHES_PATH/mlibc.patch"
    meson setup --cross-file ci/umbra.cross-file --prefix="$SYSROOT_PATH/usr/" --buildtype=release build
    ninja -j$(nproc) -C build
    ninja install -C build
    cd "$TOOLCHAIN_PATH"
}

# Download and build binutils
build_binutils() {
    if [ ! -f "binutils-2.41.tar.xz" ]; then
        echo "Downloading binutils..."
        wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz
    fi
    if [ ! -d "binutils-2.41" ]; then
        echo "Extracting binutils..."
        tar xf binutils-2.41.tar.xz
    fi
    echo "Building binutils..."
    cd binutils-2.41
    patch -N -p1 < "$PATCHES_PATH/binutils.patch"
    ./configure --target=x86_64-umbra --prefix="$SYSROOT_PATH" --with-sysroot="$SYSROOT_PATH" --disable-nls --disable-werror
    make -j$(nproc)
    make install-strip
    cd "$TOOLCHAIN_PATH"
}

# Download and build autoconf
build_autoconf() {
    if [ ! -f "autoconf-2.69.tar.xz" ]; then
        echo "Downloading autoconf..."
        wget https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.xz
    fi
    if [ ! -d "autoconf-2.69" ]; then
        echo "Extracting autoconf..."
        tar xf autoconf-2.69.tar.xz
    fi
    echo "Building autoconf..."
    cd autoconf-2.69
    ./configure --prefix="$SYSROOT_PATH"
    make -j$(nproc)
    make install
    cd "$TOOLCHAIN_PATH"
}

# Download and build gcc
build_gcc() {
    if [ ! -f "gcc-13.2.0.tar.xz" ]; then
        echo "Downloading gcc..."
        wget https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz
    fi
    if [ ! -d "gcc-13.2.0" ]; then
        echo "Extracting gcc..."
        tar xf gcc-13.2.0.tar.xz
    fi
    echo "Building gcc..."
    if [ ! -d "build_gcc" ]; then
        mkdir build_gcc
        mv "gcc-13.2.0" "build_gcc"
    fi
    cd build_gcc/gcc-13.2.0
    patch -d . -N -p1 < "$PATCHES_PATH/gcc.patch"
    contrib/download_prerequisites
    cd gcc
    "$SYSROOT_PATH/bin/autoconf"
    cd libstdc++-v3
    "$SYSROOT_PATH/bin/autoconf"
    cd ../../
    ./gcc-13.2.0/configure --target=x86_64-umbra --prefix="$SYSROOT_PATH" --disable-nls --enable-languages=c,c++ --without-headers --with-sysroot="$SYSROOT_PATH" --enable-initfini-array --disable-multilib --with-system-zlib --enable-shared --enable-host-shared
    make -j$(nproc) all-gcc
    make -j$(nproc) all-target-libgcc
    make -j$(nproc) all-target-libstdc++-v3
    make install-strip-gcc
    make install-strip-target-libgcc
    make install-strip-target-libstdc++-v3
    cd "$TOOLCHAIN_PATH"
}

# Main script
check_dependencies
create_toolchain_directory
echo "Starting the build process..."
build_mlibc
build_binutils
build_autoconf
build_gcc
echo "Build process completed successfully."
