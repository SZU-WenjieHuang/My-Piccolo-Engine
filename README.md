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

### 17 重新编译命令
cmake -B build

### 18 Shader的处理以及绑定流程
所有的glsl Shader，都会在windows用Visul Studio编译的过程中生成对应的.h文件以及编译之后的spv文件，存储在 engine/shader/generated文件夹下;

### 19 撕裂的条纹的bug，是因为mipmap

在render_resource.cpp中修改:  传入一个color_grading 的mipmap level = 1; 就不会给它生成mipmap了。
why？

难道是默认=0是生成全部，然后=1是只生成最高精度那层？

```cpp
        // create color grading texture
        rhi->createGlobalImage(
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image,
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image_view,
            m_global_render_resource._color_grading_resource._color_grading_LUT_texture_image_allocation,
            color_grading_map->m_width,
            color_grading_map->m_height,
            color_grading_map->m_pixels,
            color_grading_map->m_format, 1);
```

### 20 Vulkan Subpass 架构
在Main Camera Pass里定义了很多的subPass；

### 21 关于解引用，引用，取址，指针

```cpp
int main() {

int num = 10;

// 取址(&):获取变量num的内存地址
int* pNum = &num;

// 解引用(*):根据指针pNum的值取得它指向的变量
int val = *pNum;

// 引用(&):新变量numRef直接引用已有变量num
int& numRef = num;

// 操作
num = 20;
cout << num << endl;       // 20
cout << val << endl;       // 10
cout << numRef << endl;    // 20

// 指针操作
*pNum = 30;
cout << num << endl;       // 30
cout << val << endl;       // 10
cout << numRef << endl;    // 30

// 引用操作
numRef = 40;
cout << num << endl;       // 40
cout << val << endl;       // 10
cout << numRef << endl;    // 40

return 0;
}
```
注意在这里，初始化 对象& 需要用一个对象，而不是用地址;

### 22 理解double buffering里的 odd 和 even的使用

### 23 override final
在main_camera_pass.h段落里有一些代码如下所示:

```cpp
        void initialize(const RenderPassInitInfo* init_info) override final;

        void preparePassData(std::shared_ptr<RenderResourceBase> render_resource) override final;
```

main_camera_pass继承自render_pass。override显式地表示main_camera_pass会重写基类里的函数，其实不加也可以。
final表示其不能被子类所重写。

### 24 接口的理解
是的，在C++中，一个类的public函数可以被称为该类对外的接口。public函数在类的定义中声明并定义，可以被类的对象或外部代码直接访问和调用。这些public函数定义了类与外部世界的交互方式，因此可以看作是该类对外的接口。

通过public函数，外部代码可以使用类的实例来执行特定的操作或获取信息，而无需了解类的内部实现细节。这种封装性和抽象性是面向对象编程的重要特性之一，它允许类的实现细节被隐藏，并提供了更好的模块化和代码复用性。

而private函数就只允许在类的内部使用，一般是被public函数调用。

### 25 枚举的作用
enum枚举的作用有很多，比如增加可读性和可维护性：枚举提供了一种直观的方式来表示一组相关的命名常量。通过使用枚举，可以用有意义的名称来标识不同的取值，使代码更易读、更易理解;

枚举常量的类型安全：枚举常量是静态类型的，编译器会在编译时进行类型检查。这意味着在使用枚举常量时，可以避免使用不正确的取值或类型错误，从而减少了潜在的错误。

在C和C++中，如果没有显式指定枚举的基础类型，默认情况下枚举的基础类型是int。要是没有特定的标明index，则就是默认从0开始递增;

比如以下这个enum:
```cpp
    enum
    {
        // attachments
        _main_camera_pass_gbuffer_a                     = 0,
        _main_camera_pass_gbuffer_b                     = 1,
        _main_camera_pass_gbuffer_c                     = 2,
        _main_camera_pass_backup_buffer_odd             = 3,
        _main_camera_pass_backup_buffer_even            = 4,
        _main_camera_pass_post_process_buffer_odd       = 5,
        _main_camera_pass_post_process_buffer_even      = 6,
        _main_camera_pass_depth                         = 7,
        _main_camera_pass_swap_chain_image              = 8,
        // attachment count
        _main_camera_pass_custom_attachment_count       = 5,
        _main_camera_pass_post_process_attachment_count = 2,
        _main_camera_pass_attachment_count              = 9,
    };
```

然后使用的话: </br>
```cpp
m_framebuffer.attachments[_main_camera_pass_gbuffer_a].format          = RHI_FORMAT_R8G8B8A8_UNORM;        // RGBA
```
相当于 
```cpp
m_framebuffer.attachments[0]
```
用==的话:</br>
```cpp
buffer_index == _main_camera_pass_gbuffer_a
```
相当于
```cpp
buffer_index == 0
```

### 26 命名空间内作用域
有时候我会好奇，在一个namespace内，但是在class外的数据，比如int和enum和struct，他们的生命周期是多少？</br>
这些算是全局静态变量，他们的生命周期与程序的生命周期相同，在程序启动时进行初始化，直到程序结束时被销毁。它们可以在命名空间内的任何位置访问，并且在命名空间外部可以通过限定名来访问。

举个例子， 比如在render_pass.h内: 在这里 这个enum和这个VisiableNodes的struct就是全局静态的。然后在Piccolo的namespace内都可见。
然后namesapce外访问的话需要这样: Piccolo::VisiableNodes。

```cpp
//......

namespace Piccolo
{
    // ...

    enum
    {
        // attachments
        _main_camera_pass_gbuffer_a                     = 0,
        _main_camera_pass_gbuffer_b                     = 1,
        _main_camera_pass_gbuffer_c                     = 2,
        _main_camera_pass_backup_buffer_odd             = 3,
        _main_camera_pass_backup_buffer_even            = 4,
        _main_camera_pass_post_process_buffer_odd       = 5,
        _main_camera_pass_post_process_buffer_even      = 6,
        _main_camera_pass_depth                         = 7,
        _main_camera_pass_swap_chain_image              = 8,
        // attachment count
        _main_camera_pass_custom_attachment_count       = 5,
        _main_camera_pass_post_process_attachment_count = 2,
        _main_camera_pass_attachment_count              = 9,
    };

    struct VisiableNodes
    {
        std::vector<RenderMeshNode>*              p_directional_light_visible_mesh_nodes {nullptr};   // directional_light 可见
        std::vector<RenderMeshNode>*              p_point_lights_visible_mesh_nodes {nullptr};        // point_lights 可见
        std::vector<RenderMeshNode>*              p_main_camera_visible_mesh_nodes {nullptr};         // main camera 可见
        RenderAxisNode*                           p_axis_node {nullptr};                              // 坐标轴节点
    };

    class RenderPass : public RenderPassBase
    {
        //......
    };
} // namespace Piccolo

```

### 27 图像布局image tilling
```cpp
    enum RHIImageTiling : int
    {
        RHI_IMAGE_TILING_OPTIMAL = 0,
        RHI_IMAGE_TILING_LINEAR = 1,
        RHI_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT = 1000158000,
        RHI_IMAGE_TILING_MAX_ENUM = 0x7FFFFFFF
    };
```

1-最优布局方式 (RHI_IMAGE_TILING_OPTIMAL)：</br>
当图像用于渲染操作（如颜色附件、深度附件等）时，通常选择最优布局方式。这种布局方式对 GPU 访问进行了优化，提供了最佳的性能和效率。
当需要对图像进行采样操作（如纹理采样）时，最优布局方式通常也是首选，因为它可以提供高效的纹理采样性能。

2-线性布局方式 (RHI_IMAGE_TILING_LINEAR)：</br>
当需要对图像进行直接读写操作时，可以选择线性布局方式。例如，当 CPU 需要直接更新图像数据或进行像素级别的修改时，线性布局方式可能更为适合。
线性布局方式可能不会提供与最优布局方式相同的性能和优化，因此在不需要直接读写操作时，通常不推荐使用线性布局方式。

3-DRM 格式修饰符扩展布局方式 (RHI_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT)：</br>
当使用了 DRM 格式修饰符扩展定义的特定布局方式时，可以选择该布局方式。
DRM 格式修饰符扩展提供了对图像布局和属性的更灵活描述，可以满足特定的硬件要求或优化需求。

4-RHI_IMAGE_TILING_MAX_ENUM 用的不多

### 28 rhi! Rendering High-level Interface (Vulkan_rhi.h)
其实RHI文件是一个对渲染API的高级抽象，它封装了很多Vulkan底层的函数，然后整合成高级的接口；通过rhi文件我们可以不直接使用VUlkanAPI，而是使用其
更高层级的接口。同时我们也可以只替换底层而不影响上层。</br>

一般来说Rhi的作用是:</br>
1-将底层API(如Vulkan)封装成更高层次的接口</br>
2-隐藏底层API的具体实现细节</br>
3-提供跨平台的渲染功能接口</br>

这个vulkan_rhi.h文件主要做了以下工作:

定义了一些Vulkan对象(设备、队列等)相关的数据结构

定义各种渲染资源(图像、缓冲区等)与命令的创建/更新接口

定义图元绘制、渲染目标绑定等渲染流水线相关接口

使用C++类和函数的形式对Vulkan进行了抽象封装

隐藏底层Vulkan具体实现,提供跨平台接口。

### 29 VUlkanUtil
VulkanUtil这个类看来是封装一些Vulkan系统层面的实用函数,目的是为Vulkan RHI实现提供支持。

主要功能有:
1-封装Vulkan底层对象的创建/删除接口,如 sampler、buffer、image 等。
2-提供资源格式转换、内存管理、图片生成等常用Utility函数。
3-缓存和管理特定sampler对象,避免重复创建。
4-封装图片格式转换和内存拷贝接口。
5-这些函数都是Vulkan RHI实现需要频繁调用的底层操作。

将它们提取出来作为Utility类,有以下优点:
1-隐藏Vulkan API细节,简化RHI代码
2-将非RHI逻辑移出RHI类,提高鲁棒性和可维护性
3-方便集中管理相关Vulkan对象(sampler等)
4-提高重复利用率,例如缓存sampler对象

所以总之,VulkanUtil类是:
1-为Vulkan RHI实现提供底层辅助功能
2-简化和优化Vulkan相关代码
3-提升Vulkan RHI代码质量和可维护性

### 30 底层对象和高级对象 
在Piccolo小引擎的架构内，并不会直接使用底层的 Vulkan对象，比如VkImage和VkDeviceMemory之类的，而是会做一定的封装。

rhi_struct.h 在这个头文件内定义了支持所有API的一些对象接口，然后vulkan_rhi_resource.h继承这个rhi_struct.h文件，并且对Vulkan对象做一定的封装。

比如以下的类其实是把vulkan底层的VkImage对象封装到了我们自定义的VulkanImage类里面，并只是增加来了一个get和一个set的方法:
```cpp
    class VulkanImage : public RHIImage
    {
    public:
        void setResource(VkImage res)
        {
            m_resource = res;
        }
        VkImage &getResource()
        {
            return m_resource;
        }
    private:
        VkImage m_resource;
    };
```
好的 再画一下导图，相信对于Vulkan再小引擎内的结构，已经游刃有余了。

### 31 Render_type  RHI bits 和 Vulkan bits间的映射
举个简单的例子，Image View里的image_aspect_flags; 

这是RHI里枚举的:
```cpp
    enum RHIImageAspectFlagBits {
        RHI_IMAGE_ASPECT_COLOR_BIT = 0x00000001,
        RHI_IMAGE_ASPECT_DEPTH_BIT = 0x00000002,
        RHI_IMAGE_ASPECT_STENCIL_BIT = 0x00000004,
        RHI_IMAGE_ASPECT_METADATA_BIT = 0x00000008,
        RHI_IMAGE_ASPECT_PLANE_0_BIT = 0x00000010,
        RHI_IMAGE_ASPECT_PLANE_1_BIT = 0x00000020,
        RHI_IMAGE_ASPECT_PLANE_2_BIT = 0x00000040,
        RHI_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT = 0x00000080,
        RHI_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT = 0x00000100,
        RHI_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT = 0x00000200,
        RHI_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT = 0x00000400,
        RHI_IMAGE_ASPECT_PLANE_0_BIT_KHR = RHI_IMAGE_ASPECT_PLANE_0_BIT,
        RHI_IMAGE_ASPECT_PLANE_1_BIT_KHR = RHI_IMAGE_ASPECT_PLANE_1_BIT,
        RHI_IMAGE_ASPECT_PLANE_2_BIT_KHR = RHI_IMAGE_ASPECT_PLANE_2_BIT,
        RHI_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
```
然后这是Vulkan原本的VkImageAspectFlagBits

```cpp
VK_IMAGE_ASPECT_COLOR_BIT //表示图像的颜色方面。
VK_IMAGE_ASPECT_DEPTH_BIT // 表示图像的深度方面。
VK_IMAGE_ASPECT_STENCIL_BIT //表示图像的模板方面。
VK_IMAGE_ASPECT_METADATA_BIT //表示图像的元数据方面。
```
他们开的就是后面这个十六进制数映射上的，枚举 RHIImageAspectFlagBits 中，RHI_IMAGE_ASPECT_COLOR_BIT 被赋予了值 0x00000001，而在 Vulkan 中，VK_IMAGE_ASPECT_COLOR_BIT 也被赋予了相同的值 0x00000001。这样，通过使用相同的值，可以将 RHI_IMAGE_ASPECT_COLOR_BIT 和 VK_IMAGE_ASPECT_COLOR_BIT 进行关联。

在具体的图形渲染接口实现中，例如使用 Vulkan API，当你需要指定图像的颜色方面时，你可以使用 RHI_IMAGE_ASPECT_COLOR_BIT。然后，实现会将其映射为相应的 Vulkan 图像方面标识 VK_IMAGE_ASPECT_COLOR_BIT，以确保正确的图像访问和操作。


### 32 十六进制表示法
在计算机编程中，十六进制表示法使用前缀 0x 来标识一个数值是以十六进制形式表示的。后面的数字部分则是十六进制数的具体表示。

例如，在 0x00000001 中，0x 是十六进制标识符，表示后面的数字是以十六进制表示的。而 00000001 则是具体的十六进制数，对应于十进制数 1。

同样地，在 0xFF00 中，0x 是十六进制标识符，而 FF00 是具体的十六进制数，对应于十进制数 65280。

因此，你可以将十六进制表示法理解为由 0x 标识符和后面的数字组成的形式，其中数字部分表示一个十六进制数。

更多的例子:

0x0A：这个表示中，x 的值是 A，因此 0x0A 表示十六进制数 0A，相当于十进制数 10。

0x1F：这个表示中，x 的值是 F，因此 0x1F 表示十六进制数 1F，相当于十进制数 31。

0xFF00：这个表示中，x 的值是 F，因此 0xFF00 表示十六进制数 FF00，相当于十进制数 65280。

0x12345678：这个表示中，x 的值是不确定的，因为它不在有效的十六进制数位范围内。它只是用作示例，表示一个十六进制数。

done!