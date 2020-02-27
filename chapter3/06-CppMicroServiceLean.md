# CppMicroServices插件架构平台


## 架构应用

### 要解决的问题
CppMicroServices 是一个实现OSGI标准的框架，提供插件注册和插件生命周期管理的能力。
CppMicroServices 使用cmake来做构建
    
### quick start
必须在cmake中定义`US_BUNDLE_NAME`，并使用框架预定义好的cmake函数统一目录，打包等。
```
usFunctionAddResources(TARGET target [BUNDLE_NAME bundle_name]
  [WORKING_DIRECTORY dir] [COMPRESSION_LEVEL level]
  [FILES res1...] [ZIP_ARCHIVES archive1...])
```

还需要定义`manifest`，内入如下：
```
{
  "bundle.symbolic_name" : "eventlistener",
  "bundle.activator" : true
}
```
代码中需要继承`BundleActivator`，并使用宏
`CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR`来生成标准化的创建和销毁函数
然后实现start和stop接口，在start和stop接口函数中，可以通过BundleContext可以获取其他的服务,也在可以把自己
的服务注册给BundleContext，供其他服务使用，接口可见需要增加`US_ABI_EXPORT `修饰 interface。 
``` 
CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(EnglishDictionary::Activator)
```
这个宏里用会到`US_BUNDLE_NAME`来生成创建函数和销毁函数。服务接口类可以注册多个，相
同服务接口类可以通过`ServiceProperties`来进行区分，在调用接口时，可以通过LDAP规则，
进行相同服务接口的过滤。

### 应用约束与依赖

### 框架易用性
主页有比较详细的文档和例子，github中也有比较清晰的测试用例。
[主页](http://cppmicroservices.org). 

## 架构原理

### 核心领域模型
关键的领域模型Bundle，Framework， Service， BundleContext，等详细关系见下图

![](./Framework.jpg)

### 运行态模型


#### 生命周期管理
Bundle作为插件（服务）的封装容器，最终提供动态库供Framework加载， framework使用
dlopen打开文件加载符号（linux类系统）。生命周期管理与动作的管理见下图：
![](./Start.jpg)

### Service的生命周期
在BundleActivor中在start，stop时机，通过Context注册和停止service的，此时在线程
中，可注册多个服务接口，生命周期自己控制，随Bundle被销毁。
### Bundle的生命周期
由框架调用install, uninstall管理Bundle，install后由里bundle的对象，但start后才有BundleThread。
有版本号通过BundleVersion来管理，提供基本的4位版本号定义，但未使用在版本升级上。
### Framework的生命周期
全局生命周期，绑定coreBundleContext，一旦销毁，所有注册Bundle的生命周期都结束。


BundleState的状态图如下：

![](./BundleState.jpg)
#### 运行态机制
Bundle启动后运行在线程中，线程的管理在coreBundleContext中，主要考虑Bundle之间在
运行态结偶，如果一个Bundle的启动时间比较长，不会影响其他bundle的注册启动过程。

### 框架能力

#### 隔离性
Bundle之间设计上解耦，调用时需要看到所依赖的接口的头文件，框架中设计了Linsener机制来结
偶服务提供和服务状态获取，Service,Bundle,Framework都有Linsener机制，Bundle可以添加到CoreContext，以获
取所依赖服务的情况，以保证对依赖接口的有效安全使用。

#### 扩展性
框架也可以添加HOOK，hook可以作为补丁来使用替代原有的插件，或增强原有插件。

#### 运维特性
未提供全局视角的动态依赖管理和事件跟踪

#### 易用性
提供易用的过滤机制，宏定义，怀有cmake封装，让bundle的开发更简化。

#### 协同性
根据生命周期包含的关系，Bundle释放时，由Bundle注册的服务会统一卸载，所有Bundle注
册在Framework，也会受Framework的生命周期影响，统一的进行卸载退出的动作。由于框架
并未提供统一的资源使用接口。
被依赖的Bundle退出时，只负责广播事件，比较依赖Bundle自身对依赖Bundle的事件注册监
听。

bundle像一个容器，bundleContext 只是一个代理，具体委托到BundlePrivate 和
CoreBundleContex上。安装以bundle为单位，bundle在activoer里注册自己的Service

符合可见性管理 US_ABI_EXPORT 用来导出Service的API
```
#if defined(US_PLATFORM_WINDOWS)
  #define US_ABI_EXPORT __declspec(dllexport)
  #define US_ABI_IMPORT __declspec(dllimport)
  #define US_ABI_LOCAL
#elif defined(US_HAVE_VISIBILITY_ATTRIBUTE)
  #define US_ABI_EXPORT __attribute__ ((visibility ("default")))
  #define US_ABI_IMPORT __attribute__ ((visibility ("default")))
  #define US_ABI_LOCAL  __attribute__ ((visibility ("hidden")))
#else
  #define US_ABI_EXPORT
  #define US_ABI_IMPORT
  #define US_ABI_LOCAL
#endif
```


## 设计分析
### 设计上的一些不足

* BundleContext 提供的API里包含代理的CoreBundleContext的，getBundles这类接口，和自
己的GetBundle在一起比较容易引起误解。

* 并没有设计bundleContext的分层隔离，全部的Service，Linsener等都注册在
  coreBundleContext上。分发范围和过滤范围都比较大。

* 框架没有集成包管理和依赖管理工具，整体设计是微服务框架，依赖管理上只设计了动态调
用的接口找不到的状态跟踪机制，未考虑静态包依赖的问题解决。

* 类方法的可见行管理上并不十分严谨，

* 接口方面的设计感觉还不是很完善，如何发布接口？，接口和包分开发布？接口设计与契
  约测试等还没看到。

### 设计上觉得比较好的地方

* 注释非常详细，使用Doxygen的注释规范，方便生成文档。
* 使用LDPA进行过滤，有一套完善的语法
* 是不是可以考虑使用Framework来进行分层隔离
* 框架在职责结偶方面设计的比较干净，

### 一些其他编程约束
singletons 在Bundle中的限制使用，单例导致的释放顺序问题超出了框架的管理范围。由
于Bundle的方法都在线程中调用，singletons需要考虑多线程都问题。

## 如何支持6独立

### 独立开发
bundle的开发需要依赖Framework中提供的基础扩展接口（BundleActivator，还有一些宏），如果依赖其他bundle的接口，需要在开发
时能拿到接口契约。

### 独立构建
每个bundle可以独立构建，也可以跟主框架一起构建。

### 独立测试
平台并未提供独立测试的相关支持。Framework有自己的单元测试，其他提供的bundle未看
到测试工程，因此也并无接口或契约相关的设计。

### 独立发布
bundle有一套标准的物理设计最佳实践，包含`include, src, test, resource,
CMakeLists.txt`，从物理设计上是内聚的，支持独立发布，但框架本身并没有绑定任何包管理
工具，发布形式理论上支持动态库发包和源码发包两种形式。

### 独立部署
在框架程序启动之后，可以通过加载动态库的方式独立部署。

### 独立替换
框架目前对多版本共存是默认支持的，但要通过`properties`来加以区分，虽然有版本号，
但未提供独立升级或灰度升级等方面的支持，但可以通过建立一个特殊的核心Bundle来支持
这个特性。

