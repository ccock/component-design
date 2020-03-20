## Modern CMake 最佳实践

---

CMake 是一个开源的跨平台自动化建构系统，是目前最主流的 C/C++语言构建工具。CMake3.0 之后引入很多新的特性，有效提升了编写构建脚本的效率，称为 Modern CMake。本文总结了在 Modern CMake 使用中的一些最佳实践，供大家参考。

### Target 概念

---

旧版 CMake 2.0 主要是基于 directory 来构建，很多复用只能靠变量实现。Modern CMake 最大的改进是引入了 target，支持了对构建的闭包性和传播性的控制
，从而实现了构建可以模块化。

**推荐 1： 在 Modern CMake 中强烈推荐抛弃旧的 directory 方式，使用 target 的方式构建整个工程。**

#### 1. tagert 分类

Target 中最核心的两个分类是：executable, library。

- 其中 executable 是可执行程序，在不同的操作系统会有不同的格式，同样一个工程内也可能需要生成多个可执行程序。 具体指令如下所示：
  ```
  add_executable(<name> [WIN32] [MACOSX_BUNDLE]
                [EXCLUDE_FROM_ALL]
                [source1] [source2 ...])
  ```
- library 代表链接库，可以分为 share, static, object, module, interface 五个种类。

  - share 表示共享库，在编译构建过程中，需要链接但不会添加到最后的可执行文件中。共享库在程序运行中可以被动态加载和替换，当被多个程序使用时还可以在内存中被共享。如果期望 library 可以被独立的部署和替换的话，需要选择这种方式。

  - static 表示静态库，会在编译过程中被一起添加生成到可执行文件中。当静态库的实现发生变更时，必须要重新编译整个系统才可以使用。使用静态库的一个好处是，生成的可执行程序可以独立的运行，不再需要依赖这个静态库。

  - module 也是共享库的一种，CMake 中限制了 moudle 类型的 libray 不能被编译时链接，只能通过 dlopen 在运行时动态加载使用。

  - object 类型的库表示一组编译后的文件，并不会打包和链接。使用 object 类型的库可以避免一些大的源文件被重复的编译，提升编译效率。

  - interface 类型的并不会编译输出文件，代表一组接口文件，可以在编译构建中被其他 target 使用。使用 interface 类型的库可以把多个模块公共的接口头文件作为一个单独 target 来被引用，构建更加高效。

定义库具体指令如下：

```
add_library(<name> [STATIC | SHARED | MODULE |OBJECT |INTERFACE] ...)
```

#### 2. target 闭包性

为了实现 target 闭包性，Modern CMake 实现 target 与 构建和使用中所有依赖建立绑定关系，从而可以拿来即用。正常情况下编译一个 target(可执行程序或者库)需要依赖如下所示：

- 源文件列表，通过 target_sources 配置。
- 头文件列表，通过 target_include_directories 配置。
- 预编译宏，通过 target_compile_definition 配置。
- 编译选项和特性，通过 target_compile_options，target_compile_features 配置。
- 链接选项，通过 target_link_options 配置。

在 C/C++软件系统中，一个 target 中大部分的头文件是仅在模块内使用，为内部接口，仅有小一部分接口头文件是外部使用，称为对外接口。在软件设计过程中，要从高内聚低耦合的角度出发，去严格设计每个 target 的外部接口和内部接口。同样构建过程中，在链接不同 target 时也需要明确指明依赖的外部接口文件，从而提高编译构建的效率。

为了更好支持这个特性，Modern CMake 针对 target 引入两个概念：user requriement(用户依赖) 和 build requirement（编译依赖）。用户依赖表示 target 使用方需要的依赖，而编译依赖表示当前 target 编译构建时需要依赖。

Modern CMake 增加了三个关键字 INTERFACE、PUBLIC、PRIVATE 分布表示不同作用域， 下面以添加头文件依赖命令为例说明：

```
target_include_directories(<target> [SYSTEM] [BEFORE]
  <INTERFACE|PUBLIC|PRIVATE> [items1...]
  [<INTERFACE|PUBLIC|PRIVATE> [items2...] ...])
```

给 target 添加头文件依赖路径时：

- **INTERFACE** ： 表示添加的头文件路径仅 target 的使用方需要，编译当前 target 并不需要。
- **PRIVATE** ： 表示添加的头文件路径仅当前 target 编译时使用，其他 target 不需要。
- **PUBLIC** ： 表示编译时和链接该 target 都需要使用。

**推荐 2： 在 Modern CMake 中强烈建议为 target 添加依赖接口时，从使用者角度考虑写明 INTERFACE， PRIVATE, PUBLIC。**

**推荐 3： 在 Modern CMake 中推荐使用 target_sources 来添加源文件依赖，保持每个接口的职责单一。**

#### 3. target 传播性

当构建工程中 包含比较多的 libary 时，编译和管理这些 Libary 之间的依赖就变得尤为重要。在 Modern CMake 中，当给 Libary 定义用户依赖和编译依赖后，通过在 target_link_libraries 中定义与其他组件间的依赖关系, 就可以自动传递和推演 target 之间的所有编译依赖。

组件间的依赖关系定义命令如下：

```
target_link_libraries(<target>
                      <PRIVATE|PUBLIC|INTERFACE> <item>...
                     [<PRIVATE|PUBLIC|INTERFACE> <item>...]...)
```

- **PRIVATE**： 被依赖 libary 的 user requirement 的会变成当前 target 的 build requirement
- **PUBLIC**：被依赖 libary 的 user requirement 的会变成当前 target 的 build requirement 和 user requirement.
- **INTERFACE**：被依赖 libary 的 user requirement 的会变成当前 target 的 user requirement

**推荐 4： 充分利用 Modern CMake 强大的依赖传递功能，合理设计每个 target 间的依赖关系。**

### package 能力

---

package 代表携带版本信息的 target，用于方便的导入第三方库。当系统最终发布的 target 比较大时，通过功能拆解为更多小粒度的 target，然后使用 package 机制组合实现所有功能。通过这种策略，从而可以实现系统更小粒度功能的单独构建与发布很有价值。在 CMake 中可以使用 find_package 来导入一个特定版本的第三方库。

例如： find_package(Qt5Gui 5.1.0 CONFIG) ，导入包 Qt5Gui，版本号 5.1.0，通用的导入包的命令如下：

```
find_package(<PackageName> [version][exact] [QUIET][module]
[REQUIRED][COMPONENTS][components...]]
[OPTIONAL_COMPONENTS components...][no_policy_scope])
```

不同语言的包管理器，在管理第三方包中实现存在很大差异。例如 JAVA 中 marven 会下载包到本地公共仓库地址中，构建时直接从本地仓库中选择合适的包进行构建。而很多动态语言例如 ruby, nodejs 等则直接把依赖的源码包下载到本地工程的一个单独目录中使用。

受制于 C/C++语言的特有复杂性，目前 Modern CMake 目前无法做到像其他语言包管理器灵活的使用方式，但 CMake 也在不断完善使用第三方库的能力。Modern CMake 目前提供两种方式使用第三方库，分别是 find_package, find_content。

find_package 用于查找本地安装的第三方包。Modern CMake 可以在用户级包仓库和系统级包仓库中寻找到已经安装的包。

- 用户级包仓库地址

  windows 的注册表 HKEY_CURRENT_USER 下：
  `HKEY_CURRENT_USER\Software\Kitware\CMake\Packages\<PackageName>`

  linux:
  `~/.CMake/packages/<PackageName>`

- 系统级包仓库地址

  windows 的注册表 HKEY_LOCAL_MACHINE 下：
  `HKEY_LOCAL_MACHINE\Software\Kitware\CMake\Packages\<PackageName>`
  在非 windows 系统中没有系统级包仓库地址。

使用 find_package 包括两种方式：config-file package 和 module package。其中 config-file package 表示 target 使用 package CMake 构建，可以直接拿来使用。而 Module package 的表示使用的 target 没有使用 CMake 构建，需要下游的使用者编写 CMake 文件。

Modern CMake 中提供了制作安装包的脚本。在 cmake 文件中加入 include(CMakePackageConfigHelpers)，就可以使用封装方法来生成 ConfigVersion.CMake 文件，其中已经自动设置好了包的相关信息。

更多关于 Package 使用介绍请参考：[Modern CMake Package 使用手册](<https://CMake.org/CMake/help/latest/manual/CMake-packages.7.html#manual:CMake-packages(7)>)

**推荐 5： Modern CMake 中 推荐使用 config-file package 的方式将 target 发布成 package，利用 package 机制将对依赖库的使用标准化。**

使用 find_package 仅可以使用安装到本地的仓库，但很多时候还需要使用远程仓库上的 库，可以有下面几种做法：

- 借助 git 的功能，例如：

  `git submodule add https://github.com/test.git`
  可以使用 git 把远程仓库的代码更新到本地目录之后，就可以像本地目录一样使用库了。

- 在 CMake 3.14 之后，可以直接使用 FetchContent_Declare(libName) 和 FetchContent_MakeAvailable(libName) 来导入一个 git 上的库后，就可以像其他 target 一样使用了。

  如下例所示：

  ```
  include(FetchContent)
  # FetchContent_MakeAvailable was not added until CMake 3.14
  if(${CMake_VERSION} VERSION_LESS 3.14)
      include(add_FetchContent_MakeAvailable.CMake)
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

在 Modern CMake 中，借助 FetchContent 可以直接使用远程 git 库中的组件，然后借助 find_package 来使用安装到本地的组件库，从而实现比较高效管理和使用第三方库。

如果 CMake 管理使用 target 方面功能还不能满足需求，可以考虑结合 concan 与 CMake 一起使用。concan 是业界目前 C/C++功能最完善的包管理器，具体请参考：[concan 官网介绍](https://docs.conan.io/en/latest/)

### CMake Module

---

Module 在很多场景下有不同的解释，这里的 CMake module 代表可以被导入的 cmake 源码文件。在 CMake 中使用 include(module)之后，就可以使用 module 中定义函数方法了。CMake 内置的 Module, 提供了很多有价值的功能方法，可以直接导入使用。

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

CMake 还提供了很多系统和编译器的检查，这些特性在构建支持跨平台应用时发挥非常大的作用。

#### 2. 直接生成 RPM 包

CMake 中提供了 module 用于直接生成 rpm 包，需要设置一系列的变量，下面是一个最简单的 rmp 包的 CMake 脚本如下：

```project (my_project)
CMake_minimum_required (VERSION 2.8)

set(VERSION "1.0.1")
<----snip your usual build instructions snip--->
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "my_project")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "John Explainer")
set(CPACK_PACKAGE_VENDOR "My Company")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMake_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMake_SYSTEM_PROCESSOR}")
include(CPack)
```

具体请参考： [CMake rpm 生成手册 ](https://CMake.org/CMake/help/v3.14/cpack_gen/rpm.html)

#### 3. 外部开源 CMake module

CMake 本身也是一门编程语言，也可以封装实现一些功能方法来提供一些更加友好的功能 API，当然也可以引入一些第三方 CMake Module 来使用。

Bazel.CMake 库就是一个非常好用的库，导入之后可以在 CMake 中想类似 Bazel 的方式来定义 target，减少了很多重复的定义。例如：

```
project(testcase VERSION 0.1.0)
include(bazel)

cc_library(cpu_id SRCS cpu_id.cc)
cc_test(cpu_id_test SRCS cpu_id_test.cc DEPS cpu_id glog)
cc_test(hello SRCS hello.cc)
```

Bazel 是一个支持多语言、跨平台的高效构建工具，对 C++的支持非常友好，是目前 Google 主推的构建工具，具体请参考好友「刘光聪」的系列文章：[Bazel build 介绍](https://www.jianshu.com/p/ab5ef02bfa2c)

**推荐 6：推荐复用 CMake 内置的 module 与第三方开源 module 中的功能实现，避免重复去造轮子。**

### 扩展补充

---

#### 1. 交叉编译

交叉编译表示编译构建所在操作系统与运行时操作系统不同，为了实现这个目标需要做到两点：

- CMake 不能自己去检测目标系统
- 编译系统也不能用原生系统的头文件和库。

当 CMake 不能检测目标系统和编译器，需要设置变量的方式来告诉 CMake, 目前 CMake 与目标系统相关的部分变量如下:

- CMake_HOST_SYSTEM_NAME
- CMake_HOST_SYSTEM_VERSION
- CMake_HOST_SYSTEM_PROCESSOR
- CMake_HOST_SYSTEM
- CMake_C_COMPILER

在进行交叉编译时，可以定义一个目标系统配置的 CMake 文件，如下所示。

```
# this one is important

SET(CMake_SYSTEM_NAME Linux)
#this one not so much
SET(CMake_SYSTEM_VERSION 1)

# specify the cross compiler

SET(CMake_C_COMPILER /opt/eldk-2007-01-19/usr/bin/ppc_74xx-gcc)
SET(CMake_CXX_COMPILER /opt/eldk-2007-01-19/usr/bin/ppc_74xx-g++)

# where is the target environment

SET(CMake_FIND_ROOT_PATH /opt/eldk-2007-01-19/ppc_74xx /home/alex/eldk-ppc74xx-inst)

# search for programs in the build host directories

SET(CMake_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories

SET(CMake_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMake_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

```

然后在执行 CMake build 时， 通过显示的指定工具链的配置文件来生成目标系统上的可执行程序，如下所示：

```
~/src$ cd build
~/src/build$ CMake -DCMake_TOOLCHAIN_FILE=~/Toolchain-eldk-ppc74xx.CMake ..
```

具体请参考：[交叉编译介绍官方文档](https://CMake.org/CMake/help/latest/manual/CMake-toolchains.7.html)

#### 2. Generator expressions

CMake 本质上是一个构建工程生成器，Generator expression 是在 build 过程中执行的表达式，从而实现根据不同配置生成不同的构建工程。

现代 IDE 很多都支持 Multi-configuration，例如 debug, release 等，在 Modern CMake 中，可以通过 generator-expression 来更好的支持这个特性。

Generator expression 可以在 target_link_libraries(), target_include_directories(), target_compile_definitions()中使用，从而可以实现根据条件来添加链接依赖，头文件路径依赖或者宏定义等。

generator-expression 定义为\$<...>的形式，该表达式实现有多种形式，并且支持嵌套使用，下面以 debug，release 的链接配置示例来简单说明：

```
target_link_directories(${PROJECT_NAME} PUBLIC
  $<$<CONFIG:Debug>:${LIB_DIRS_DEBUG}>
  $<$<CONFIG:Release>:${LIB_DIRS_RELEASE}>)
```

具体请参考：[编译生成式官方介绍](https://cmake.org/cmake/help/v3.0/manual/cmake-generator-expressions.7.html)

### 总结

---

Modern CMake 3.0 功能和特性与 CMake2.0 上有很大的变化，而且新版本还在不断完善中。希望每位开发者可以拥抱变化，优先选用 Modern CMake 来构建系统，并在编写构建脚本过程中可以参考下面的一些实践总结。

- 在 Modern CMake 中强烈推荐抛弃旧的 directory 方式，使用 target 的方式构建整个工程。
- 在 Modern CMake 中强烈建议为 target 添加依赖接口时，从使用者角度考虑写明 INTERFACE， PRIVATE, PUBLIC。
- 在 Modern CMake 中推荐使用 target_sources 来添加源文件依赖，保持每个接口的职责单一。
- 充分利用 Modern CMake 强大的依赖传递功能，合理设计每个 target 间的依赖关系。
- 推荐使用 config-file package 的方式将 target 发布成 package，利用 package 机制将对依赖库的使用标准化。
- 推荐复用 CMake 内置的 module 与第三方开源 module 中的功能实现，避免重复去造轮子。

### 参考资料

- [CMake-doc](https://CMake.org/CMake/help/v3.17/index.html)
- [awesome-CMake](https://github.com/onqtam/awesome-CMake)
- [CMake examples](https://github.com/ccock/CMake-examples)
- [CMake modules](https://github.com/rpavlik/CMake-modules)
- [Simplifying build in C++](https://mropert.github.io/2017/10/19/simplifying_build-part1/)
