# 包管理

## Linux上软件安装的四种方式

- 通用二进制编译:由志愿者把开发完成的源代码编译成二进制文件，打包后发布在网络上，大家都可以通过网络进行下载，到本地之后，经过解压配置就可以使用。
- 软件包管理器：使用包管理工具安装，有时候必须要解决软件包之间的依赖问题，例如rpm和deb等。
- 软件包前端管理工具：可以自动解决软件包依赖关系，例如yum和apt-get等。
- 源码包安装：从网络上下载软件的源码包到本地计算机，用gcc等编译工具编译成二进制文件后才能使用，有时必须要解决库文件的缺失问题。

### linux包文件部署位置

- 二进制程序，位于 /bin, /sbin, /usr/bin, /usr/sbin, /usr/local/bin, /usr/local/sbin 等目录中。
- 库文件，位于 /lib, /usr/lib, /usr/local/lib 等目录中。Linux中库文件以 .so（动态链接库）或 .a（静态链接库）作为文件后缀名。
- 配置文件，位于 /etc 目录中。
- 帮助文件：手册, README, INSTALL (/usr/share/doc/)

## Linux包管理

| 操作系统	 | 格式	          | 工具   |
|-----------|---------------|-------|
|Debian     |	.deb	    | apt, apt-cache, apt-get, dpkg |
|Ubuntu     |	.deb	    | apt, apt-cache, apt-get, dpkg |
|CentOS     |	.rpm	    | yum   |
|Fedora     |	.rpm	    | dnf   |
|FreeBSD    |	Ports, .txz	| make, pkg |

## RPM and YUM

### 功能

- 文件清单
- 文件放置路径
- 提供的功能说明
- 依赖关系

### 工具

#### RPM

软件包管理器：RPM（RedHat Package Manager | RPM is Package Manager）

RPM最大的特点就是需要安装的软件已经编译过，并已经打包成RPM机制的安装包，通过里头默认的数据库记录这个软件安装时需要的依赖软件。当安装在你的Linux主机时，RPM会先依照软件里头的数据查询Linux主机的依赖属性软件是否满足，若满足则予以安装，若不满足则不予安装。

功能： 
- RPM管理支持事务机制。增强了程序安装卸载的管理。
- RPM的功能：打包、安装、查询、升级、卸载、校验、数据库管理。
- 软件已经编译打包，所以传输和安装方便，让用户免除编译
- 在安装之前，会先检查系统的磁盘、操作系统版本等，避免错误安装
- 软件的信息都已经记录在linux主机的数据库上，方便查询、升级和卸载

缺点：
- 由于Linux中的程序大多是小程序。程序与程序之间存在非常复杂的依赖关系。RPM无法解决软件包的依赖关系。
- 软件包安装的环境必须与打包时的环境一致或相当
- 必须安装了软件的依赖软件
- 卸载时，最底层的软件不能先移除，否则可能造成整个系统不能用

#### RPM包

用RPM工具可以将二进制程序进行打包，包被称为RPM包。
RPM包并不是跨平台的。RedHat的RPM包与SUSE的RPM包不能混用。实际上RedHat的安装，初始软件也都是使用RPM包进行安装的。

RPM包的命名规范：name-version-release.os.arch.rpm

RPM分包：在把二进制文件打包时，将主要功能打入主包。将辅助功能打入分包。分包在需要的情况下安装，若不需要就可以不安装。

- 主包：核心包。
- 分包：又称为支包。

源码包和二进制包。

`rpmbuild -bs hello-1.0.0.spec`生成源码包，里面包含spec文件和源码的tar.gz文件；src的rpm包放在SRPMS下；
`rpmbuild -bb hello-1.0.0.spec`生成二进制包，里面只包含spec file段下的文件；二进制包放在RPMS下；

默认安装位置：
/etc  配置文件放置目录
/usr/bin  一些可执行文件
/usr/lib  一些程序使用的动态链接库
/usr/share/doc  一些基本的软件使用手册与说明文件
/usr/share/man  一些man page档案

RPM常用选项：
-i：表示安装。
-v, -vv, -vvv：表示详细信息。
-h：以"#"号显示安装进度。
-q：查询指定包名。
-e：卸载指定包名。
-U：升级软件，若未软件尚未安装，则安装软件。
-F：升级软件。
-V：对RPM包进行验证。
--nodeps：忽略依赖关系。
--query：查询指定包名。同-q选项。
--hash：同-h。
--install：表示安装，同-i选项。
--test：仅作测试，不真正执行，可用于测试安装，测试卸载。
--replacepkgs：重新安装。替换原有的安装。
--force：忽略软件包及文件的冲突。
--initdb：新建RPM的数据库。
--rebuilddb：重建RPM的数据库。
--percent：以百分比的形式输出安装的进度。

RPM包的查询：
rpm -q：查询某一个RPM包是否已安装
rpm -qi：查询某一个RPM包的详细信息
rpm -ql：列出某RPM包中所包含的文件。
rpm -qf：查询某文件是哪个RPM包生成的。
rpm -qa：列出当前系统所有已安装的包

- 安装RPM包
命令格式：rpm -i /PATH/TO/RPM_FILE
一般组合起来使用：-ivh
命令格式：rpm -ivh PATH/TO/RPM_FILE

- 卸载RPM包
命令格式：rpm -e 包名

- 查询RPM相关信息
结合-q选项，RPM提供了许多种查询信息的方式。
命令格式：rpm -q 包名
命令格式：rpm --query 包名
查询所有已经安装包：rpm -qa

- 升级软件
命令格式：rpm -Uvh rpm包
命令格式：rpm -Fvh rpm包

- 校验RPM包
命令格式：rpm -V 包名
对已经安装的软件，进行将要。若无输出，则表示已安装的软件没有被修改。若软件被修改，则会输出信关信息。具体请查看RPM相关手册。

- RPM的数据库
数据库文件位于：/var/lib/rpm

rpm --initdb    #新建数据库
rpm --rebuilddb  #重建数据库

#### YUM

YUM（Yellow dog Updater, Modified），在Fedora和RedHat以及SUSE中的Shell前端软件包管理器。

YUM使用Python语言写成。YUM客户端基于RPM包进行管理，可以通过HTTP服务器下载、FTP服务器下载、本地软件池的等方式获得软件包，可以从指定的服务器自动下载RPM包并且安装，可以自动处理依赖性关系。

YUM通过依赖rpm软件包管理器, 实现了rpm软件包管理器在功能上的扩展, 因此YUM是不能脱离rpm而独立运行的。

YUM在安装RPM时，会从服务器下载相应包，且缓存在本地。

YUM的配置方式是基于分段配置的。

主配置文件：/etc/yum.conf

YUM的片段配置：/etc/yum.repos.d/*.repo

YUM的特点：
1）可以同时配置多个资源库(Repository)
2）简洁的配置文件(/etc/yum.conf)
3）自动解决增加或删除rpm包时遇到的依赖性问题
4）使用方便
5）保持与RPM数据库的一致性

YUM原理说明：

- Server端先对程序包进行分类后存储到不同repository容器中; 再通过收集到大量的rpm的数据库文件中程序包之间的依赖关系数据, 生成对应的依赖关系和所需文件在本地的存放位置的说明文件(.xml格式), 存放在本地的repodata目录下供Client端取用

- Cilent端通过yum命令安装软件时发现缺少某些依赖性程序包, Client会根据本地的配置文件(/etc/yum.repos.d/*.repo)找到指定的Server端, 从Server端repo目录下获取说明文件xxx.xml后存储在本地/var/cache/yum中方便以后读取, 通过xxx.xml文件查找到需要安装的依赖性程序包在Server端的存放位置, 再进入Server端yum库中的指定repository容器中获取所需程序包, 下载完成后在本地实现安装。

##### YUM配置

- 主配置

主配置文件：/etc/yum.conf

```ini
[main]        #main仓库。[ ]中括号表示一个仓库的定义。其中是仓库的名称。
cachedir=/var/cache/yum/$basearch/$releasever    #RPM包的缓存位置。
keepcache=0    #RPM包在本地是否需要长期保存。1表示yes，0表示no。
debuglevel=2    #日志级别。
logfile=/var/log/yum.log    #日志文件。
exactarch=1    #下载的RPM包是否需要与本地平台完全匹配。1表示yes，0表示no。
obsoletes=1
gpgcheck=1    #是否需要自动来源合法性检测。
plugins=1
installonly_limit=5
bugtracker_url=http://bugs.centos.org/set_project.php?project_id=16&ref=http://bdistroverpkg=centos-release
```

- repo配置

YUM的片段配置：/etc/yum.repos.d/*.repo

```ini
[base]
name=CentOS 6.4 x86_64
baseurl=http://172.16.0.1/cobbler/ks_mirror/centos-6.4-x86_64/
enabled=1
gpgcheck=0
```

对配置文件中的一些配置项作说明：
- [ ... ]：仓库的名称。不能重复。
- name：对仓库的描述，该项必须有。
- baseurl：配置仓库的路径。用于指定一个url。
- mirrorlist：指向一个镜像列表，里面有多个url。
- enabled：是否启用当前仓库。值为1或0，默认为1。
- gpgcheck：是否需要gpg校验。值为1或0，默认为1。
- gpgkey：验证RPM包的密钥文件路径。该文件可以在远处服务器上，也可以在本地。
- cost：代价，其本质是仓库优先级的配置。值越低，表示访问的代价越低，也即优先使用。

##### YUM 命令

yum的命令形式一般是如下：yum [options] [subcommand] [package ...]

yum list相关命令
yum list all

只显示已安装的包。
命令：yum list installed

只显示没有安装，但可安装的包。
命令：yum list available

查看所有可更新的包。
命令：yum list updates

显示不属于任何仓库的，额外的包。
命令：yum list extras

显示被废弃的包
命令：yum list obsoletes

新添加进yum仓库的包
命令：yum list recent

模糊匹配搜索
命令格式：yum search 查询名
例：查询软件包名中出带有init的软件包。
命令：yum search init

查看当前能够使用的yum仓库
命令：yun repo list

显示所有仓库
命令：yum repo list all

显示禁用的仓库
命令：yum repo list enabled

显示启用的仓库
命令：yum repo list disabled

显示软件包的摘要信息
命令格式：yum info 包名
类似于rpm -qi 包名 ，yum info没有rpm -qi显示的详细全。但可以显示出安装状态（Installed，Available）

查询某个文件是由哪个软件包生成的
该功能类似于rpm -qf 包名。
命令格式：yum provides 文件
命令格式：yum whatprovides 文件

清空本地yum的缓存
yum仓库若更新，则本地缓存就没有意义了。所以本地缓存需要清空。

命令格式：yum clean [ packages | metadata | expire-cache | rpmdb | plugins | all ]

手动在本地建立缓存
yum客户端会下载远程yum的文件。在本地生成缓存。
命令：yum makecache

安装应用程序
基本格式：yum [-y] install 包1 包2 ... 包n
说明：
若安装多个包，则使用包名之间使用空格隔开。
安装过程中，yum会询问用户是否安装，使用yum -y 选项，表示自动回答为yes。
例：使用安装wget。
命令：yum install wget

重新安装软包
命令格式：yum [-y] reinstall 包1 包2 ... 包n

升级软件包
命令格式：yum update 包1 包2 ... 包n
命令格式：yum update-to 包-版本号
说明：update-to可以指定版本号。

检测可升级的包
命令：yum check-update

卸载软件包
命令格式：yum remove 包1 包2 ... 包n
注意：若该包被依赖，则该卸载可能会导致一些问题。如A依赖B，若卸载B，则A也会被卸载。

本地安装升级RPM包
在RHEL6/CentOS6可以直接使用install，update命令安装本地rpm包。
命令格式：yum install rpm包路径
命令格式：yum update rpm包路径
或者使用localinstall，localupdate。在RHEL5/CentOS5下必须使用localinstall，localupdate。
命令格式：yum localinstall rpm包路径
命令格式：yum localupdate rpm包路径

yum安装rpm默认会查询软件包来源合法性，但有时没提供密钥，无法安装。使用--nogpgcheck选项，可以避免yum作校验。
命令：yum localinstall --nogpgcheck

包组管理
rpm包可以组合成包组，安装卸载可以共同进行。

查看yum仓库里的包组
命令：yum grouplist
Installed Groups 表示已安装的组。其他组类似。

显示指定的包组信息
命令：yum groupinfo "Development tools"
安装包组
命令格式：yum [-y] groupinstall 包组1 包组2 ... 包组n
例：安装开发环境，构建编译源码的环境。
一般为了防止出现不必要的问题，开发环境需要配置如下三个包组：
RHEL6/CentOS6："Development tools"、"Server Platform Development"、"Desktop Platform Development"
RHEL5/CentOS5："Development tools"、"Development Libraries"
输入命令：yum [-y] groupinstall "Development tools" "Server Platform Development" "Desktop Platform Development"

升级包组
命令格式：yum [-y] groupupdate包组1 包组2 ... 包组n

卸载包组
命令格式：yum [-y] groupremove包组1 包组2 ... 包组n

查看此前安装卸载等操作历史
命令：yum history

##### YUM REPO安装

yum仓库又称为yum源，yum仓库一般会支持ftp协议(ftp://)，http协议(http://)，文件协议(file://)。

使用createrepo命令创建YUM仓库。该命令系统中默认是没有的。可以使用rpm或yum安装上该命令。
格式：createrepo rpm包目录
该命令会在指定目录中生成repodata目录。该目录中是所有RPM包的信息文件，及其依赖关系的信息文件。以xml文档和sqllite数据库文件的形式存储。

修改repo文件，baseurl指向yum仓库的位置。例如本地磁盘用：`file:///media/drom`

baseurl中可使用yum本地变量：

说明：
$releasever：当前操作系统的主版本号。若CentOS6.4 该值为6。
$arch：当前平台版本架构。x86_64 或 i386/i586/i686。
$basearch：当前平台的基本架构。x86_64 或 i386。
$YUM0-9：这十个变量分别被 shell 环境中的同名变量的值所替代。如果 /etc/yum.conf 文件中设置了这些变量，而 shell 环境中没有同名变量，它的值则不被代替。
例：配置基于网易镜像站的跨平台路径。
baseurl=http://mirror.sohu.com/centos/$relasever/os/$basearch/

## 遗留问题

- yum下载rpm包的时候，不会自动下载RPM包的构建时依赖（spec中 build requirements）对吧？
- yum下载rpm包的时候，会自动下载RPM包的发布时依赖（spec中：requirements中对应的）对吧？那么RPM包的发布时依赖关系，yum库是怎么知道的？
- RPM包打包的时候，spec 中file段下指定的是需要打包的文件，相对于build-root目录来说？也就是其实spec文件是不会打包到RPM包中的对吧？
- yum下载一个RPM包，其中的二进制文件安装到linux哪个系统目录下，默认是怎么抉择的？

## reference

- [Linux软件安装管理](https://www.jianshu.com/p/ee60a9d6bd7d)
- [Linux包管理基础：apt、yum、dnf 和 pkg](https://linux.cn/article-8782-1.html)
- [Linux软件安装中RPM与YUM 区别和联系](https://www.cnblogs.com/LiuChunfu/p/8052890.html)
- [CentOS的软件包的管理之rpm和yum](https://www.cnblogs.com/renpingsheng/p/7050418.html)
- [Linux 中 RPM 的构建与打包(深度推荐)](https://www.ibm.com/developerworks/cn/linux/l-lo-rpm-build-package/index.html)
- [源码制作RPM包](https://blog.csdn.net/u010749412/article/details/22993479)
- [rpmbuild案例](https://blog.csdn.net/u012373815/article/details/73257754)
- [SPEC文件](https://www.cnblogs.com/michael-xiang/p/10480809.html)
- [SPEC文件解析](https://blog.csdn.net/iamonlyme/article/details/53131105?utm_medium=distribute.pc_relevant_t0.none-task-blog-BlogCommendFromMachineLearnPai2-1.nonecase&depth_1-utm_source=distribute.pc_relevant_t0.none-task-blog-BlogCommendFromMachineLearnPai2-1.nonecase)
- [本机安装yum仓库](https://www.cnblogs.com/FengGeBlog/p/10230311.html)


