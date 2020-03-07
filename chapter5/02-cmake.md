## Modern CMAKE

---

CMake 是一个开源的跨平台自动化建构系统，是目前最主流的 C/C++语言构建工具。CMAC3.0 之后引入很多新的特性，进一步的提升了编写构建脚本效率，被称为 Modern CMAKE，本文进行了基本内容的学习汇总。

### Target 概念

---

Modern Cmake 最大的改进是引入了 target， 可以作为一个整体比较方便的被组合和引用来构建整个系统。而旧版本的 CMAKE 则主要是基于 directory 的来构建的，很多的复用则只能靠变量实现。

#### 1. 分类

Target 中最核心的两个分类是：executable, library。

- 其中 executable 是可执行程序，在不同的操作系统平台会有不同的格式，同时一个构建系统中也可能需要生成多个可执行程序。 具体指令如下所示：
  ```
  add_executable(<name> [WIN32] [MACOSX_BUNDLE]
                [EXCLUDE_FROM_ALL]
                [source1] [source2 ...])
  ```
- library 代表链接库，可以分为 share, static, object, module, interface 五个种类。
  - share 代表动态库，并不会添加到最后的可执行文件中。可以动态加载和链接，在运行过程中当被多个程序使用时，可以在内存中被共享。
  - static 代表静态库，会在编译过程中被一起添加到生成的可执行文件中。一但静态库的实现发生变化，必须要重新编译整个系统才可以使用。
  - module 也是共享库的一种，但是不会导出任何符号，不能被连接，只能通过 dlopen 在运行时动态加载使用。
  - object 类型的库则仅是代表一组编译后的文件，并不会执行打包和链接。
  - interface 类型的并不会便于编译输出文件，代表一组接口文件，在编译构建中被其他 target 使用。
  ```
  add_library(<name> [STATIC | SHARED | MODULE |OBJECT |INTERFACE] ...)
  ```

#### 2. target requirement

正常情况下编译一个 target(可执行程序或者库)需要依赖下面内容：

- 源文件列表，通过 target_sources 配置。
- 头文件列表，通过 target_include_directories
- 预编译宏，通过 pre-processor macros 配置
- 编译选项和特性，通过 target_compile_options，target_compile_features 配置。
- 链接选项，通过 target_link_options 配置

在 C/C++软件系统中，一个组件库中大部分的接口是仅在模块内使用，仅有小一部分接口是外部使用的。为了提高编译效率，在编译链接不同组件时仅需要使用组件对外提供的小部分接口文件即可。

为了更好的支持这个特点，Modern Cmake 针对 target 引入两个概念：user requriement(用户依赖) 和 build requirement（编译依赖）。其中用户依赖代表组件使用方需要的依赖，而编译依赖代表当前组件编译构建是需要的依赖。

Modern Cmake 增加了三个关键字 INTERFACE、PUBLIC、PRIVATE 分布表示不同作用域， 以下面以添加头文件依赖命令为例：

```
target_include_directories(<target> [SYSTEM] [BEFORE]
  <INTERFACE|PUBLIC|PRIVATE> [items1...]
  [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
```

给组件添加头文件依赖路径时，INTERFACE 代表添加的头文件路径是仅被组件的使用方使用，编译当前组件并不需要。而 PRIVATE 代表添加的头文件路径仅被当前组件编译时使用。而 PUBLIC 在表示同时被自己和使用方使用。

#### 3. target requirement propagtion

当构建系统中包括比较多的 libary 时，编译和管理这些 Libary 之间的依赖就变得尤为重要。在 Modern Cmake 中针对多个 libary 之间的依赖管理有很大的改善。当给 Libary 定义用户依赖和编译依赖后，并在 target_link_libraries 中定义与其他组件间的依赖关系, Morden CMake 可以自动传递和推演 target 之间的所有编译依赖。

组件建的依赖关系定义命令如下：

```
target_link_libraries(<target>
                      <PRIVATE|PUBLIC|INTERFACE> <item>...
                     [<PRIVATE|PUBLIC|INTERFACE> <item>...]...)
```

- 其中 private 代表： 被依赖 libary 的 user requirement 的会变成当前 target 的 build requirment
- 其中 public 代表：被依赖 libary 的 user requirement 的会变成当前 target 的 build requirment 和 user requirement.
- 其中 INTERFACE 代表：被依赖 libary 的 user requirement 的会变成当前 target 的 user requirment

Morden Cmake 可以根据上门的规则去建立所有组件建的依赖关系。

### package 引入。

package 用来指代一个依赖库，代表一些头文件、动态库、静态库等，并携带了版本信息。 在 cmake 中可以使用 find_package 来导入一个特定版本的第三方库。

例如： find_package(Qt5Gui 5.1.0 CONFIG) ，导入包 Qt5Gui，版本号 5.1.0，通用的导入包的命令如下：

```
find_package(<PackageName> [version][exact] [QUIET][module]
[REQUIRED][COMPONENTS][components...]]
[OPTIONAL_COMPONENTS components...][no_policy_scope])
```

不同语言在使用第三方包时使用的方式存在差异，例如 JAVA 语言 marven 时会下载包到本地公共仓库地址中，构建时直接从本地仓库中选择合适包进行构建。而很多动态语言例如 ruby, nodejs 等则直接把依赖的源码包下载到本地的工程的一个单独目录中使用。

在 C++构建过程中，CMAKE 提供了两个集中的地方来安装和查找第三方库，分别是用户级包仓库和系统级包仓库，使用 find_package 可以在用户级包仓库和系统级仓库中寻找到已经安装的包。

- 用户级包仓库地址

  windows 的注册表 HKEY_CURRENT_USER 下：
  `HKEY_CURRENT_USER\Software\Kitware\CMake\Packages\<PackageName>`

  linux:
  `~/.cmake/packages/<PackageName>`

- 系统级包仓库地址

  windows 的注册表 HKEY_LOCAL_MACHINE 下：
  `HKEY_LOCAL_MACHINE\Software\Kitware\CMake\Packages\<PackageName>`
  在非 windows 系统中没有系统级包仓库地址。

find_package 包括两种，config-file package 和 module package。其中 config-file package, 表示使用的库已经 package cmake 构建，直接拿过来使用，而 Module package 的话是使用的库没有使用 cmake 构建, 需要下游的使用者自己写 cmake 文件。

同样 cmake 原生就提供了 moudle 可以用于制作安装包。下面一个制作 cmake 包的工具。

```
project(UpstreamLib)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

set(Upstream_VERSION 3.4.1)

include(GenerateExportHeader)

add_library(ClimbingStats SHARED climbingstats.cpp)
generate_export_header(ClimbingStats)
set_property(TARGET ClimbingStats PROPERTY VERSION ${Upstream_VERSION})
set_property(TARGET ClimbingStats PROPERTY SOVERSION 3)
set_property(TARGET ClimbingStats PROPERTY
  INTERFACE_ClimbingStats_MAJOR_VERSION 3)
set_property(TARGET ClimbingStats APPEND PROPERTY
  COMPATIBLE_INTERFACE_STRING ClimbingStats_MAJOR_VERSION
)

install(TARGETS ClimbingStats EXPORT ClimbingStatsTargets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)
install(
  FILES
    climbingstats.h
    "${CMAKE_CURRENT_BINARY_DIR}/climbingstats_export.h"
  DESTINATION
    include
  COMPONENT
    Devel
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/ClimbingStats/ClimbingStatsConfigVersion.cmake"
  VERSION ${Upstream_VERSION}
  COMPATIBILITY AnyNewerVersion
)

export(EXPORT ClimbingStatsTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/ClimbingStats/ClimbingStatsTargets.cmake"
  NAMESPACE Upstream::
)
configure_file(cmake/ClimbingStatsConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/ClimbingStats/ClimbingStatsConfig.cmake"
  COPYONLY
)

set(ConfigPackageLocation lib/cmake/ClimbingStats)
install(EXPORT ClimbingStatsTargets
  FILE
    ClimbingStatsTargets.cmake
  NAMESPACE
    Upstream::
  DESTINATION
    ${ConfigPackageLocation}
)
install(
  FILES
    cmake/ClimbingStatsConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/ClimbingStats/ClimbingStatsConfigVersion.cmake"
  DESTINATION
    ${ConfigPackageLocation}
  COMPONENT
    Devel
)
```

在 CMakePackageConfigHelpers 的 moudule 中封装了方法来构造 ConfigVersion.cmake 文件，并在文件中设置了包的相关信息。

使用 find_package 可以用于使用已经安装到本地的仓库，但是很多时候还需要使用服务器上的仓库，在 C++系统中常用下面几种做法：

- 通过 Submodle 的方式引入，例如：

  `git submodule add https://github.com/test.git`
  然后可以使用 git 把远程仓库的代码添加到本地之后，就可以像本地目录一样使用了。

- 在 CMAKE 3.14 之后，可以直接使用 FetchContent_Declare(MyName) 和 FetchContent_MakeAvailable 来导入一个 git 上的库。比原有 Cmake 中的其他的实现简单很多。导入包之后，就可以像其他 target 一样使用了。

  如下例所示：

  ```
  include(FetchContent)
  # FetchContent_MakeAvailable was not added until CMake 3.14
  if(${CMAKE_VERSION} VERSION_LESS 3.14)
      include(add_FetchContent_MakeAvailable.cmake)
  endif()

  set(SPDLOG_GIT_TAG  v1.4.1)  # 指定版本
  set(SPDLOG_GIT_URL  https://github.com/gabime/spdlog.git)  # 指定git仓库地址

  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY    ${SPDLOG_GIT_URL}
    GIT_TAG           ${SPDLOG_GIT_TAG}
  )

  FetchContent_MakeAvailable(spdlog)
  ```

### 交叉编译

交叉编译表示编译构建所在的操作系统与运行时的操作系统不同的情况，为了实现这个目标需要做到两点：

- Cmake 不能自己去检测目标系统
- 编译系统也不能用原生系统的头文件和库。

cmake 不能检测目标系统和编译器，需要设置变量的方式来告诉 Cmake, 目前 cmake 与目标系统相关的变量如下:

- CMAKE_HOST_SYSTEM_NAME
- CMAKE_HOST_SYSTEM_VERSION
- CMAKE_HOST_SYSTEM_PROCESSOR
- CMAKE_HOST_SYSTEM
- CMAKE_C_COMPILER

在进行交叉编译时，可以定义一个目标系统配置的 cmake 文件，如下所示。

```
# this one is important

SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler

SET(CMAKE_C_COMPILER /opt/eldk-2007-01-19/usr/bin/ppc_74xx-gcc)
SET(CMAKE_CXX_COMPILER /opt/eldk-2007-01-19/usr/bin/ppc_74xx-g++)

# where is the target environment

SET(CMAKE_FIND_ROOT_PATH /opt/eldk-2007-01-19/ppc_74xx /home/alex/eldk-ppc74xx-inst)

# search for programs in the build host directories

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

```

然后再执行的时候， 通过显示的指定工具链的配置文件生成目标系统上的可执行程序，如下所示：

```

~/src$ cd build
~/src/build$ cmake -DCMAKE_TOOLCHAIN_FILE=~/Toolchain-eldk-ppc74xx.cmake ..
```

### cmake 自带的 module

cmake 原生就提供了很多扩展 Module, 提供了很多有价值的方法。
例如可以用于平台检查功能

检查使用存在指定头文件:

- Module: INCLUDE(CheckIncludeFiles)

- Usage: CHECK_INCLUDE_FILES(headers variable)

- Example: CHECK_INCLUDE_FILES(strings.h HAVE_STRINGS_H)

检查方法是否存在：

- Module: INCLUDE(CheckFunctionExists)

- Usage: CHECK_FUNCTION_EXISTS(function variable)

- Example: CHECK_FUNCTION_EXISTS(madvise HAVE_MADVISE)

还提供了很多其他的检测：CheckSymbolExists， CheckLibraryExists，CheckTypeSize，CheckCXXSourceCompilesd。

cmake 还提供了很多系统和编译器的检查。

Cmake 本身也是一门编程语言，也可以封装实现一些功能方法来提供一些更加友好的功能 API，当然也可以引入一些第三方已经开发好的 cmake Module 来使用。
例如：bazel.cmake 库就是一个非常好用的库，导入之后可以在 cmake 中想类似 bazel 的方式来定义 target，减少了很多重复的定义。

例如：

```
project(testcase VERSION 0.1.0)
include(bazel)

cc_library(cpu_id SRCS cpu_id.cc)
cc_test(cpu_id_test SRCS cpu_id_test.cc DEPS cpu_id glog)
cc_test(hello SRCS hello.cc)
```

### 参考资料

- [cmake-doc](https://cmake.org/cmake/help/v3.17/index.html)
- [awesome-cmake](https://github.com/onqtam/awesome-cmake)
- [cmake examples](https://github.com/ccock/cmake-examples)
- [cmake modules](https://github.com/rpavlik/cmake-modules)
- [Simplifying build in C++](https://mropert.github.io/2017/10/19/simplifying_build-part1/)
