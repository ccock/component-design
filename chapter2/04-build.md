# 组件构建

## 基于包构建的交叉工具链选择


RPM，自研工具：

根据arch设置环境变量：CC，CFLAGS...
根据build requires，下载依赖，然后设置对应的头文件到环境变量 -I中；

CONAN：

参见：
- [cross compile](https://docs.conan.io/en/latest/systems_cross_building/cross_building.html#cross-building)
- [cross compile in docker](https://docs.conan.io/en/latest/howtos/run_conan_in_docker.html?highlight=toolchain)
- [build context and host context](https://docs.conan.io/en/latest/devtools/build_requires.html?highlight=toolchain)
- [build root for embedded linux](https://docs.conan.io/en/latest/integrations/cross_platform/buildroot.html?highlight=toolchain)
- [yocto for conan](https://docs.conan.io/en/latest/integrations/cross_platform/yocto.html?highlight=toolchain)

定义不同的profile，每个profile可以自定义编译器和环境变量：

```yml
# profile1

 [settings]
 zlib:compiler=clang
 zlib:compiler.version=3.5
 zlib:compiler.libcxx=libstdc++11
 compiler=gcc
 compiler.version=4.9
 compiler.libcxx=libstdc++11

 [env]
 zlib:CC=/usr/bin/clang
 zlib:CXX=/usr/bin/clang++
```

环境变量可以按照包级别去指定，例如上面：Your build tool will locate clang compiler only for the zlib package and gcc (default one) for the rest of your dependency tree.

再例如：

```yml
# profile

toolchain=/usr/x86_64-w64-mingw32 # Adjust this path
target_host=x86_64-w64-mingw32
cc_compiler=gcc
cxx_compiler=g++

[env]
CONAN_CMAKE_FIND_ROOT_PATH=$toolchain
CHOST=$target_host
AR=$target_host-ar
AS=$target_host-as
RANLIB=$target_host-ranlib
CC=$target_host-$cc_compiler
CXX=$target_host-$cxx_compiler
STRIP=$target_host-strip
RC=$target_host-windres

[settings]
# We are cross-building to Windows
os=Windows
arch=x86_64
compiler=gcc

# Adjust to the gcc version of your MinGW package
compiler.version=7.3
compiler.libcxx=libstdc++11
build_type=Release
```