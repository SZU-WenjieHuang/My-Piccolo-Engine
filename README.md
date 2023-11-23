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

### 06 使用Json进行序列化和反序列化

Q1 首先，什么是序列化？ 什么是反序列化？</br>

序列化:</br>
将对象转换成二进制数据格式的过程。常见的有JSON、XML等文本格式,以及二进制格式比如Protobuf。</br>

这个过程:</br>
1-会按照规则将对象各个字段和属性的值抽取出来。</br>
2-按照约定的格式组织这些值,加入标识信息。</br>
3-产生可以序列化的二进制数据。</br>

反序列化:</br>
将序列化数据转换回对象的过程。</br>

这个过程:</br>
1-解析二进制数据,获取各字段的值和对象类型信息。</br>
2-根据类型信息重新创建对象。</br>
3-将字段值还原到对象对应属性上。</br>
4-重构完成原始对象。</br>

Q2 有什么序列化后的数据格式</br>
JSON;</br>

Q3 为什么要把对象序列化到Json，然后再把Json反序列化回对象</br>
1-易于持久化存储。序列化为JSON后,对象 State可以很方便地存储在文件或数据库中,实现对象状态的持久化。</br>
2-方便传输。序列化后的JSON数据是自描述和轻量级的,很适合通过网络传输。比如不同系统之间的远程对象通信。</br>
3-语言和平台无关。JSON为纯文本格式,任何语言和平台都可以解析,实现系统间交互和数据交换。</br>
4-人可读。相比二进制格式,JSON更适合人眼阅读和调试。</br>
5-高效。相比像XML等格式,JSON数据体积更小,解析速度更快。</br>

### 07 在Piccolo Engine上添加Lua脚本，使其挂载到小白人角色身上

Q1 什么是引擎里的脚本系统</br>
游戏引擎中的脚本系统主要用来实现一些游戏规则和行为逻辑;</br>

脚本独立于引擎源码,可以在运行时生效且不需要重启引擎。</br>
使用脚本语言如Lua描述游戏规则、行为逻辑等可配置元件。</br>
可以方便地定制并升级游戏内容而无需修改引擎代码。</br>
引擎提供脚本接口供脚本调用,实现数据交互和对游戏场景的操作。</br>
常见的脚本内容包括NPC行为、任务逻辑、 UI交互等游戏核心功能。</br>
也可用于配置部件参数、游戏调试和故障排除。</br>
常用脚本语言是Lua,由于其轻量和与C/C++良好集成,适合嵌入游戏开发。</br>
引擎需要提供加载、编译、执行脚本的脚本运行时支持。</br>

所以总体来说,脚本系统就是一个动态扩展引擎功能的工具,开发者通过脚本定制和智能化游戏规则逻辑,而不需要修改底层引擎代码,从而实现更快的迭代开发。</br>

Q2 什么是Lua</br>
Lua是一种轻量级的语言</br>
1-轻量级:Lua解释器体积小,只有约150KB,嵌入效率高。</br>
2-基于注册的:提供C函数注册表供C/C++调用Lua函数,方便嵌入应用。</br>
3-优雅的语法:基于C语法但使用设计良好的语法,入门难度低。</br>

Q3 Lua脚本挂载到小白人角色身上的步骤


### 08 小引擎使用全局变量 g_runtime_global_context,可以在任意地方方便地访问各个系统，这种方法有什么优缺点，有没有其他方式可以实现同样的目的？

### 09 Object上不同类型的Component的调用顺序是不确定的, 比如一个Player可能首先跑脚本再跑动画，另一个Player可能会先动画再脚本，这会给游戏的运行带来什么问题？

### 10 在这个例子里只实现了Lua脚本的简单执行，目前这个脚本只能简单执行并且每一帧执行一次，还不能调用任何引擎的接口，我们要这么设计Lua脚本的生命周期，以及与引擎各个系统的交互方式？
