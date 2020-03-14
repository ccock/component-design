# API管理

## cmake Module

cmake中将库生成为Module方式的话：

```cmake
add_library(value MODULE value.c)
add_executable(main main.c)
target_link_libraries(main value)
```

上面的CMakeLists.txt中最后一行会提醒value库是一个module，不允许被链接。只能链接`STATIC`和`SHARED`类型的库。

在OSX下，可以发现，当设置`add_library(value SHARED value.c)`的话，将会生成`libvalue.dylib`；而改为` add_library(value MODULE value.c)`的时候，生成的是`libvalue.so`。

可以手动指定链接这个so，例如`target_link_libraries(main "${CMAKE_CURRENT_BINARY_DIR}/libvalue.so")`，OSX下会提醒`libvalue.so`是一个bundle，只可以dlopen。而在linux下则可以直接链接。

由此可见，MODULE不允许链接是CMAKE的限制，它利用了不同的平台的限制来完成这个约束。但是在linux由于是部分bundle和so的，所以绕过CMake只要产生的so都是可以链接的。

## control the visibility of API

如下文件，编译指定`-fvisibility=hidden`，则生成的动态库符号中全局变量和函数都是隐藏的。

```c
int g_value = 5;

int getValue(){
    return g_value;
}

void setValue(int v) {
    g_value = v;
}
```

```cmake
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
add_library(value SHARED value.c)
```

```sh
nm libvalue.so

0000000000201020 d g_value
00000000000005da t getValue
00000000000005e9 t setValue
```

这时所有符号，无论是编译时链接，还是动态dlopen，都是无法找到符号的。

而显示指定导出符号：

```c
#define PUB __attribute__ ((visibility ("default")))

PUB int g_value = 5;

PUB int getValue(){
    return g_value;
}

PUB void setValue(int v) {
    g_value = v;
}
```

```sh
nm libvalue.so

0000000000201020 D g_value
00000000000005da T getValue
00000000000005e9 T setValue
```

这时符号无论是编译时链接还是动态dlopen则都是可以找到的。

## symbol table for LD

还有一种情况，假设动态库libcode.so链接了一个静态库libutil.a。静态库libutil.a中的符号默认在动态库libcode.so中是导出的。

为了隐藏静态库中的符号，我们可以给静态库中需要导出的符号增加`__attribute__ ((visibility ("default")))`，同时给静态库的构建加上`-fvisibility=hidden`。

遗憾的是，某些情况下，我们是没法修改静态库的源码的（静态库是第三方提供的），这时我们可以采用显示为链接器指定符号表的方式，选择性导出符号。

创建一个libcode.version文件：

```
CODEABI_1.0 {
    global: *entry_point*;
    local: *;
};
```

然后如下构建libcode.so：

```sh
$ g++ code.cpp libutil.a -o shared -fPIC -Wl,--version-script=libcode.version
```

## do not miss symbols in dynamic library

```c
//value.c

extern int g_value;

int getValue(){
    return g_value;
}

void setValue(int v) {
    g_value = v;
}
```

```cmake
add_library(value SHARED value.c)
```

OSX系统下，编译期报错：

```
Undefined symbols for architecture x86_64:
  "_g_value", referenced from:
      _getValue in value.c.o
      _setValue in value.c.o
ld: symbol(s) not found for architecture x86_64
clang: error: linker command failed with exit code 1 (use -v to see invocation)
make[2]: *** [libvalue.dylib] Error 1
make[1]: *** [CMakeFiles/value.dir/all] Error 2
make: *** [all] Error 2
```

而在linux系统下，则是可以正常构建出libvalue.so的。

但是在linux下会在将该so连接到可执行程序的时候报错：

```c
//main.c

extern int getValue();
extern void setValue(int v);

int main(int argc, char **argv) {  
    printf("get_value = %d\n", getValue());
    setValue(6);
    printf("g_value = %d\n", getValue());
}
```

```cmake
add_library(value SHARED value.c)
target_link_libraries(main value)
```

上面会在链接期生成main的时候报错：

```
[ 50%] Built target value
Scanning dependencies of target main
[ 75%] Building C object CMakeFiles/main.dir/main.c.o
[100%] Linking C executable main
libvalue.so: undefined reference to `g_value'
collect2: error: ld returned 1 exit status
CMakeFiles/main.dir/build.make:95: recipe for target 'main' failed
make[2]: *** [main] Error 1
CMakeFiles/Makefile2:104: recipe for target 'CMakeFiles/main.dir/all' failed
make[1]: *** [CMakeFiles/main.dir/all] Error 2
Makefile:83: recipe for target 'all' failed
make: *** [all] Error 2
```

如果将main中对value中的符号使用改为dlopen，则会在运行期出错：

```c
// main.c

#include <stdio.h>  
#include <dlfcn.h>  
    
int main(int argc, char **argv) {  
    void *handle;  
    int (*getV)();
    void (*setV)(int);
    char *error;  
    
    handle = dlopen ("./libvalue.so", RTLD_LAZY);  
    if (!handle) {  
        fprintf (stderr, "%s ", dlerror());  
        exit(1);  
    }  
    
    getV = dlsym(handle, "getValue");  
    if ((error = dlerror()) != NULL)  {  
        fprintf (stderr, "%s ", error);  
        exit(1);  
    }  
        
    setV = dlsym(handle, "setValue");  
    if ((error = dlerror()) != NULL)  {  
        fprintf (stderr, "%s ", error);  
        exit(1);  
    }      
    
    printf("%d ", (*getV)());  

    setV(6);

    printf("%d ", (*getV)()); 

    printf("\n");

    dlclose(handle);  
    return 0;  
}  
```

```cmake
add_library(value SHARED value.c)
add_executable(main main.c)
target_link_libraries(main dl)
```

```
# ./main
./libvalue.so: undefined symbol: g_value root@1516c48a21ff:/code/build# 
```

因此我们得到一个结论：不要让动态库缺符号，否则无论是被静态还是动态使用，都会让使用方出错。

当然解决方案是：

- 可以让动态库生成的时候，通过静态链接把自己需要的符号包含完整：

```c
//var.c

int g_value = 5;
```

```cmake

add_library(var STATIC var.c)
add_library(value SHARED value.c)
target_link_libraries(value var)
```

这样libvalue.so中的符号就是完整的，自然静态或者动态使用是没有问题的。

但是如果把cmake改为如下，让var变成一个动态库：

```cmake

add_library(var SHARED var.c)
add_library(value SHARED value.c)
target_link_libraries(value var)
```

那么libvalue.so中仍旧是缺少`g_value`符号的，但是它记录了它依赖了libvar.so。如下：

```sh
# ldd libvalue.so 
linux-vdso.so.1 (0x00007ffce57c7000)
libvar.so => /code/build/libvar.so (0x00007f222e3b5000)
```

所以最终链接libvalue.so的时候不用指定链接libvar.so:

```cmake
add_library(var SHARED var.c)
add_library(value SHARED value.c)
target_link_libraries(value var)
add_executable(main main.c)
target_link_libraries(main value)
```

而当我们删除libvar.so后，再次运行main就会失败：

```sh
./main 
./main: error while loading shared libraries: libvar.so: cannot open shared object file: No such file or directory
```

上述行为在main使用dlopen动态加载libvalue.so时是一样的，只用动态加载libvalue.so，不用指定libvar.so。而当删除libvar.so后，再次运行main会出错，提示找不到libvar.so。

- 可以在main的链接中指定libvalue.so缺少的符号所在的库

```cmake
add_library(var STATIC var.c)
add_library(value SHARED value.c)
add_executable(main main.c)
target_link_libraries(main value var)
```

这时libvalue.so中标明缺失g_value符号，最后main的可执行里面有g_value符号。

而将var改为动态库后，main中也没有g_value符号了，而是运行时从libvar.so中获取。当删除libvar.so后，再次运行main会失败：

```sh
# rm -rf libvar.so
# ./main 
./main: error while loading shared libraries: libvar.so: cannot open shared object file: No such file or directory
```

而将这种做法，应用于dlopen打开libvalue.so的main实现的时候，是没有效果的：

```cmake
add_library(var STATIC var.c)
# add_library(var SHARED var.c)
add_library(value SHARED value.c)
add_executable(main main.c)
target_link_libraries(main dl var)
```

上例中无论是将var生成为静态库还是动态库，对于main来说都不会将var中的符号提前导入。所以main中dlopen libvalue.so的时候，会报错：

```
 ./main 
./libvalue.so: undefined symbol: g_value
```

即使是静态链接动态库，main也要自己知道libvalue.so的所有依赖，为其链接全。这样无异于把libvalue的依赖暴露给了使用者。因此我们建议对于动态库，最好是完整的，自己将自己的依赖链接全，不要缺符号。

对于静态库，一般情况下和别的静态库不会提前链接，另外静态库经常用于代码裁剪，所以不做这个约束。

## reference

- [Controlling the Exported Symbols of Shared Libraries](https://www.gnu.org/software/gnulib/manual/html_node/Exported-Symbols-of-Shared-Libraries.html)
- [Why is the new C++ visibility support so useful?](https://gcc.gnu.org/wiki/Visibility)
- [Control over symbol exports in GCC](http://anadoxin.org/blog/control-over-symbol-exports-in-gcc.html)
- [Introduction to symbol visibility](https://developer.ibm.com/technologies/systems/articles/au-aix-symbol-visibility/)
- [关于__declspec(dllimport)的理解](https://blog.csdn.net/sinat_22991367/article/details/73695039)