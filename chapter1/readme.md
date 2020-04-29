# 组件化定义

维基百科：[基于组件的软件工程](https://en.wikipedia.org/wiki/Component-based_software_engineering)：component-based software engineering（CBSE），component-based development（CBD）

组件化发展两个阶段：
- 最开始基于组件的开发，强调通过分离关注点，让软件内部松耦合，可复用，使软件开发长期低成本；
- 实践中，人们认为组件应该是面向服务架构中的一个服务，这样就为组件引入了更多特点；例如，组件能够基于事件驱动架构（event-driven-architecture：EDA）产生和消费事件；

组件的特点：
- 一个独立的软件组件（software componnet）是一个软件包（software package），一个web service，一个web resource或者是一个封装了一系列相关功能和数据的模块；
- 组件是内聚的和模块化的；
- 组件间通过接口（interface）进行协作；接口是组件的签名，使用方无需了解组件内部，组件间仅通过接口进行协作；（UML组件图：边界、接口和协作关系）
- 组件的可替换性：只要是兼容的，一个新版本的组件就可以替换老的组件（开发态或者运行态替换）。B能替换A，B满足A的契约：对系统所提供的不少于A，所使用的不多于A；
- 组件经常是对象视图或者是一组对象视图，而非类视图（强调组件的运行完整性），另外还要附上IDL定义的组件接口；
- 组件要强调其可复用性，组件应该：1）足够的文档；2）完整的测试（功能和非功能）；3）设计时明确组件会被放置于不能预测的上下文环境；
- 组件是需要有底座框架支撑的：http://www.existentialprogramming.com/2010/05/hole-for-every-component-and-every.html。（Crnkovic, I.; Sentilles, S.; Vulgarakis, A.; Chaudron, M. R. V. (2011). "A Classification Framework for Software Component Models". 这篇论文对所有的组件化框架进行了汇总分类）；

组件的历史：
-  Douglas McIlroy's address at the NATO conference on software engineering in Garmisch, Germany, 1968, titled Mass Produced Software Components. 应对软件危机，举例是Unix操作系统；
-  Brad Cox of Stepstone largely defined the modern concept of a software component.[4] He called them Software ICs and set out to create an infrastructure and market for these components by inventing the Objective-C programming language. 
-  组件有两种形态：1）可执行程序的构成部分；2）分布式系统中通过网络交互或者IPC（进程间通讯）的每个独立可执行体；
-  一些实现：IBM led the path with their System Object Model (SOM) in the early 1990s. As a reaction, Microsoft paved the way for actual deployment of component software with Object linking and embedding (OLE) and Component Object Model (COM).


区别于模块化，维基百科：[modular programming](https://en.wikipedia.org/wiki/Modular_programming)

模块化类似于组件化第一阶段，强调解耦、复用。将逻辑上内聚的功能和数据封装起来，形成一个模块。

面向对象流行后，模块化被掩盖了。当前模块化是比类更大的一种逻辑闭包划分。语言的模块化参考Java或者C++的模块化特性演进过程；

因此可以知道，组件一定是模块，模块未必是组件。组件比模块有更多的特性要求。