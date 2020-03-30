# C/C++符号隐藏与依赖管理

众所周知，解决代码耦合问题的核心原则之一是信息隐藏。即把所有客户不需要关心的信息尽力隐藏起来，只暴露出仅需要被依赖的东西。这样做不仅可以控制变化的波及范围，降低测试成本，提高安全性，同时还能方便各种代码分析工具和重构工具对代码进行更精准的引用分析。

在信息隐藏的基础上，我们还需要做好不同软件模块之间的依赖管理。依赖管理包含了如何控制最小化依赖，如何发布自己的API，如何获取别人的API，以及如何对依赖关系进行追溯和控制，包括解决冲突。

信息隐藏和依赖管理影响着软件能否高效的独立开发、构建和测试，关乎着开发团队的协作效率以及软件的独立发布粒度。基于此，现代编程语言大都提供了丰富的信息隐藏手段和依赖管理工具，包括不同级别的代码符号可见性控制、显示的API导入与导出能力，以及模块化构建和包管理的工具链等等。

然而我们知道，C和C++语言由于出现年代早、历史包袱重，对符号隐藏以及依赖管理的特性和工具的支持演进较慢。尤其是C语言，相比其它语言更是捉襟见肘。在我咨询的大多C或C++项目中，最容易被问及的问题中就包含“我要如何管理这乱麻般的项目头文件”，并且但凡和代码有关的工作（无论重构、构建、测试）大多都得先从梳理混乱的头文件包含关系开始。

事实上，C和C++语言在实践中是有发展出自己的一套信息隐藏和依赖管理机制的，这些做法已经被许多优秀的C/C++项目广泛使用着。下面给大家介绍下这些C/C++信息隐藏与依赖管理的优秀实践，虽然和别的语言相比仍旧有不完美的地方，但对于绝大多数项目来说已经完全够用了。

## 基本的符号隐藏

C语言中全局变量和函数的符号是默认外部可见的。只要我们能看到一个符号的声明（通过包含头文件或者本地`extern`声明），我们就可以在一个编译单元中直接使用另一个编译单元中的符号，甚至是另一个软件库中的符号。因为符号的全局可见性，链接器会在链接期帮我们跨编译单元找到对应的符号并进行链接。

C语言这种默认全局可见性看起来使用简单，但却在实践中引起了很多麻烦。

首先全局可见性增加了代码符号的冲突几率。为了避免符号冲突，在大的C项目中我们必须为所有全局变量和函数起很长的名字，一般需要加上“子系统名_”或者“模块名_”之类的前缀。这样导致代码不够简洁，而且生成的二进制还会占用更多的空间。

另外，默认的全局可见性让使用`extern`的成本很低。`extern`为使用外部符号提供了一种直通车机制，这种做法绕过了别人提供的头文件，可以直接引用对方本不想不暴露的符号。这不仅造成一种间接的隐式依赖，而且还导致了潜在的安全风险。

对`extern`不加控制的项目，其依赖关系最终肯定会变成一团乱麻。更进一步，`extern`会造成全局变量和函数原型的重复声明，这不仅破坏了`DRY（Don't Repeat Yourself）`原则，还为代码埋下了潜在的安全问题。我已经不止一次见过全局变量的维护者修改了变量类型，如将`U32 g_ports[MAX_NUM]`修改为`U16 g_ports[MAX_NUM]`但是不小心遗漏了某处`extern U32 g_ports[MAX_NUM]`，然后引起了各种难以定位的内存和复位问题。

所以，我们需要遵守的第一条重要的原则是：**尽量避免使用`extern`关键字**。`extern`只在很少几种情况下是有用的，例如明确要链接某些第三方的没有头文件的二进制库，或者调用汇编编写的函数以及访问编译器/链接器自动生成的符号等。

尽力消灭代码中的`extern`绝对会改善你的设计，但是这并没有改变C语言会将符号置为全局可见的事实。这时我们需要另一个非常重要的关键字`static`来帮忙。

`static`是C语言中仅有的用于隐藏符号的手段，因此用好它的意义十分重要。

`static`在C语言中主要有两种作用。1）对于函数内的局部变量，它指示该变量的内存不在栈上，而在全局静态区。2）对于全局变量和函数来说，它指示对应的符号可见性被约束在本编译单元内，不会暴露出去。

对于符号隐藏，我们主要使用`static`的第二个用途。由于使用`static`修饰的全局变量和函数的符号不会被导出，所以我们可以给这些变量和函数起更精炼的名字，同时编译器也会帮我们做更好的优化，生成更小的二进制。

更重要的是，尽量多的使用`static`会让我们改善设计，进而得到符合“Modular C”风格的设计。即将状态（全局变量）和无需暴露的函数通过`static`隐藏到编译单元内部，只将真正的API接口声明到头文件中。由于使用`static`修饰的符号是没法`extern`的，结合上一条，强制使用方只能显示的通过包含对应的头文件来调用开放的API，这样代码自然变得更加的模块化。

所以，我们给出C语言符号隐藏另一个原则：**尽可能多的使用`static`关键字来封装细节，让代码遵从Modular C的设计风格**。

现在我们转向C++。得益于C++的面向对象特性，我们有了类以及对应的访问性控制关键字`private`、`protected`和`public`。这些关键字可以修饰类的成员以及类的继承关系，从而对内和对外呈现出不同级别的可访问性。这些关键字的用法在各种教科书中都有，本文不做更多介绍。 推荐大家熟练掌握这些关键字的用法，记得千万不要把类中的一切都公开出去（虽然我见过很多人确实这么做的）。记住一个原则，那就是**尽可能多的使用`private`关键字**。

除了类，C++语言还有一个用于隐藏信息极好的特性，那就是命名空间`namespace`。`namespace`让我们能够对符号分类，将其控制在独立的命名空间中，而不用像C语言中那样靠增加名字前缀来避免符号冲突。

遗憾的是C++中命名空间是没有可访问性控制的，也就是说命名空间中的符号全部是公开的，外部通过命名空间路径都是可以访问到的。不过C++语言提供了匿名命名空间的特性，凡是在匿名命名空间中的符号都是不导出的。也就是说匿名命名空间中的符号只在本编译单元内部可见，外部是不能使用的。其作用类似于C语言中的`static`，但是写起来更加简洁。

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

如上面例子中：`Port`、`ports`和`getRateOf`只能在"example.cpp"中访问，而`getPortRate`则在该编译单元外也可以使用。这里对于C++语言，我们推荐**尽可能使用命名空间来管理符号，尤其是使用匿名命名空间来隐藏符号**。

C++语言为了兼容C，仍旧使用头文件机制发布API。为了在C++的头文件中更好的隐藏符号，我们在这里先来区分两个概念：“可见性”与“可访问性”。

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

用户只要包含这个头文件，就可以看到`Storage`类中的所有的方法声明以及成员变量定义。因此从可见性上来说，这个类的所有函数声明和成员变量的定义都是外部可见的。然而从可访问性上来说，我们只能访问这个类的公开的构造函数`Storage()`和`getCharge()`接口。

从上面的例子中可以看到，C++头文件中类定义对外的可见性和可访问性是不一致的。当可见性大于可访问性的时候，带来的问题是：当我们修改了类的私有函数或者成员变量定义（用户可见但是不可访问的符号）时，事实上并不会影响用户对该类的使用方式，然而所有使用该类的用户却被迫要承担重新编译的负担。

为了避免上面的问题，降低客户重新编译的负担，我们需要在头文件中尽量少的暴露信息。对类来说需要尽量让其外部可见性和可访问性在头文件中趋于一致。

那要怎么做呢？主要有以下手段：

- 可以将类的静态私有（static private）成员直接转移到类实现文件中的匿名命名空间中定义；

如上例中的`static unsigned int totalCapacity`是不需要定义到类的头文件中的，可以直接定义到该类实现文件的匿名命名空间中。

```cpp
// Storage.cpp

#include "Storage.h"

namespace
{
    // remove "static unsigned int totalCapacity" in Storage.h, and define it here
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

- 对于类的普通私有成员方法，可以将它依赖的成员变量当做参数传给它，这样它就可以变成类的静态私有函数。然后就可以依照前面的方法将其移到类实现文件中的匿名命名空间中；

如上例中类的`bool isValid() const`私有成员方法的实现中访问了类的成员变量`this->capacity`。我们修改`isValid`方法的实现，将`capacity`作为参数传递给它，这样`isValid`在类中的声明就可以变为`static bool isValid(unsigned int capacity)`，实现变为：

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

可以看到，使用PIMPL方法就是把所有的调用都委托到一个内部类（本例中的`Impl`）的指针上。由于指针的类型只用做前置声明，所以使用PIMPL手法的类的私有成员只用包含一个内部类的前置声明和一个成员指针即可。而`Impl`类则包含了原来类的所有真正的成员和函数实现。因为`Impl`类可以实现在cpp文件中，所以达到了进一步隐藏信息的效果。

从上例我们看到，由于`Storage`类的所有私有成员都转移到了内部的`Impl`类中，所以`Storage`类的头文件中不再需要包含"StorageType.h"，只用在实现文件中包含即可。因此使用PIMPL手法，可以解决头文件耦合与物理依赖传递的问题。

不过，通过代码示例也可以看到使用PIMPL方法是有成本的，它增加了间接函数调用和动态内存分配的开销。而且由于代码多了一层封装，导致整体复杂度上升了。因此除非解决某些严重的物理依赖问题，一般不会大面积使用该手法。

最后，一个完备的PIMPL实现会借助`unique_ptr`类型的智能指针。本例为了简化示例所以采用了裸指针实现，更完整和通用的PIMPL实现可以参见 [https://en.cppreference.com/w/cpp/language/pimpl](https://en.cppreference.com/w/cpp/language/pimpl)。

到此，我们总结一下C/C++语言自身有关符号可见性控制的原则和方法：

```
1) 尽量避免使用extern关键字；
2) 对于C语言，尽可能多的使用static关键字来封装细节，让代码遵从Modular C的设计风格；
3）对于C++，尽可能多的使用private关键字；
4）对于C++，尽可能使用命名空间来管理符号，尤其是使用匿名命名空间来隐藏符号；
5）头文件尽量隐藏信息，缩小头文件内的符号可见性。可以采取的手段有：
    - 将类的静态私有成员转移到实现文件的匿名命名空间中；
    - 在某些情况下，可以将类的私有方法重构成类的静态私有方法，然后移入到实现文件的匿名命名空间中；
    - 对于某些严重的头文件耦合问题，可以选择使用PIMPL方法，隐藏类的所有非公开成员及其依赖的头文件；
```

## 库级别符号隐藏

当程序规模变大之后，人们会对软件进行模块划分，以便分而治之。有了模块之后，就可以将其构建成库（静态库或者动态库）发布给别人使用。

前文所述的符号隐藏手段对于模块内代码的信息隐藏是够的，但是对于库来说是不够的。

当程序规模变大后，我们不可能把所有代码都写到同一个C文件或者CPP文件中。当代码被拆分到多个实现文件中，它们之间需要互相访问就必须通过头文件暴露自己的可访问API给别人。但是当所有文件都被打包在一起编译成库再提供给第三方的时候，这些内部开放的接口却未必都需要被作为库接口暴露出去。

常见的一种做法是将库的内部头文件和外部的头文件分开，对外不发布内部头文件。这是C/C++常用的一种库级别的头文件管理手段，后面我们会专门介绍。遗憾的是，仅通过不发布私有头文件，并没有解决所有问题。

即便不发布内部头文件，内部跨编译单元可被访问的符号默认情况下仍旧会被库全部导出。这样不仅浪费了二进制的空间，增加了库之间符号冲突的概率，而且还让软件包承担了不必要的安全风险。导出的内部符号仍旧可以被外部强制extern，或者是被拿来做一些hack的事情。

现代编程语言会引入module机制来管理软件模块或者库的外部可见性问题，让开发者在发布软件的时候显示的指定需要导出给外部的API，其它的符号都只能被内部访问。但是C和C++语言由于历史包袱重（新的特性需要尽量兼容已经编译过的既有代码），C++语言直到20版本才将module特性标准化，而C语言的module特性至今仍不见踪影。（事实上Java的module特性从2011年提出直到2017年才通过Java9发布，也历时七年之久）。

由于C++20标准刚刚出来不久，编译器对module机制的支持还很不完善，所以该特性离进入实用还有不少距离。感兴趣的同学可以看看我的朋友张超写的这篇文章[《C++ Modules 初窥》](https://www.jianshu.com/p/6ddd5dc909db)。

回到现实中，在没有语言直接支持的情况下，我们如何隐藏库的内部符号，显示的指定需要导出的API呢？

方法是有的，那就是借助编译器扩展。

GCC4之后支持使用`-fvisibility=hidden`编译选项，将库的所有符号默认设置为对外不可见。这样编译出的二进制就不会导出可供外部链接的符号。然后再结合GCC的`__attribute__ ((visibility ("default")))`属性，在代码中明确指定可以暴露给外部的API，于是我们就可以显示的控制库的对外API的可见性。

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

Windows下也有类似的机制`__declspec(dllexport)`，它和gcc下的`__attribute__ ((visibility ("default")))`作用类似。稍微不同的是Windows下还存在`__declspec(dllimport)`用于API的使用方显示导入外部API，以便编译器对代码进行优化，但gcc下没有对应的扩展。

为了让使用上述编译器扩展的代码能够跨平台，使用该特性的时候可以封装一个宏，根据代码所在的平台和编译器版本，自动转化成不同的实现。

```cpp
// keywords.h

#if defined _WIN32 || defined __CYGWIN__
  #ifdef BUILDING_MOD
    #ifdef __GNUC__
      #define MOD_PUBLIC __attribute__ ((dllexport))
    #else
      #define MOD_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #else
    #ifdef __GNUC__
      #define MOD_PUBLIC __attribute__ ((dllimport))
    #else
      #define MOD_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
    #endif
  #endif
  #define MOD_LOCAL
#else
  #if __GNUC__ >= 4
    #define MOD_PUBLIC __attribute__ ((visibility ("default")))
    #define MOD_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define MOD_PUBLIC
    #define MOD_LOCAL
  #endif
#endif
```

如上参考了"https://gcc.gnu.org/wiki/Visibility"中给出的宏定义。它根据不同的平台和编译器版本，定义了`MOD_PUBLIC`和`MOD_LOCAL`的不同实现。

```cpp
#include "keywords.h"

MOD_PUBLIC void function(int a);

class MOD_PUBLIC SomeClass
{
   int c;
   // Only for use within this DSO(Dynamic Shared Object)
   MOD_LOCAL void privateMethod();
public:
   Person(int _c) : c(_c) { }
   static void foo(int a);
};
```

如上的例子中，`void function(int a)`和`class SomeClass`在库的内部和外部都可访问，但是类的`void privateMethod()`接口只能在库的内部使用，外部是无法使用的。

至此，我们给出当前现状下C/C++库级别API的管理建议：**可以使用编译选项默认隐藏库的符号，然后使用编译器属性显示指定库需要导出的API**。

最后我们补充一点对动态库的要求。

不同平台对于静态库和动态库的使用大部分时候是相似的，但在某些细节上仍然会有区别。

所有平台下的静态库(.a或者.lib)都是可以缺符号的，即在生成时可以存在待链接的外部符号。然而对于动态库，OSX下要求不能缺符号（OSX下动态库是dylib格式，生成时是需要链接成功的，如果缺符号链接器会报错）。而在Linux系统下动态库(.so)生成的时候却是可以缺符号的。

在Linux下，如果是在链接期使用缺符号的so，需要构建目标通过指定其它的动态库或者静态库为缺失符号的so把符号补全，否则就会链接失败。而如果是采用`dlopen`的方式打开so的话，那么该so必须自身符号是完备的，否则在动态加载的时候会出错。

因此，这里我们给出另一个C/C++库符号管理的建议：**保证动态库不要缺符号，是自满足的**。如果违反了这条原则，那么这个动态库就无法用于动态加载；即使只是链接期使用，因为把符号缺失的细节泄露给了使用者，造成使用方的麻烦，所以也是不推荐的。

动态库可以和静态库进行链接，以获取自己需要的符号。但是有些时候我们只想要和静态库进行链接，却不想在动态库中将静态库中的符号间接暴露出去。这时可以采用`-fvisibility=hidden`选项重新编译该静态库。但遗憾的是我们不总是能够控制第三方静态库的编译过程，这时可以借助链接器提供的显示指定符号表的方法。该方法需要按照链接器的规范写一个导出符号表，在链接期通过参数传递给链接器，这样就可以精细的控制动态库需要暴露的符号了。该方法并不常用，因此我们不多做介绍，具体用法可以参考[https://www.gnu.org/software/gnulib/manual/html_node/LD-Version-Scripts.html](https://www.gnu.org/software/gnulib/manual/html_node/LD-Version-Scripts.html)。

而动态库和动态库的链接，其实并不需要把对方的二进制真实链接进来。目标的动态库会记住它所依赖的动态库（通过目标动态库中的rpath）。这种情况下也算该动态库是自满足的，因为用户在使用该动态库的时候，并不需要再为其寻找依赖。

最后我们总结一下对于库符号管理的一些建议：

```
1）推荐使用编译选项默认隐藏库的所有符号，然后使用编译器属性显示指定库需要导出的API；
（建议对该方法进行封装，以保证代码兼容各种平台和编译器版本）

2）保证动态库不要缺符号，是自满足的；
```

## 头文件管理

前文谈了代码和库的符号隐藏手段。在C/C++中，无论我们如何对符号进行隐藏，最后该暴露给客户的API还是要声明到头文件中发布给别人使用。如何设计和管理好头文件，决定了我们更大范围内的依赖治理水平。

首先谈谈头文件设计。这里一个重要前提是要理解**头文件首先是提供给别人使用的**。

很多C/C++程序员习惯了一个实现文件对应一个头文件，因此总下意识的觉得头文件先是给自己用的，所以无论什么声明（宏、常量、类型、函数）都一股脑先声明到自己的头文件中。

这是个很糟糕的做法！因为客户使用你API的标准做法就是包含你的头文件，上述做法的头文件会将大量实现细节暴露给所有客户，增加了彼此的耦合，造成无谓的依赖和构建负担。

所以，首先要明白头文件是提供给别人使用的，否则把所有符号都声明在自己的实现文件里岂不是更简单。因此，头文件设计要站在客户的角度去思考：1）怎么让别人用着方便？即遵循自满足原则；2）怎么减少别人不必要的依赖？即遵循最小公开原则。

下面我们看看一个具体的C的头文件：

```cpp
// executor_api.h

#ifndef H867A653E_0C66_4A68_80C4_B0F253647F7F
#define H867A653E_0C66_4A68_80C4_B0F253647F7F

#include "executor/keywords.h"
#include "executor/command_type.h"

#ifdef __cplusplus
extern "C" {
#endif

struct Executor;

MOD_PUBLIC struct Executor* executor_clone(const struct Executor* e);
MOD_PUBLIC void executor_exec(struct Executor* e, CommandType cmd);

#ifdef __cplusplus
}
#endif

#endif
```

上面是一个标准的C的头文件。首先为了保证每个头文件在同一个编译单元中只展开一次，头文件的内容必须处于Include Guard中，也即熟悉的`#ifndef ... #define ... #endif`中。

Include Guard中的宏需要全局唯一，一般使用路径名和文件名的大写加下划线，这种做法有个问题是，当文件重命名后经常忘记改对应的宏，久而久之就会不小心出现冲突。有的地方会使用`#pragma once`作为Include Guard来避免这个问题，不过这不是标准，存在兼容性问题。在上例中仍旧是采用标准做法，只是宏采用IDE自动生成的UUID，这样既能保证全局唯一，也不会和文件名产生重复。没必要纠结这个宏的可读性，它是给编译器看的，不是给程序员看的。

接下来为了自满足性，该头文件include了它依赖的其它头文件。本例中是宏`MOD_PUBLIC`和枚举`CommandType`所在的"keywords.h"和 "command_type.h"。

再往下是如下语句块：

```cpp
#ifdef __cplusplus
extern "C" {
#endif

//...

#ifdef __cplusplus
}
#endif
```

这个语句块声明了如果该头文件被C++的程序所使用的话，就将中间的所有符号包在`extern "C" { }`语句块中间（因为C++的编译器中有`__cplusplus`的定义，而C编译器下没有）。`extern "C" { }`指示大括号的符号需要按照C语言的标准进行符号链接，即不经过C++名称粉碎(name mangling)过程，这样就可以保证C++程序能正确链接到C语言的实现。

注意上述对`extern "C" { }`用途的解释，它和`extern`的含义是完全不同的。另外注意仔细看上例，`extern "C" { }`是放在所有的`#include`语句下面的，也就是说 **`extern "C" { }`中间不要包含`#include`语句**。我们希望每个头文件自己声明自己需要放置在`extern "C" { }`中的符号，不要为别的头文件代劳，否则可能出现某些匪夷所思的编译或链接错误（原因解释起来稍微有些复杂，记住这个原则就好了）。

通过上面的解释可以看到，该语句块完全是为了让C语言的API也能被C++程序所使用，因此如果可以保证该C程序永远不会被C++程序调动，也可以不用加。遗憾的是这个保证经常被打破，比如当前主流的C程序的单元测试框架大多是C++写的，因此当你要对所写的C程序做单元测试的时候，就必须把头文件交给C++程序使用。所以，如果没有特殊的原因，建议对所有的C语言头文件加上上述语句块，以保证其能在更大范围内使用。

我们继续看上例中的头文件，接下来的是一句前置声明`struct Executor`。前置声明是解除头文件依赖的好方法，一般函数的参数、返回值、以及结构体中的指针和引用类型等都只用前置声明即可，无需包含头文件。而枚举、宏以及需要知道内存布局或大小的类型定义，则需要显示包含头文件。所以上例中的`CommandType`由于是枚举所以必须包含头文件"command_type.h"，而`struct Executor`在后面的函数声明中仅当做参数和返回值，而且都是使用其指针类型，因此只用前置声明而无需包含定义其结构体的头文件。

示例的头文件的最后是对外API `executor_clone`和`executor_exec`的函数的声明，这里还进一步使用了我们之前介绍过的`MOD_PUBLIC`进行API的显示导出。

上述这些基本是一个标准的C语言头文件的全貌。

前面我们说了，头文件首先是给别人用的，但为了避免重复声明，自己也可以包含自己对外发布的头文件。

如本例，为了避免`Executor`的实现文件重复声明`MOD_PUBLIC struct Executor* executor_clone(const struct Executor* e)`和`MOD_PUBLIC void executor_exec(struct Executor* e, CommandType cmd)`，所以executor.c也包含了executor_api.h。

```cpp
// executor.c

#include "executor/executor_api.h"

struct Executor {
    // ...
};

struct Executor* executor_clone(const struct Executor* e) {
    // ...
}

void executor_exec(struct Executor* e, CommandType cmd) {
    // ...
}

```

如果需要把某些符号通过头文件共享给内部其它实现文件，但是又不需要把这类头文件公布出去。这时建议把头文件分开，明确分成对外头文件和私有头文件。自己可以同时包含对外的和私有的头文件，但对外只发布公开头文件。

假设本例中，`Executor`的结构体定义需要向内部公开，但是外部并不需要看到。这时可以新创一个内部头文件executor.h包含`struct Executor`的定义，但对外仍然只发布executor_api.h。这时executor.c可以同时包含executor_api.h和executor.h，而外部客户只能包含executor_api.h，无法访问到executor.h。

除了按内外部用途将头文件分开，有的时候当满足 1）库的使用方明确且有限；2）库的使用方对库头文件中符号依赖存在明显差异；这时为了避免库的不同用户因为依赖相同的头文件而互相影响（例如库按照一个使用方的要求修改了头文件中的某个函数声明，却导致并不依赖该函数的其它使用方都要重新编译），这时可以按照“接口隔离原则”，把对外头文件按照不同用户进一步分开。一般大项目中的内部模块会容易满足上述条件，而开源代码由于并不能假设自己的用户所以一般不这么做。

OK，接下来我们遇到的问题是，当按照内外部用途拆分开的头文件越来越多，在目录结构上要如何进行有效的规划和管理呢？

继续用上面的例子示例，当前社区对于单个库目录的主流布局如下：

```
executor
│
│   README.md
│   CMakeLists.txt    
│
└───include
│   │
│   └───executor
│       │   keywords.h
│       │   command_type.h
│       │   executor_api.h
│       │   ...
│   
└───src
│   │   executor.h
│   │   executor.c
│   │   ...    
│   │   CMakeLists.txt
│   
└───tests
│   │   executor_stub.h
│   │   executor_stub.cpp
│   │   executor_test.cpp
│   │   ...    
│   │   CMakeLists.txt
│   
└───benchmarks
│   │   performance_test.cpp
│   │   ...    
│   │   CMakeLists.txt
│   
└───docs
│   │   quickstart.md
│   │   apis.md
│   │   ...    
```

在这个目录布局中，首先会将所有对外发布的头文件都放在"include/<module_name>"目录下，这样方便发布的时候直接把include下的所有头文件一次导出。

这里在include目录和实际的头文件中间增加一层以模块名命名的目录（如include/executor），是为了无论自己还是发布后给别人用，都希望对外头文件的包含路径能明确的从模块名开始（make中-I统一指定到每个模块的include目录），这样方便一眼看出头文件是哪个模块的API。例如上例中无论是内部还是外部使用executor_api.h，都希望写作`#include "executor/executor_api.h"`，这样一眼看去便知当前依赖的是executor模块的API。

在上面的目录布局中，所有的实现文件都放在src目录下，内部头文件也放在src目录下，和自己的实现文件放在一起。

其它常见的顶级目录还有：

- tests目录下是库的功能测试用例以及供测试代码使用的桩文件，还有测试单独使用的头文件；

- benchmarks目录下是性能测试用例，或者其它非功能性测试用例；

- docs目录下是库的使用手册或者API接口文档等；

无论是include/executor目录，还是src、tests、benchmarks目录，需要的时候都可以在内部继续划分子目录。

再稍微看看构建。库顶层的CMake文件用于对构建做整体控制，指定构建src目录，以及选择是否构建tests和benchmarks。

src、tests和benchmarks下有自己更具体的CMake文件用于控制内部的构建细节。由于对外头文件和内部头文件的分离，所以构建脚本的编写也变得容易。关于构建的话题，我们后面会详细的讲述，这里先略过。

上述目录结构是C/C++社区主流的一种布局规范。社区中还有其它的一些布局格式，但是经过对比并不比这个布局清晰及使用范围大。另外这个布局与其它和C/C++语言相似的现代化语言的标准布局是趋于一致的（如RUST）。

我们推荐在实践中尽量遵循上述目录布局规范。即使在一个集中式的大项目中，也请保持其中每个模块的目录布局符合上述规范，即内外部头文件分离，同时每个模块自己维护和管理自己的头文件。切忌不要把所有模块的对外头文件都集中放到一个大目录下，这样会让每个模块的头文件和实现离得过远，还容易导致把所有模块的公开头文件一下子全部暴露给每个模块从而引起各种依赖混乱问题。这个话题我们在后面谈依赖管理时还会再聊。

至此，我们总结下对头文件设计和管理的一些建议：

```
1）明白头文件首先是提供给别人使用的，头文件设计要遵循自满足原则和最小公开原则；
2）遵循头文件的设计规范，本文提到了Include Guard，extern "C"和前置声明等使用时的一些最佳实践；
3）将对外头文件和对内头文件分开；在满足一定条件（库的使用方明确、有限，且对库接口的依赖存在明显差异）时，可以进一步按照接口隔离原则将对外头文件对不同用户分开；
4）对头文件的目录管理尽量遵循主流的社区规范；避免将所有模块或者库的对外头文件集中放置到一起然后暴露给所有用户；
```

## 依赖管理

一个项目中，除了非常底层的软件模块外，大多数模块都需要其它的模块的协助才能完成功能，这需要借助模块之间的依赖管理能力。

依赖管理包含如何控制模块间的最小化依赖，如何发布自己的API，如何获取别人的API，以及如何对依赖关系进行追溯和控制，包括解决冲突。

依赖管理不仅决定了模块间的协作方式，还决定了单一模块能否高效的独立开发、构建和测试，以及能否独立的进行发布。

前文我们总结了每个模块如何做好自己的符号隐藏与头文件设计，那么模块之间的依赖又要如何管理和维护呢？

为了回答这个问题，我们先来审视下不同开发阶段对于所依赖的其它模块到底需要哪些东西。

在写代码或者阅读代码的时候，我们需要看到当前模块所依赖的其它模块的外部头文件。只有这样代码才能不缺失符号声明，IDE才能正常解析、跳转和提醒，我们才能正确调用所依赖的接口完成自己的代码开发。

所以在模块的独立开发过程中，能看到所依赖模块的公开头文件是至关重要的，而对其它模块的内部细节（内部头文件、实现文件、构建脚本等）都是不需要的。

当然开发过程中还要能对所开发的模块执行独立的构建，以便能快速验证当前的代码能否被正确编译和链接。这时有可能需要依赖的其它模块的二进制，这取决于当前模块是要构建成静态库、动态库还是可执行程序。

如果当前模块是构建成静态库，那么它的构建活动主要是编译和打包，所以从严格意义上说是不需要依赖方的二进制的。

如果当前开发的模块是要构建成一个动态库或者可执行程序，那么如我们前文所述它必须要能完整链接，这时就必须能获得它所依赖的其它模块的二进制。

所以从独立构建的角度来说，我们最多还需要所依赖的其它模块的二进制。

但是如果正在开发的模块有基于代码的测试工程，无论是单元测试工程还是针对整个模块的功能测试工程，就可以通过运行测试工程的构建来触发模块源码的构建。这时缺失的外部符号可以用桩来填补，因此可以降低对外部的二进制依赖。

为了提高测试工程对构建的验证有效性，我们需要遵循一些原则：1）测试的构建环境和生产构建环境尽可能保持一致；2）测试工程尽量复用被测模块的生产环境构建脚本；3）测试构建产生的模块二进制库最好和生产环境保持一致。

在满足上述条件后，我们可以在开发阶段大胆的使用测试构建代替真实构建，以降低我们对其它模块的二进制依赖，提高我们的开发效率。

不过对于动态库和可执行程序，不要忘了在最终发布的时候，仍然是需要和真正的依赖方的二进制进行链接的。所以从完整意义上来说，对于动态库和可执行程序，在构建阶段仍是需要能够获取到依赖方的二进制的。

通过上面的分析我们看到，想要独立开发、构建和测试，最重要的是能够获取依赖方的公开头文件，而在一些场合下还需要依赖方的二进制。有上述这些就够了。

那么回到依赖管理上，**好的依赖管理技术就是要保证我们在不同阶段所依赖的东西可以低成本的精准获得，同时又不会过度获得。**

我们来看看当前C/C++常见的依赖管理手段。

首先是基于源码的依赖管理。常见的做法是将所有代码都在一个代码仓中，模块之间通过目录进行隔离。这种情况下，我们只要clone下代码，就可以看到其它所有模块的代码，无论是头文件还是实现代码。

这种依赖管理方式简单、低成本，但却不是“精准”的。模块之间太容易“过度”看到对方的细节，因此容易导致从源码到构建上不必要的耦合。

这种方式下经常遇到的第一个问题就是模块间的头文件耦合。由于源码都在一起，所以很容易要求所有模块的公开头文件全部集中放置在一个目录下，每个模块都可以include这个目录。这种方式下每个模块依赖其它模块的公开头文件成本很低，但也正是因为成本低所以很容易随意包含。最终导致大家都互相交织在了一起而难以独立发布。

上述集中式头文件管理存在两个常见的变种：一个是在构建开始阶段再把所有模块的公共头文件拷贝到一起然后执行构建，另一个是通过构建变量先将每个模块的公开头文件路径串在一起然后逐一传递给每个待构建模块的构建脚本。这两个变种和上面的头文件集中管理的问题是一样的，每个模块仍旧可以看到并随意包含其它所有模块的头文件。更糟的是，这两种做法还进一步带来了构建上的依赖：每个模块的构建都必须先从根构建开始执行（因为根构建可以跨模块完成构建前的文件拷贝或者构建变量拼接行为），这会导致内部模块丧失独立构建的能力。

我们容易在基于源码的依赖管理方式下遇到的第二个问题就是构建的耦合。这种方式下构建往往喜欢采用自顶向下设计：即整个项目需要从根构建开始执行，先准备环境，初始化各种构建变量，然后按照依赖顺序逐一执行每个模块的构建，最后再链接和打包。这种方式虽然整体看起来简单高效，但是却造成了了每个内部模块之间以及与全局之间的构建耦合。每个内部模块的构建都必须从根构建开始执行，不仅构建速度慢，而且丧失了模块的独立构建的能力。

说了这么多，那么在基于全量源码的依赖管理方式下，是否就不能做到内部模块的独立的开发、构建、测试与发布呢？答案是可以做到。

看看以下做法：

- 每个模块在自己的目录下自行维护自己的公开头文件（还记得前文中推荐过的模块目录布局吗）；
- 每个模块有自己**内置**的独立构建脚本和启动入口，并且有一致的模块级构建和测试的触发命令；
- 每个模块可以通过构建入口参数或者环境变量获得`PROJECT_ROOT`，作为整个项目源码的根目录；
- 所有模块基于`PROJECT_ROOT`的相对路径遵循一套统一的约定，包括二进制的发布路径（可以是在每个模块内部的临时目录，也可以是在`PROJECT_ROOT`下的某个集中路径）；
- 每个模块根据自己的对外依赖，在自己的构建脚本里面描述所依赖的其它模块的头文件路径（可以按照约定用`PROJECT_ROOT`和模块名计算出来）。如果构建需要其它模块的二进制，就在约定的二进制目录下获得，如果找不到就调用统一的模块构建命令触发依赖模块进行构建；
- 构建成功后，将生成的二进制发布到约定二进制目录中；
- 如果模块要独立发布给第三方，需要模块里有内置的打包脚本（可以写到构建脚本里面），在构建成功后将自己的头文件和二进制（如果是动态库或者可执行程序，则还要包含所依赖的二进制）按照打包格式进行打包，并发布到对应的仓库中；

上面的这套做法，有点类似早期golang语言的依赖管理方式。没错，golang在没有引入module机制之前采用的就是基于`GOPATH`的单一代码库管理方式，它是Google的单一代码库实践在golang语言中的应用。Google在单一代码库中能做到内部软件模块的独立开发、构建、测试与发布，是由良好的设计规划能力，工程工具能力，以及以团队自治为基础但又不缺乏整体协作纪律的组织方式和文化做基础的。

上述这套做法，解决了前面说的在全量源码管理方式下的模块与全量头文件耦合以及模块与外部构建之间的耦合问题，最终让每个模块可以做到独立开发、构建、测试和发布。这里每个模块相当于是一个闭包，它自行管理自己的头文件和完整的构建以及命令入口。这套做法遵循约定优于配置的原则，制定了一套需要共同严格遵守的约定，每个模块的独立构建和发布过程都基于这套约定之上。

采用上述这种做法，对构建工具和构建设计会有一些更高的要求。

首先，构建设计需要解决每个模块的构建代码中共享的构建配置、构建参数和工具脚本的重复问题。由于现在每个模块是一个自治的构建单元，拥有自己独立的构建脚本和内置的构建启动入口。这样所有模块的构建都需要干更多重复的事情，比如配置一样的构建环境、选择相同的体系架构和编译链接参数等等。这些公共活动和代码需要通过设计提炼，出来作为共享的构建库，让每个模块在构建时自行依赖和调用。如果模块采用源码发包的话，这些共享的构建代码库还需要作为包的构建时依赖一起发布，以便客户在构建时也能获得。

而对构建工具更高的要求，主要是需要构建工具能按照构建目标控制构建变量的作用域和传递性。软件模块在构建的时候可能创建了某些构建变量，用于保存编译参数、预编译宏或者所有依赖的头文件路径等，这些构建变量我们希望它们的作用域和传递性是可以控制的。比如我们不希望在构建执行过程中触发了依赖模块的构建后，当前模块的这些构建变量被默认继承了过去，也不希望依赖模块构建结束后修改了当前模块的构建变量的值。因此我们希望构建工具能支持更好的模块化构建，即按照不同的构建目标控制构建变量的作用域和传递性。

幸运的是[CMake](https://cmake.org/)从3.0版本开始支持模块化构建，它引入了target的概念，以及基于target建立起了构建上下文的可见性和传播控制机制，可以满足我们的上述需求。关于CMake的这些用法和实践方式，建议看看我的朋友尉刚强的这篇文章：[《Modern CMake最佳实践》](https://www.jianshu.com/p/8abf754654c4)。强烈建议那些在用CMake，但仍旧以老的directory为中心的方式在用的项目，能够切换成以target为中心的使用方式，不要浪费了Modern Cmake的这个核心特性的价值。

还剩下一个问题是：采用上面这种方式，整个项目完整的构建和发布怎么做呢？可以把项目的完整构建和发布也当做一个内部模块，它可以没有任何业务代码，但是拥有自己独立的构建脚本。和任何普通的模块一样，它在自己的构建里面先在约定的二进制目录中寻找它所依赖的其它模块的二进制，如果找不到就触发对应模块的构建和发布，最后再完成整体的打包和发布。可见这种方式下，我们根本不需要之前的自顶向下的构建过程，每个构建都是平等且独立的。另外，由于构建的闭包化，其实更加容易的进行并行构建。

如果是一个集中式的项目，上述方式就已经能够满足依赖管理的需要了。上述方式可以帮每个模块“轻易而精准”的获取依赖，虽然仍旧有些“过度”（毕竟还是能够看到别人的源码），但是通过工具以及纪律约束，也可以保证不会有副作用。

然而上述方式对于社区化开发是完全不够的。

社区化开发很难将所有依赖的源码都放在一起，也很难控制其它依赖的变更时机及其兼容性，这时就需要更强大的依赖管理手段了。这个手段就是我们都知道的“包管理”。

包管理最大的价值在于制定了一套管理软件包的统一标准，其中包括包的版本标准、打包发布标准、全链条的依赖追踪与冲突解决标准，以及基于这套标准之上的工具链。包管理可以满足我们对依赖管理的完整定义：即可以保证我们在不同阶段的依赖都能够低成本的精准获得，同时又不会过度获得。因此，大多数编程语言都把包管理作为语言的工程核心对待。遗憾的是由于C/C++包管理的不成熟，所以对包管理的使用并不如其它语言那么普遍。

关于C/C++包管理的最新进展以及使用建议，可以看看我的这篇文章[C/C++代码复用与包管理](https://www.jianshu.com/p/5de358c1c007)，这里就不再赘述了。

最后总结一下关于依赖管理的话题:

```
1) 依赖管理技术要保证在不同阶段所依赖的东西可以低成本的精准获得，同时又不会过度获得;
2) 依赖管理关乎软件模块能否独立的开发、构建、测试与发布。做好依赖管理需要好的设计规划能力，工程工具能力以及纪律约束；
3）对C/C++来说，构建工具和构建设计是依赖管理中非常重要的一环；
4）根据自己的项目特点，选择在合适的时机使用包管理器，对依赖进行更好的管理；
```

## 尾巴

由于在写这系列文章的时候，恰好脚踝通风引起滑膜炎，痛苦不堪，所以就先写到这里了。

在写这些文章的时候，把很多优秀的C和C++开源库又翻出来看了下。C的有[cJSON](https://github.com/DaveGamble/cJSON)、[jemalloc](https://github.com/jemalloc/jemalloc)、[libuv](https://github.com/libuv/libuv)、[redis](https://github.com/antirez/redis)、[sqlite](https://repo.or.cz/sqlite.git)；C++的有[CAF](https://github.com/actor-framework/actor-framework)、[cpp-react](https://github.com/schlangster/cpp.react)、[folly](https://github.com/facebook/folly)、[muduo](https://github.com/chenshuo/muduo)。本来想找对符号隐藏和依赖管理做得好的库给大家剖析下的，但是这次就先缩水一下吧。

在上述库中，推荐大家有精力的话可以看看著名的[libuv]（https://github.com/libuv/libuv）。libuv是一个跨平台的异步IO库，被用在著名的nodejs里作为事件驱动的引擎。它综合使用了我们前面说到的各种实践：从目录布局，到库的API符号隐藏，到构建设计，当然它的源码设计也是很不错的。

最后，祝大家2020年一切顺利！

## 补充

### CMake Module特性测试

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

### control the visibility of API

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

### symbol table for LD

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

### do not miss symbols in dynamic library

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