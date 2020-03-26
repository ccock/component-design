# C/C++符号隐藏与API管理

众所周知，解决代码耦合问题的核心原则之一是信息隐藏。即把所有客户不需要关心的信息尽力隐藏起来，只暴露出仅需要被依赖的东西。这样做不仅可以控制变化的波及范围，降低测试成本，还能方便各种代码分析工具和重构工具对代码进行更精准的依赖分析。

基于此，现代编程语言大都提供了丰富的信息隐藏手段，包括不同级别的代码符号可见性控制、显示的API导入与导出能力，以及管理软件包之间API依赖关系的工具链等等。

然而我们知道，C和C++语言由于出现年代早、历史包袱重，对符号隐藏以及API管理的特性支持演进较慢。尤其是C语言，相比其它语言更是捉襟见肘。在我咨询的大多C或C++项目中，最容易被问的问题中就包含“我要如何管理这该死的项目头文件”，但凡和代码有关的工作（无论重构、构建、测试）也都得从梳理混乱的头文件包含关系开始。

事实上，C和C++语言在实践中发展出了一套符号隐藏和API管理的机制，这套做法已经被许多优秀的开源C/C++项目广泛使用着。下面给大家介绍下这些C/C++符号隐藏与API管理的优秀实践，虽然和别的语言相比仍旧有不完美的地方，但对于绝大多数项目来说已经完全够用了。

## 基本的符号隐藏

C语言中全局变量和函数的符号默认是外部可见的，因此我们可以在一个编译单元中直接`extern`另一个编译单元中的符号，甚至是另一个代码包中的符号。由于全局可见性，链接器会在链接期帮我们跨编译单元找到对应的符号进行链接。

这种全局可见性看起来使用简单，但是却为C语言带来了很多麻烦。首先为了避免符号冲突，在大的项目中我们必须为所有全局变量和函数起很长的名字，并且需要加上“子系统名_”或者“模块名_”之类的前缀。这样不仅代码不够简洁，而且生成的二进制也会占用更多的空间。

其次，全局可见性让使用`extern`的成本很低。`extern`为外部符号依赖提供了一种直通车机制，这种做法绕过了别人提供的头文件，造成了一种间接的隐式依赖。长此以往，项目中的依赖关系最终会变成一盘意大利面条。不仅如此，`extern`会造成全局变量和函数原型的重复声明，这不仅破坏了`DRY（Don't Repeat Yourself）`原则，而且为代码引入了很多潜在的安全问题。我已经不止一次见过全局变量的维护者修改了变量类型，如将`U32 g_ports[MAX_NUM]`修改为`U16 g_ports[MAX_NUM]`但是遗漏了`extern U32 g_ports[MAX_NUM]`，然后引起了各种难以定位的内存和复位问题。

所以，我们需要遵守的第一条重要的原则是：**尽量避免使用`extern`关键字**。`extern`只在很少几种情况下是有用的，例如要hack某些第三方的没有头文件的二进制库，或者调用汇编编写的函数以及访问编译器/链接器自动生成的符号等。

尽力消灭代码中的`extern`绝对会改善你的设计，但是这不会改变C语言会将符号置为全局可见的事实。这时我们需要另一个非常重要的关键字`static`来帮忙。

`static`是C语言中仅有的用于隐藏符号的手段，因此用好它的意义十分重要。

`static`在C语言中主要有两种作用。1）对于函数内的局部变量，它指示该变量的内存不在栈上，而在全局静态区。2）对于全局变量和函数来说，它指示对应的符号可见性被约束在本编译单元内，不会暴露出去。

对于符号隐藏，我们主要使用`static`的第二个用途。由于使用`static`修饰的全局变量和函数的符号不会被导出，所以我们可以给它们更短的命令，同时编译器也会帮我们做更好的优化，生成更小的二进制。

更重要的是，尽量多的使用`static`会让我们改善设计，进而得到符合“Modular C”风格的设计。即将状态（全局变量）和无需暴露的函数通过`static`隐藏到编译单元内部，只将真正的API接口声明到头文件中。由于使用`static`修饰的符号是没法`extern`的，结合上一条，强制使用方只能显示的通过include对应的头文件来调用开放的API，这样代码自然变得更加的模块化。

所以，我们给出C语言符号隐藏另一个原则：**尽可能多的使用`static`关键字来隐藏细节，并遵从Modular C的设计风格**。

现在我们转向C++。得益于C++的面向对象特性，我们有了类以及对应的可访问性控制关键字`private`、`protected`和`public`。这些关键字可以修饰类的成员以及类的继承关系，从而对内和对外呈现出不同级别的访问性控制。这里我们对这些关键字不做更多介绍，只是推荐大家能够在代码中更好的使用这些关键字来隐藏信息，记得千万不要把类中的一切都公开出去（虽然我见过很多人确实这么做的）。记住一个原则，那就是**尽可能多的使用`private`关键字**。

除了类，C++语言还有一个用于隐藏信息极好的特性，那就是命名空间`namespace`。`namespace`让我们能够对符号分级，把符号的直接可见性控制在独立的命名空间中，而不用像C语言中那样靠增加名字前缀来避免符号冲突。遗憾的是C++中命名空间是没有可访问性控制的，也就是说命名空间中的符号全部是公开的，外部通过命名空间路径都是可以访问到的。不过C++语言提供了匿名命名空间的特性，凡是在匿名命名空间中的符号都是不导出的。也就是说匿名命名空间中的符号只在本编译单元内部可见，外部是不能使用的。其作用类似于C语言中的`static`，只是写起来更加简洁。

```cpp
// example.cpp

namespace {
    struct Port {
        // ...
    };

    Port ports[MAX_NUM];

    unsigned int getRateOf(const Port& port) {
        // ...
    }
}

unsigned int getPortRate(unsigned int portId) {
    // ...
}
```

如上面例子中：`Port`、`ports`和`getRateOf`只能在"example.cpp"中访问，而`getPortRate`则在该编译单元外也可以使用。这里对于C++语言，我们推荐**尽可能合理使用命名空间，尤其是使用匿名命名空间来隐藏符号**。

C++语言为了兼容C，仍旧使用头文件机制发布API。为了设计好C++的头文件，我们需要先区分两个概念：“可见性”与“可访问性”。

以下面这个`Storage`类定义的头文件“Storage.h”为例：

```cpp
// Storage.h

#include "StorageType.h"

class Storage {
public:
    Storage();
    unsigned int getCharge() const;
private:
    bool isValid() const;
private:
    StorageType type;
    unsigned int capacity; 
    static unsigned int totalCapacity;
};
```

用户只要包含这个头文件，就可以看到`Storage`类中的所有的方法声明以及成员变量定义。因此从可见性上来说，这个类的所有函数声明和成员变量的定义都是对外可见的。然而从可访问性上来说，我们只能访问这个类的公开的构造函数`Storage()`和`getCharge()`接口。

从上面的例子中可以看到，C++头文件中类定义对外的可见性和可访问性是不一致的。当可见性大于可访问性的时候，带来的问题是：当我们修改了类的私有函数或者成员变量定义（用户可见但是不可访问的符号）时，事实上并不会影响用户对该类的使用方式，然而所有使用该类的用户却被迫要承担重新编译的负担。

为了避免上面的问题，降低客户重新编译的负担，我们需要在头文件中尽量少的暴露信息。对类来说需要尽量让其对外的可见性和可访问性在头文件中趋于一致。

那要怎么做呢？主要有以下手段：

- 可以将类的静态私有（static private）成员直接转移到类实现文件中的匿名命名空间中定义；

如上例中的`static unsigned int totalCapacity`是不需要定义到类的头文件中的，可以直接定义到该类实现文件的匿名命名空间中。

```cpp
// Storage.cpp

#include "Storage.h"

namespace
{
    unsigned int totalCapacity = 0;
}

Storage::Storage() {
    // ...
}

bool Storage::isValid() const {
    if (this->capacity > totalCapacity) {
        // ...
    }
    // ...
}

unsigned Storage::int getCharge() const {
    if(this->isValid(this->capacity)) {
        // ...
    }
    // ...
}
```

- 对于类的非静态私有成员方法，可以将它依赖的成员变量当做参数传给它，这样它就可以变成类的静态私有函数。然后就可以依照前面的方法将其移到类实现文件中的匿名命名空间中；

如上例中类的`bool isValid() const`私有成员方法的实现中访问了类的成员变量`this->capacity`。我们修改`isValid`方法的实现，将`capacity`作为参数传递给它，这样`isValid`在类中的声明变为`static bool isValid(unsigned int capacity)`，实现变为：

```cpp
// Storage.cpp

bool Storage::isValid(unsigned int capacity) {
    if (capacity > totalCapacity) {
        // ...
    }
    // ...
}
```

现在我们就已经可以参照前面的原则，将类的私有静态成员搬移到实现文件的匿名命名空间中，将其在头文件中的声明删除。

```cpp
// Storage.h

#include "StorageType.h"

class Storage {
public:
    Storage();
    unsigned int getCharge() const;
private:
    StorageType type;
    unsigned int capacity; 
};
```

```cpp
// Storage.cpp

#include "Storage.h"

namespace
{
    unsigned int totalCapacity = 0;

    bool isValid(unsigned int capacity) {
        if (capacity > totalCapacity) {
            // ...
        }
        // ...
    }    
}

Storage::Storage() {
    // ...
}

unsigned Storage::int getCharge() const {
    if(isValid(this->capacity)) {
        // ...
    }
    // ...
}
```

经过上面的操作，类中的私有方法和静态私有成员都从头文件移到了实现文件的匿名命名空间中了。那么最后剩下的类的非静态私有成员变量能否也隐藏起来呢？

方法是有的，就是使用[PIMPL（pointer to implementation）](https://cpppatterns.com/patterns/pimpl.html)方法。

- 可以使用PIMPL方法隐藏类的私有成员。

对于上例，使用PIMPL后实现如下：

```cpp
// storage.h

class Storage {
public:
    Storage();
    unsigned int getCharge() const;
    ~Storage();
private:
    class Impl;
    Impl* p_impl{nullptr}; 
};
```

```cpp
// Storage.cpp

#include "Storage.h"
#include "StorageType.h"

namespace
{
    unsigned int totalCapacity = 0;

    bool isValid(unsigned int capacity) {
        if (capacity > 0) {
            // ...
        }
        // ...
    } 
}

class Storage::Impl {
public:
    Impl() {
        // original implmentation of Storage::Storage()
    }

    unsigned int getCharge() const {
        // original implmentation of Storage::getCharge()
    }
private:
    StorageType type;
    unsigned int capacity;     
};

Storage::Storage() : p_impl(new Impl()){
}

Storage::~Storage(){
    if(p_impl) delete p_impl;
}

unsigned int Storage::getCharge() const {
    return p_impl->getCharge();
}

```

可以看到，使用PIMPL方法就是把所有的调用委托到一个内部类（本例中的`Impl`）的指针上。由于指针的类型只用做前置声明，所以使用PIMPL手法的类的私有成员只用包含一个内部类的前置声明和一个成员指针即可。而`Impl`类则包含了原来类的所有真正的成员和函数实现。因为`Impl`类可以实现在cpp文件中，所以达到了进一步信息隐藏的效果。

从上例我们看到，由于`Storage`类的所有私有成员都转移到了内部的`Impl`类中，所以`Storage`类的头文件中不再需要包含"StorageType.h"，只用在实现文件中包含即可。因此使用PIMPL手法，可以解决头文件耦合与物理依赖传递问题。

不过，通过代码示例也可以看到使用PIMPL方法是有成本的，它增加了间接函数调用和动态内存分配的开销。而且由于代码多了一层封装，导致整体复杂度上升了。因此除非解决某些严重的物理依赖问题，一般不会大面积使用该手法的。

最后，一个完备的PIMPL实现会借助`unique_ptr`类型的智能指针。本例为了简化示例所以采用了裸指针实现，更完整和通用的PIMPL实现可以参见 [https://en.cppreference.com/w/cpp/language/pimpl](https://en.cppreference.com/w/cpp/language/pimpl)。

到此，我们总结一下C/C++语言自身有关符号可见性控制的原则和方法：

```
1) 尽量避免使用extern关键字；
2) 对于C语言，尽可能多的使用static关键字来隐藏细节，遵从Modular C的设计风格；
3）对于C++，尽可能多的使用private关键字；
4）对于C++，尽可能合理使用命名空间，尤其是使用匿名命名空间来隐藏符号；
5）头文件尽量隐藏信息，缩小头文件内的符号可见性。可以采取的手段有：
    - 将类的静态私有成员转移到实现文件的匿名命名空间中；
    - 在某些情况下，可以将类的私有方法重构成类的静态私有方法，然后移入到实现文件的匿名命名空间中；
    - 对于某些严重的头文件耦合问题，可以选择使用PIMPL方法，隐藏类的所有非公开成员及其依赖的头文件；
```

## 库级别符号隐藏

当程序规模变大之后，人们会对软件进行模块划分，以便分而治之，降低软件维护成本。有了模块之后，就可以将其构建成库（静态库或者动态库）发布给别人使用。

前文所述的符号隐藏手段对于模块内代码做信息隐藏是够的，但是对于库来说是不够的。当程序规模变大后，我们不可能把所有代码都写到一个C文件或者CPP文件中。当代码被拆分到多个实现文件中，它们之间需要互相访问就必须通过头文件暴露自己的可访问API给别人。但是当所有文件都被打包在一起编译成库再提供给第三方的时候，这些内部开放的接口却未必都需要被作为库接口暴露出去。

常见的一种做法是将库的内部头文件和外部的头文件分开，对外不发布内部头文件。这是C/C++常用的一种库级别的头文件管理手段，后面我们会专门介绍。遗憾的是，仅通过不发布私有头文件，并没有解决所有问题。

即使不发布内部头文件，内部跨编译单元可被访问的符号默认情况下仍旧会被库全部导出。这样不仅浪费了二进制的空间，增加了库之间符号冲突的概率，而且还让软件包承担了不必要的安全风险。导出的内部符号仍旧可以被外部强制extern，或者是被拿来做一些hack的事情。

现代编程语言会引入module机制来管理软件模块或者库的外部可见性问题，让开发者在发布软件的时候显示的指定需要导出给外部的API，其它的符号都只能被内部访问。但是C和C++语言由于历史包袱重（新的特性需要尽量兼容已经编译通过的代码），C++语言直到20版本才将module特性标准化，而C语言的module特性至今仍不见踪影。（事实上Java的module特性从2011年提出直到2017年才通过Java9发布，也历时七年之久）。

由于C++20标准刚刚出来不久，编译器对module机制的支持还很不完善，所以该特性离进入实用还有不少距离。该兴趣的同学可以先看看我的朋友张超写的这篇文章[《C++ Modules 初窥》](https://www.jianshu.com/p/6ddd5dc909db)。

回到现实中，在没有语言直接支持的情况下，我们如何隐藏库的内部符号，显示的指定需要导出的API呢？

方法是有的，那就是借助编译器扩展。

GCC4之后支持使用`-fvisibility=hidden`编译选项，将库的所有符号默认设置为对外不可见。这样编译出的二进制就不会导出可供外部链接的符号。然后再结合GCC的`__attribute__ ((visibility ("default")))`属性，在代码中明确指定可以暴露给外部的API，于是我们就达到了显示控制库对外API可见性的目的。

如下代码示例：

```cpp
// entry.h

void function1();
__attribute__ ((visibility ("default"))) void entry_point();
```

```cpp
// entry.cpp

#include "entry.h"

void function1() {
    // ...
}

void entry_point() {
    function1();
}
```

当我们采用`-fvisibility=hidden`将entry.cpp编译成静态库或者动态库后，无论用户是静态链接还是使用`dlopen`动态库的方式，都只能访问到`void entry_point()`函数，而不能访问到`void funcion1()`。

通过该方法，我们不仅能显示控制库的导出API，还可以帮助编译器和链接器优化出更好的二进制，并且缩短动态库的加载时间。

关于该特性更多的细节，可以参考["https://gcc.gnu.org/wiki/Visibility"](https://gcc.gnu.org/wiki/Visibility)。

Windows下也有类似的机制`__declspec(dllexport)`，和gcc下的`__attribute__ ((visibility ("default")))`作用类似。稍微不同的是Windows下还存在`__declspec(dllimport)`用于API的使用方显示导入外部API，以便编译器对代码进行优化，但gcc下没有对应的扩展。

为了让使用上述编译器扩展的代码能够跨平台，使用该特性的库往往都会把它封装成一个宏，根据代码所在的平台和编译器版本，转化成不同的实现。

```cpp
#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
      #define DLL_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
      #define DLL_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif
```

如上是"https://gcc.gnu.org/wiki/Visibility"中给出的宏定义。它根据不同的平台和编译器版本，定义了`DLL_PUBLIC`和`DLL_LOCAL`的不同实现。

```cpp
DLL_PUBLIC void function(int a);

class DLL_PUBLIC SomeClass
{
   int c;
   // Only for use within this DSO(Dynamic Shared Object)
   DLL_LOCAL void privateMethod();
public:
   Person(int _c) : c(_c) { }
   static void foo(int a);
};
```

如上的例子中，`void function(int a)`和`class SomeClass`在库的内部和外部都可访问，但是类的`void privateMethod()`接口只能在库的内部使用，外部是无法使用的。

至此，我们给出当前现状下C/C++库级别API的管理建议：**可以使用编译选项默认隐藏库的符号，然后使用编译器属性显示指定库需要导出的API**。

最后我们补充一点对动态库的要求。

不同平台对于静态库和动态库的使用大部分时候是相似的，但在某些细节上仍然会有区别。例如所有平台中的静态库(.a或者.lib)都是可以缺符号的，即在生成时可以存在待链接的外部符号。然而对于动态库，OSX下要求不能缺符号（OSX下是dylib格式，生成时是需要链接成功的，如果缺符号链接器会报错）。而在Linux系统下动态库(.so)生成的时候却是可以缺符号的。在Linux下，如果是在链接期使用缺符号的so，则需要构建目标通过指定其它的动态库或者静态库为缺失符号的so把符号补全，否则就会链接失败。而如果是采用`dlopen`的方式打开so的话，那么该so必须自身符号是完备的，否则在动态加载的时候会出错。

因此，在这里我们给出另一个C/C++库符号管理的建议：**保证动态库不要缺符号，是自满足的**。如果违反了这条原则，那么这个库就无法用于动态加载；即使对于链接期使用，因为把符号缺失的细节泄露给了使用者，造成使用方的麻烦，所以也是不推荐的。

动态库可以和静态库进行链接，以获取自己需要的符号。但是有些时候我们只想要和静态库进行链接，却不想在动态库中将静态库中的符号暴露出去。这时可以采用`-fvisibility=hidden`选项重新编译该静态库。但遗憾的是我们不总是能够控制第三方静态库的编译过程，这时可以借助链接器提供的显示指定符号表的方法。该方法需要按照链接器的规范写一个导出符号表，在链接期通过参数传递给链接器，这样就可以精细的控制动态库需要暴露的符号了。该方法并不常用，因此我们不多做介绍，具体用法可以参考[https://www.gnu.org/software/gnulib/manual/html_node/LD-Version-Scripts.html](https://www.gnu.org/software/gnulib/manual/html_node/LD-Version-Scripts.html)。

最后我们总结一下对于库符号管理的一些建议：

```
1）推荐使用编译选项默认隐藏库的所有符号，然后使用编译器属性显示指定库需要导出的API；
2）保证动态库不要缺符号，是自满足的；
```

## 头文件管理

## 依赖管理

### 基于构建

### 基于包管理

## 热补丁

## 代码举例

开放与隐藏是有矛盾的，自寻平衡。

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

由此可见，MODULE不允许链接是CMAKE的限制，它利用了不同的平台的限制来完成这个约束。但是在linux下，由于是不分bundle和so的，所以绕过CMake，只要产生的so都是可以链接的。

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

对于CMake，也可以如下指定可见性：`set(CMAKE_C_VISIBILITY_PRESET hidden)`或者对于C++：`set(CMAKE_CXX_VISIBILITY_PRESET hidden) `。

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

- 可以在生成动态库的时候，通过静态链接把自己需要的符号包含完整：

```c
//var.c

int g_value = 5;
```

```cmake

add_library(var STATIC var.c)
add_library(value SHARED value.c)
target_link_libraries(value var)
```

这样libvalue.so中的符号就是完整的，自然静态或者动态使用都是没有问题的。

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

所以最终使用方链接libvalue.so的时候不用指定链接libvar.so:

```cmake
add_library(var SHARED var.c)
add_library(value SHARED value.c)
target_link_libraries(value var)

add_executable(main main.c)
target_link_libraries(main value) # 只用指定链接libvalue.so
```

而当我们删除libvar.so后，再次运行main就会失败：

```sh
./main 
./main: error while loading shared libraries: libvar.so: cannot open shared object file: No such file or directory
```

上述行为在main使用dlopen动态加载libvalue.so时是一样的: 只用动态加载libvalue.so，不用指定libvar.so。而当删除libvar.so后，再次运行main也会出错，提示找不到libvar.so。

- 可以在main的链接中指定libvalue.so缺少的符号所在的库

```cmake
add_library(var STATIC var.c)
add_library(value SHARED value.c)

add_executable(main main.c)
target_link_libraries(main value var) # 显示指定链接var静态库
```

这时libvalue.so中依然是缺失g_value符号，但是最后main的可执行程序的符号表里面是有g_value符号的，它是从libvar.a中链接到的。

而将var改为动态库后，main的符号表中也没有g_value符号了，而是运行时从libvar.so中获取。当删除libvar.so后，再次运行main会失败：

```sh
# rm -rf libvar.so
# ./main 
./main: error while loading shared libraries: libvar.so: cannot open shared object file: No such file or directory
```

而用上面的思路继续测试动态加载的场景： 让main先和var静态链接，然后在运行时动态dlopen打开libvalue.so：

```cmake
add_library(var STATIC var.c) # 或者写为： add_library(var SHARED var.c)
add_library(value SHARED value.c)

add_executable(main main.c)
target_link_libraries(main dl var)
```

上例中无论是将var生成为静态库还是动态库，对于main来说都不会将var中的符号提前导入。
所以main中dlopen libvalue.so的时候，会报错：

```
 ./main 
./libvalue.so: undefined symbol: g_value
```

这是由于main在和var链接（无论var是静态库还是动态库）的时候，main中是不缺符号的，因此不会提前从var库中链接任何符合（包括libvalue.so中缺少的g_value）。

通过这个例子，我们知道动态库缺少符号，只能通过静态链接的方式，由使用方帮其通过链接其它的库，将符号在最终的目标中补全。而即使采用这种方式，main也要自己知道libvalue.so的所有依赖，这样无异于把libvalue的依赖暴露给了使用者。因此我们建议对于动态库，最好是完整的，自己将自己的依赖链接全，不要缺符号。

对于静态库，一般情况下和别的静态库不会提前链接，另外静态库经常用于代码裁剪，所以不做这个约束。

## reference

- [Controlling the Exported Symbols of Shared Libraries](https://www.gnu.org/software/gnulib/manual/html_node/Exported-Symbols-of-Shared-Libraries.html)
- [Why is the new C++ visibility support so useful?](https://gcc.gnu.org/wiki/Visibility)
- [Control over symbol exports in GCC](http://anadoxin.org/blog/control-over-symbol-exports-in-gcc.html)
- [Introduction to symbol visibility](https://developer.ibm.com/technologies/systems/articles/au-aix-symbol-visibility/)
- [关于__declspec(dllimport)的理解](https://blog.csdn.net/sinat_22991367/article/details/73695039)