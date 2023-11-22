# BugNotes

记录在学习Piccolo Engine过程中遇到的bug与进步方向。

### 01 编译链接库的问题

错误 LNK1104 无法打开文件“..\..\3rdparty\JoltPhysics\Build\Debug\Jolt.lib” PiccoloEditor D:\Piccolo-main\build\engine\source\editor\LINK 1

解决办法:</br>
因为JoltPhysics这个项目开启了“将警告视为错误”，而其编译时又确实有几个警告，于是这个项目出现了错误，编译失败，因此也就没有生成Jolt.lib文件，链接时也就找不到这个文件。
你可以在解决方案管理器内找到ThirdParty/JoltPhysics/Jolt选中它，然后点击上方项目菜单，点击属性，打开Jolt属性页，左侧配置属性栏选择C/C++，右侧找到“将警告视为错误”项，把它设为否就可以了。

解决完这个bug之后，即可正常编译和运行Piccolo小引擎;

### 02 文件的主要代码和入口

engine/source/runtime</br>
是主要的代码和文件入口，底下包含core/function/platform/resource四层内容。

我们主要关注的结构是runtime/core, 即核心层的代码。

### 03 用SourceTrail追踪整个项目

使用SourceTrial追踪项目首先需要有一个compile_commands.json;

其中有包含了编译信息，我们需要把这个文件提供给SourceTrail来创建项目;</br>

![](/Images/1.png)

如图, 绿色的文件，红色是namespaces，黄色是函数，蓝色是全局变量;

### 04 engine.h 和 engine.cpp
这里是整个引擎的入口, 主要干了两件事:</br>
1-在engine开启和关闭的时候开启和关闭 runtime_global_context的System</br>
2-在run()函数里每一帧tick一下，分为logicalTick()和renderTick(), 因为Piccolo是逻辑和渲染分离的架构</br>

LogicalTick主要是Tick了world和input,主要是World；</br>

***World->Level->GameObject->GameObject挂载的components(经典的component设计模式)***</br>

其中Level有很多，但一次只有一个活跃的Level。而且GameObject并没有实际的功能，功能都是在Component里。这个
Tick的逻辑链是引擎架构的核心。

![Alt text](/Images/2.png)

### 05 global_context.cpp 和 global_contex.h
RuntimeGlobalContext管理所有全局系统的创建和销毁顺序。</br>
startSystems方法负责初始化各种系统,如场景、物理、渲染等。它们被作为共享指针保存。</br>
shutdownSystems方法负责关闭并清理各系统。</br>

它使用了单例模式管理所有系统,并采用依赖关系初始化和关闭顺序,形成一个完整的游戏服务提供链,
给其他模块提供方便易用的接口。这是一种常见的系统封装方式。</br>
具体的方法是: startSystem() 用shared_ptr提供了全局唯一入口, 用shared_ptr不用unique_ptr主要是为了方便多个组件间共享，也避免了内存管理。

在这里的变量，g开头的就是全局变量(global)，m开头的就是成员变量(member)</br>
