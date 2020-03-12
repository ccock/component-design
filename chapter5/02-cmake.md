## Modern Cmake 最佳实践

---

Cmake 是一个开源的跨平台自动化建构系统，是目前最主流的 C/C++语言构建工具。Cmake3.0 之后引入很多新的特性，有效提升了编写构建脚本的效率，称为 Modern Cmake。本文对 Modern cmake 的新特性和使用方法进行简单介绍，希望对大家有帮助。

### Target 概念

---

Cmake 2.0 主要是基于 directory 的来构建，很多的复用则只能靠变量实现。Modern Cmake 最大的改进是引入了 target， 它作为一个整体很方便的被组合和复用来构建整个系统。

    推荐 1： 在 Modern Cmake 中强烈推荐抛弃旧的 directory 方式，使用 target 的方式构建整个工程。

#### 1. 分类

Target 中最核心的两个分类是：executable, library。

- 其中 executable 是可执行程序，在不同的操作系统会有不同的格式，同样一个工程内也可能需要生成多个可执行程序。 具体指令如下所示：
  ```
  add_executable(<name> [WIN32] [MACOSX_BUNDLE]
                [EXCLUDE_FROM_ALL]
                [source1] [source2 ...])
  ```
- library 代表链接库，可以分为 share, static, object, module, interface 五个种类。

  - share 表示共享库，在编译构建过程中，需要进行链接但不会添加到最后的可执行文件中。共享库在程序运行中可以被动态加载和替换，当被多个程序使用时可以在内存中被共享。如果期望组件可以被独立的部署和替换的话，需要选择这种方式。

  - static 表示静态库，会在编译过程中被一起添加生成到可执行文件中。当静态库的实现发生变更，必须要重新编译整个系统才可以使用。使用静态库的一个好处是，生成的可执行程序在运行时可以独立的运行，不再需要依赖这个静态库。

  - module 也是共享库的一种，但是不会导出任何符号，不能被编译时链接，只能通过 dlopen 在运行时动态加载使用。

  - object 类型的库表示一组编译后的文件，并不会执行打包和链接。使用 object 类型的库可以避免一些大的源文件被重复的编译，提升编译效率。

  - interface 类型的并不会编译输出文件，代表一组接口文件，可以在编译构建中被其他 target 使用。使用 interface 类型的库可以把多个模块公共的接口头文件作为一个单独 target 来被引用。

定义库具体指令如下：

```
add_library(<name> [STATIC | SHARED | MODULE |OBJECT |INTERFACE] ...)
```

#### 2. target requirement

正常情况下编译一个 target(可执行程序或者库)需要依赖下面内容：

- 源文件列表，通过 target_sources 配置。
- 头文件列表，通过 target_include_directories
- 预编译宏，通过 target_compile_definition 配置
- 编译选项和特性，通过 target_compile_options，target_compile_features 配置。
- 链接选项，通过 target_link_options 配置

在 C/C++软件系统中，一个组件库中大部分的头文件是仅在模块内使用，仅有小一部分接口头文件是外部使用，称为对外接口。为了提高编译速度，可以在链接不同组件时仅使用组件仅谁使用外部接口即可。

为了更好的支持这个特性，Modern Cmake 针对 target 引入两个概念：user requriement(用户依赖) 和 build requirement（编译依赖）。其中用户依赖表示组件使用方需要的依赖的文件，而编译依赖表示当前组件编译构建是需要的依赖的文件。

Modern Cmake 增加了三个关键字 INTERFACE、PUBLIC、PRIVATE 分布表示不同作用域， 下面以添加头文件依赖命令为例说明：

```
target_include_directories(<target> [SYSTEM] [BEFORE]
  <INTERFACE|PUBLIC|PRIVATE> [items1...]
  [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
```

给组件添加头文件依赖路径时：

- **INTERFACE** ： 表示添加的头文件路径仅组件的使用方需要，编译当前组件并不需要。
- **PRIVATE** ： 表示添加的头文件路径仅当前组件编译时使用，其他组件不需要。
- **PUBLIC** ： 表示当前组件编译时和链接该组件都需要使用。


    推荐 2： 在 Modern Cmake 中强烈建议所有添加依赖接口时，从组件的使用角度考虑写明是INTERFACE， PRIVATE, PUBLIC。

    推荐 3： 在 Modern Cmake 中推荐使用target_sources来添加源文件，保持每个接口的职责单一。

#### 3. target requirement propagtion

当构建工程中 包含比较多的 libary 时，编译和管理这些 Libary 之间的依赖就变得尤为重要。在 Modern Cmake 中，当给 Libary 定义用户依赖和编译依赖后，在 target_link_libraries 中定义与其他组件间的依赖关系, 就可以自动传递和推演 target 之间的所有编译依赖。

组件间的依赖关系定义命令如下：

```
target_link_libraries(<target>
                      <PRIVATE|PUBLIC|INTERFACE> <item>...
                     [<PRIVATE|PUBLIC|INTERFACE> <item>...]...)
```

- **PRIVATE**： 被依赖 libary 的 user requirement 的会变成当前 target 的 build requirement
- **PUBLIC**：被依赖 libary 的 user requirement 的会变成当前 target 的 build requirement 和 user requirement.
- **INTERFACE**：被依赖 libary 的 user requirement 的会变成当前 target 的 user requirement


    推荐 4： 充分利用 Modern Cmake 强大的依赖传递功能，合理设计每个组件的依赖与组件间的依赖关系。

### package 特性。

package 用来代表一个依赖库，包含一些头文件、动态库、静态库等，并携带了版本信息。 在 cmake 中可以使用 find_package 来导入一个特定版本的第三方库。

例如： find_package(Qt5Gui 5.1.0 CONFIG) ，导入包 Qt5Gui，版本号 5.1.0，通用的导入包的命令如下：

```
find_package(<PackageName> [version][exact] [QUIET][module]
[REQUIRED][COMPONENTS][components...]]
[OPTIONAL_COMPONENTS components...][no_policy_scope])
```

不同包管理器使用第三方包实现存在很大差异，例如 JAVA 中 marven 会下载包到本地公共仓库地址中，构建时直接从本地仓库中选择合适的包进行构建。而很多动态语言例如 ruby, nodejs 等则直接把依赖的源码包下载到本地的工程的一个单独目录中就使用。

受制于语言的特有复杂性，目前 Modern Cmake 目前无法做到像其他语言包管理器灵活的使用方式，但是 Cmake 也在不断完善使用第三方库的能力。Modern Cmake 目前提供两种方式使用第三方库，分别是 find_package, find_content。

find_package 用于查找本地安装的第三方包。Modern Cmake 可以在用户级包仓库和系统级包仓库中寻找到已经安装的包。

- 用户级包仓库地址

  windows 的注册表 HKEY_CURRENT_USER 下：
  `HKEY_CURRENT_USER\Software\Kitware\CMake\Packages\<PackageName>`

  linux:
  `~/.cmake/packages/<PackageName>`

- 系统级包仓库地址

  windows 的注册表 HKEY_LOCAL_MACHINE 下：
  `HKEY_LOCAL_MACHINE\Software\Kitware\CMake\Packages\<PackageName>`
  在非 windows 系统中没有系统级包仓库地址。

使用 find_package 包括两种方式：config-file package 和 module package。其中 config-file package, 表示组件库使用 package cmake 构建，可以直接拿来使用。而 Module package 的表示使用的库没有使用 cmake 构建, 需要下游的使用者自己编写 cmake 文件。

cmake 原生就提供了 moudle 可以用于制作安装包。在 CMakePackageConfigHelpers 中封装了方法来构造 ConfigVersion.cmake 文件，在文件中设置了包的相关信息。

    推荐 5： 对外发布共享包，在Modern Cmake 中推荐提供package的cmake文件，支持直接命令安装。

[Modern Cmake Package 使用参考手册](<https://cmake.org/cmake/help/latest/manual/cmake-packages.7.html#manual:cmake-packages(7)>)

使用 find_package 仅可以使用安装到本地的仓库，但很多时候还需要使用远程服务器上的仓库，可以有下面几种做法：

- 借助 git 的功能，例如：

  `git submodule add https://github.com/test.git`
  然后可以使用 git 把远程仓库的代码添加到本地之后，就可以像本地目录一样使用了。

- 在 CMAKE 3.14 之后，可以直接使用 FetchContent_Declare(libName) 和 FetchContent_MakeAvailable(libName) 来导入一个 git 上的库，导入包之后，就可以像其他 target 一样使用了。

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

在 Modern cmake 中，借助 FetchContent 可以直接使用远程 git 库中的组件，借助 find_package 来使用安装到本地的组件库，可以帮助工程比较有效的管理和使用第三方组件库。

如果 Cmake 使用组件方面的功能不能满足你需求，可以考虑结合 concan 与 cmake 一起使用。其中 concan 是业界目前 C/C++功能最完善的包管理器，[具体请参考](https://docs.conan.io/en/latest/)

### 交叉编译

交叉编译表示编译构建所在的操作系统与运行时的操作系统不同，为了实现这个目标需要做到两点：

- Cmake 不能自己去检测目标系统
- 编译系统也不能用原生系统的头文件和库。

cmake 不能检测目标系统和编译器，需要设置变量的方式来告诉 Cmake, 目前 cmake 与目标系统相关的部分变量如下:

- CMAKE_HOST_SYSTEM_NAME
- CMAKE_HOST_SYSTEM_VERSION
- CMAKE_HOST_SYSTEM_PROCESSOR
- CMAKE_HOST_SYSTEM
- CMAKE_C_COMPILER

在进行交叉编译时，可以定义一个目标系统配置的 cmake 配置文件，如下所示。

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

然后再执行 CMake 时， 通过显示的指定工具链的配置文件生成目标系统上的可执行程序，如下所示：

```
~/src$ cd build
~/src/build$ cmake -DCMAKE_TOOLCHAIN_FILE=~/Toolchain-eldk-ppc74xx.cmake ..
```

[交叉编译介绍官方文档](https://cmake.org/cmake/help/latest/manual/cmake-toolchains.7.html)

### cmake 自带的 module

cmake 原生就提供了很多扩展 Module, 提供了很多有价值的方法。

#### 1.用于平台检查功能

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

#### 2. 直接生成 RPM 包

Cmake 中提供了 module 用于直接生成 rpm 包，需要设置一系列的变量，下面是一个最简单的 rmp 包的 Cmake 脚本如下：

```project (my_project)
cmake_minimum_required (VERSION 2.8)

set(VERSION "1.0.1")
<----snip your usual build instructions snip--->
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "my_project")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "John Explainer")
set(CPACK_PACKAGE_VENDOR "My Company")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)
```

[cmake 制作 rpm 包参考](https://cmake.org/cmake/help/v3.14/cpack_gen/rpm.html)

#### 3. 外部开源 cmake module

Cmake 本身也是一门编程语言，也可以封装实现一些功能方法来提供一些更加友好的功能 API，当然也可以引入一些第三方已经开发好的 cmake Module 来使用。
例如：bazel.cmake 库就是一个非常好用的库，导入之后可以在 cmake 中想类似 bazel 的方式来定义 target，减少了很多重复的定义。

例如：

````

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
```
````
