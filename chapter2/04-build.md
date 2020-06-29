# 组件构建

## 基于包构建的交叉工具链选择


RPM，自研工具：

根据arch设置环境变量：CC，CFLAGS...
根据build requires，下载依赖，然后设置对应的头文件到环境变量 -I中；

CONAN：

根据参数生成cmake文件，通过include该文件，然后使用其中的变量。
怎么选择工具链？