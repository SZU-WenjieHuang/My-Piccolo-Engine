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

Q3 Lua脚本挂载到小白人角色身上的步骤</br>
Step1: 下载Lua库和sol2库，sol2库可以帮我们非常好的在C++里执行Lua脚本;</br>

Step2: 将他们放在engine/3rdparty文件夹下，并开始构建(编写CMake脚本，并在CmakeList的文件中添加Lua和sol2)；

step3：把这两个库连接到Piccolo runtime上，在runtime文件夹下的CMakeList内添加Lua和Sol2，并编译。

step4: 把lua加入到component里。在runtime/function/framework/component里加入一个lua文件夹，添加.h和.cpp。是最简单的形式

step5：小引擎的world是从一个json资产内加载的，在assert/world/内有一个 hello.world.json。 顺着这个Json文件，我们可以找到更多的Json文件，比如level.json, 然后还能顺藤摸瓜找到在objects/character\player底下的player.object.json。 里面就描述了小白人Player所挂载的各种component。在这个json里的就是需要被反序列化的字段。在里面添加Lua;

step6：脚本的执行都在LuaComponent::tick(float delta_time)里面；

step7：用sol库把lua脚本跑起来，然后再重新编译；


### 08 小引擎使用全局变量 g_runtime_global_context,可以在任意地方方便地访问各个系统，这种方法有什么优缺点，有没有其他方式可以实现同样的目的？
answer in bilibili:</br>
点就是方便，随处可以访问所有系统；缺点是某种意义上来说破坏了程序设计应该有的高内聚低耦合特性——我们把全局变量侵入到了一些类中。</br>
解决方法：我觉得可以再思考一下类架构，想办法消融掉这个全局变量；

answer by GPT：</br>
优点:</br>
1-简单直观,任何位置都可以直接访问系统上下文,不需要进行参数传递。</br>
2-避免重复建立和传递上下文对象,减少代码量。</br>

缺点:</br>
1-增加全局依赖,破坏了模块与系统的隔离与解耦。</br>
2-难以追踪依赖,给代码维护和扩展带来困难。</br>
3-全局变量可能被任意位置修改值,难以控制并发访问,影响线程安全。</br>

其他替代方法:</br>
使用服务定位器模式,通过单例方式获取系统上下文。</br>

### 09 Object上不同类型的Component的调用顺序是不确定的, 比如一个Player可能首先跑脚本再跑动画，另一个Player可能会先动画再脚本，这会给游戏的运行带来什么问题？

answer in bilibili:</br>
不同component可能存在依赖关系，比如我们写脚本让动画发生变化，那么动画要在脚本后调用</br>
解决方案：可以显式的给定component的顺序，方法就是通过json中的顺序来确定，这样就可以通过修改json中不同component的相对位置来调整component顺序

answer by GPT：</br>
解决办法是明确规定每个Object上各Component的调用顺序优先级,通过标记或编号来精确控制顺序,避免顺序不确定导致的问题。或者采用事件驱动的顺序来回避这类问题。


### 10 在这个例子里只实现了Lua脚本的简单执行，目前这个脚本只能简单执行并且每一帧执行一次，还不能调用任何引擎的接口，我们要这么设计Lua脚本的生命周期，以及与引擎各个系统的交互方式？

answer in bilibili:</br>
可以参考一下unity的c#脚本设计方法，允许脚本通过反射来访问各个GO与组件


### 11 CMake，为什么每一个主要文件夹，如runtime和shader的文件夹下都会有自己的CMakeList.txt文件，这些txt有什么用？

构建在软件工程中是一个广泛使用的术语。构建通常是指编译源代码以及相关的链接阶段,生成最终的可执行文件或库文件。就是在我们编译过程中，编译生成目标代码，和动态或者静态连接库文件成为可执行文件exe的过程。

CMakeLists.txt描述了项目各模块的构建信息和规则,其中包含目标文件，源文件，和需要连接的libraries等资料。

在每一个重要目录下都有一个CMakeList.txt是为了更好的实现各个模块之间的解耦。

### 12 反射系统的目的

此处可以参见课程的笔记:
https://github.com/SZU-WenjieHuang/Markdown-note/blob/main/Games104/14-%E5%BC%95%E6%93%8E%E5%B7%A5%E5%85%B7%E9%93%BE%E9%AB%98%E7%BA%A7%E6%A6%82%E5%BF%B5.md

1-动态创建对象实例:</br>
通过类名称或类型信息动态地创建对象,而不需要预先给定具体类。

2-动态调用成员:</br>
运行时解析并调用类的成员函数或属性,而不需要在代码里直接硬编码。

3-插件热更新:</br>
允许动态加载插件代码,并通过反射使用插件提供的接口而无需重新编译主程序。

4-脚本绑定:</br>
允许脚本语言通过反射调用C++类的成员,实现C++与脚本深度融合。

5-传感数据绑定:</br>
通过反射实时解析传感数据结构,实现数据绑定到脚本或UI组件。

6-编辑器自定义组件:</br>
允许通过JSON等定义组件,根据定义构建相应类或对象实例。

### 13 VScode检索函数调用小技巧
按住shift+F12, 就可以在VScode里检索到所有调用该函数的地方。当然要是键盘上的F12被一些比如计算器之类的快捷功能占领，则需要多按一下Fn才行。


### 14 渲染系统

小引擎的渲染系统叫render system; 在source/runtime/function/render 文件夹下有 render_system.h 和 render_system.cpp; 是渲染系统的入口。</br>

render system在全局的Global_context.cpp里被初始化，然后在engine.cpp里被每一帧都tick()一次。最后在 Global_context.cpp里的shutdownSystems()函数里被clear()。</br>

在renderSystem.cpp内有初始化了Camera，Scene，Resources， renderPipeline等Class；

在我们的 render_pipeline.cpp里面，包括了forward Render和deffered Render两个内容。然后还有初始定义的10种Pass:

1-PointLightShadowPass</br>
2-DirectionalLightShadowPass</br>
3-MainCameraPass</br>
4-ToneMappingPass</br>
5-ColorGradingPass</br>
6-UIPass</br>
7-CombineUIPass</br>
8-PickPass</br>
9-FXAAPass</br>
10-ParticlePass</br>

在Tick()函数的入口处就是一个processSwapData()函数，用的是“双缓冲”(double buffering)模式，用来做渲染数据的交换，包括level_resource, game_object_resource,camera_swap_data等对象信息。剩下的步骤可以看代码。</br>

总结一下渲染的步骤就是:
在renderSystem内的tick() 是整个渲染的入口，首先渲染会从逻辑获取数据，然后准备一堆前置数据之后，开始走Vulkan管线渲染。走到 render_pipeline.cpp里实现forward_rendering和deffered_rendering, 然后又在main_camera_pass.cpp内具体地去draw每一个Pass。这又回到了熟悉的Vulkan，VulkanYYDS。</br>

以上就是渲染系统的框架。


### 15 RHI
RHI是渲染硬件接口(Render Hardware Interface)的缩写。

RHI在游戏引擎中的作用是:
1-它隐藏了底层图形API(如DirectX、OpenGL等)的具体实现细节。
2-提供一个渲染抽象层,标准化各个图形API框架下的开发流程。
3-使渲染系统与具体的硬件平台解偶并支持多平台。

具体来说,RHI主要 responsibility有:
1-定义图形资源的抽象类型,如纹理、Shader等。
2-定义渲染命令列表和绘图调用接口。
3-管理显存,加载/释放资源。
4-提供采样器状态等辅助功能。
5-封装窗口/设备创建等基础功能。
6-同步渲染线程和游戏逻辑。

### 16 Color Grading
Color Grading在渲染管线中主要用来对渲染结果进行着色处理。</br>
我的理解是，Color Grading把原本一个色彩空间的颜色给转变到另一个已有的色彩空间里。</br>
