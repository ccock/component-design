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

## reference

- [Controlling the Exported Symbols of Shared Libraries](https://www.gnu.org/software/gnulib/manual/html_node/Exported-Symbols-of-Shared-Libraries.html)
- [Why is the new C++ visibility support so useful?](https://gcc.gnu.org/wiki/Visibility)
- [Control over symbol exports in GCC](http://anadoxin.org/blog/control-over-symbol-exports-in-gcc.html)
- [Introduction to symbol visibility](https://developer.ibm.com/technologies/systems/articles/au-aix-symbol-visibility/)
- [关于__declspec(dllimport)的理解](https://blog.csdn.net/sinat_22991367/article/details/73695039)