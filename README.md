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

### 33 Struct的初始化
像这样的结构体:
```cpp
struct RHIAttachmentReference
{
    uint32_t attachment;
    RHIImageLayout layout;
};
```
可以这样子初始化:
```cpp
RHIAttachmentReference color_grading_pass_input_attachment_reference {};
```
并不需要加struct的关键字

### 34 {} 内嵌作用域

看代码的时候会发现一种情况: 会有一些没有title的作用域如下:

```cpp
    void MainCameraPass::setupDescriptorSetLayout()
    {
        m_descriptor_infos.resize(_layout_type_count);   // 7 种不同的Layout

        {
            RHIDescriptorSetLayoutBinding mesh_mesh_layout_bindings[1];

            RHIDescriptorSetLayoutBinding& mesh_mesh_layout_uniform_buffer_binding = mesh_mesh_layout_bindings[0];
            mesh_mesh_layout_uniform_buffer_binding.binding                       = 0;
            mesh_mesh_layout_uniform_buffer_binding.descriptorType                = RHI_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            mesh_mesh_layout_uniform_buffer_binding.descriptorCount               = 1;
            mesh_mesh_layout_uniform_buffer_binding.stageFlags                    = RHI_SHADER_STAGE_VERTEX_BIT;
            mesh_mesh_layout_uniform_buffer_binding.pImmutableSamplers            = NULL;

            RHIDescriptorSetLayoutCreateInfo mesh_mesh_layout_create_info {};
            mesh_mesh_layout_create_info.sType        = RHI_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            mesh_mesh_layout_create_info.bindingCount = 1;
            mesh_mesh_layout_create_info.pBindings    = mesh_mesh_layout_bindings;

            if (m_rhi->createDescriptorSetLayout(&mesh_mesh_layout_create_info, m_descriptor_infos[_per_mesh].layout) != RHI_SUCCESS)
            {
                throw std::runtime_error("create mesh mesh layout");
            }
        }

        // ...
    }
```

这类作用域其实是内嵌作用域，起到的作用是变量只在这个内嵌块中有效,外部无效，相当于给变量"包裹"起来,限定其作用范围。

### 35 一个pipelineLayout包含多个DescriptorLayout的情况
假设我们要渲染一个3D场景,包含:</br>
Environment texture</br>
Model texture</br>
Framebuffer object (FBO)</br>

我们可以定义:</br>
Descriptor Set Layout 1:</br>
Bind the environment texture</br>

Descriptor Set Layout 2:</br>
Bind the model texture</br>

Descriptor Set Layout 3:</br>
Bind the FBO</br>

然后定义一个Pipeline Layout包含他们三个</br>
Pipeline Layout:</br>
Bind Descriptor Set Layout 1</br>
Bind Descriptor Set Layout 2</br>
Bind Descriptor Set Layout 3</br>

这样的好处是我们可以使用不同的descriptorSet来管理不同的resources</br>

### 36 shader的处理逻辑

01 GLSL文件(.vert / .frag)</br>
最开始的就是我们编写的shader，以GLSL的形式；

02 SPIR-V文件(.spv)</br>
在编译阶段，通过 ./cmake/ShaderCompile.cmake 将 .vert 和 .frag文件
编译成 .spv文件。讲道理在这个阶段vulkan就可以直接读取使用了，但我们希望减少
文件加载的开销。

03 CPP的vector(.h)</br>
使用GenerateShaderCPPFile.cmake脚本将SPIR-V文件转换为C++源代码，并将其存储在一个std::vector<unsigned char>变量中。

这样做的好处是，在运行时，你可以直接使用生成的C++源代码中的std::vector<unsigned char>变量来加载和访问着色器代码，而无需在运行时加载和解析原始的GLSL或SPIR-V文件。

通过将着色器代码转换为C++源代码并嵌入在可执行文件中，可以减少运行时的文件加载和解析时间，并且使着色器代码更加独立和可移植。

阅读这两个.cmake 以理解cmake文件的运行过程与代码逻辑;

### 37 Vertex Input的 Binding 和 Attribute的关系

我们在准备 pipeline的时候需要设置vertex input的信息:
```cpp
auto vertex_binding_descriptions   = MeshVertex::getBindingDescriptions();
auto vertex_attribute_descriptions = MeshVertex::getAttributeDescriptions();
```

实际上这就是类似OpenGL里的VAO去描述顶点的信息和属性。

binding、position 和 attribute 之间的关系如下：(一个Binding内可以包括多个Position)

Binding 描述了一组顶点属性的布局和访问方式。</br>
Position 是顶点数据结构中的一个属性，通常用于表示顶点的位置信息。</br>
Attribute 描述了顶点数据的其他属性，如法线、颜色、纹理坐标等。</br>

在以下的代码里设置: render_mesh.h

```cpp
// input vertex 的 binding
static std::array<RHIVertexInputBindingDescription, 3> getBindingDescriptions()
{
    std::array<RHIVertexInputBindingDescription, 3> binding_descriptions {};

    // position
    binding_descriptions[0].binding   = 0;
    binding_descriptions[0].stride    = sizeof(VulkanMeshVertexPostition);
    binding_descriptions[0].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
    // varying blending  
    binding_descriptions[1].binding   = 1;
    binding_descriptions[1].stride    = sizeof(VulkanMeshVertexVaryingEnableBlending);
    binding_descriptions[1].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
    // varying
    binding_descriptions[2].binding   = 2;
    binding_descriptions[2].stride    = sizeof(VulkanMeshVertexVarying);
    binding_descriptions[2].inputRate = RHI_VERTEX_INPUT_RATE_VERTEX;
    return binding_descriptions;
}

// vertex input 的 attribute
static std::array<RHIVertexInputAttributeDescription, 4> getAttributeDescriptions()
{
    std::array<RHIVertexInputAttributeDescription, 4> attribute_descriptions {};

    // position
    attribute_descriptions[0].binding  = 0;
    attribute_descriptions[0].location = 0;
    attribute_descriptions[0].format   = RHI_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[0].offset   = offsetof(VulkanMeshVertexPostition, position);

    // varying blending
    attribute_descriptions[1].binding  = 1;
    attribute_descriptions[1].location = 1;
    attribute_descriptions[1].format   = RHI_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[1].offset   = offsetof(VulkanMeshVertexVaryingEnableBlending, normal);
    attribute_descriptions[2].binding  = 1;
    attribute_descriptions[2].location = 2;
    attribute_descriptions[2].format   = RHI_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions[2].offset   = offsetof(VulkanMeshVertexVaryingEnableBlending, tangent);

    // varying
    attribute_descriptions[3].binding  = 2;
    attribute_descriptions[3].location = 3;
    attribute_descriptions[3].format   = RHI_FORMAT_R32G32_SFLOAT;
    attribute_descriptions[3].offset   = offsetof(VulkanMeshVertexVarying, texcoord);

    return attribute_descriptions;
}
```

为什么三个bingding 对应四个Position? 因为可能处于数据格式的要求，访问格式的需要求。 比如在这里 texcoord的数据格式就和Normal/tangent不同。</br>
可以把binding理解成position的一个组。

### 37 可重用的DescriptorSet设计

### 38 render resource xmind

### 39 assert
assert 是一个在程序中用于进行断言检查的宏。它的作用是在运行时检查一个条件是否为真，如果条件为假，则触发断言错误，并终止程序执行。断言用于在开发和调试阶段对程序的假设进行验证，以确保程序的正确性和稳定性。

assert 宏的基本语法如下：
```cpp
#include <cassert>

assert(condition);
```

condition 是一个布尔表达式，表示要进行断言检查的条件。如果 condition 为假（即 false），则 assert 宏会触发断言错误。

以下是一些使用 assert 的示例：

```cpp
#include <cassert>

void divide(int a, int b) {
    assert(b != 0);  // 确保除数不为零
    int result = a / b;
    // ...
}

void processArray(int* array, int size) {
    assert(array != nullptr);  // 确保数组不为空指针
    assert(size > 0);  // 确保数组大小大于0
    // ...
}

int main() {
    int x = 5;
    assert(x > 0);  // 确保 x 大于0

    divide(10, 2);
    divide(10, 0);  // 触发断言错误，除数为零

    int arr[] = {1, 2, 3};
    processArray(arr, 3);
    processArray(nullptr, 0);  // 触发断言错误，空指针和大小为0

    return 0;
}
```

断言通常在调试阶段启用，而在发布版本中被禁用。在发布版本中，断言语句会被编译器移除，以提高程序的性能。因此，断言的使用主要是为了帮助开发者在开发和调试过程中及时发现问题，并不是程序中必须要有的一部分。

### 40 C++ 中的-> 和 . 的区别

一句话概括: -> 用于指针访问成员对象和成员函数，不需要解引用；

(1) . 运算符：用于直接访问对象的成员，适用于对象本身或对象引用。

(2) -> 运算符：用于通过指针访问对象的成员，适用于指向对象的指针。

1-对象类型：. 运算符适用于对象本身或对象引用，而 -> 运算符适用于指向对象的指针。</br>
2-操作符优先级：-> 运算符的优先级比 . 运算符的优先级高。因此，在表达式中，-> 运算符的执行顺序优先于 . 运算符。</br>
3-操作数：. 运算符的左操作数是一个具体的对象或对象引用，右操作数是成员名。而 -> 运算符的左操作数是一个指向对象的指针，右操作数是成员名。</br>
4-解引用：使用 . 运算符时，我们可以直接访问对象的成员，而不需要解引用指针。而使用 -> 运算符时，必须先对指针进行解引用，然后才能访问对象的成员。</br>

以下是一个demo:

```cpp
#include <iostream>

class MyClass {
public:
    int value;

    void print() {
        std::cout << "Value: " << value << std::endl;
    }
};

int main() {
    MyClass obj;
    obj.value = 42;

    MyClass* ptr = &obj;  // 创建指针 obj取址

    // 使用 . 运算符访问对象的成员
    std::cout << "Value (obj): " << obj.value << std::endl;
    obj.print();

    // 使用 -> 运算符访问指针所指向的对象的成员
    std::cout << "Value (ptr): " << ptr->value << std::endl;
    ptr->print();

    return 0;
}
```

### 41 Vulkan中定义一个image resource需要的四个对象

1-***Image***: 它描述了图像的格式、尺寸、使用方式等属性。
sType：指定结构体类型，通常为 VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO。</br>
pNext：指向附加的创建信息，通常为空。</br>
flags：指定图像的标志位，如是否支持立方体贴图、是否支持多级渐远纹理等。</br>
imageType：指定图像的类型，如 1D、2D 或 3D。</br>
format：指定图像的像素格式。</br>
extent：指定图像的尺寸，包括宽度、高度和深度。</br>
mipLevels：指定图像的多级渐远纹理级别数量。</br>
arrayLayers：指定图像的数组层数量。</br>
samples：指定图像的抗锯齿采样数量。</br>
tiling：指定图像的布局方式，如线性布局或渐远纹理布局。</br>
usage：指定图像的使用方式，如颜色附件、深度/模板附件、采样等。</br>
sharingMode：指定图像的共享方式，如在多个队列之间共享。</br>
queueFamilyIndexCount 和 pQueueFamilyIndices：指定共享图像的队列族索引。</br>
initialLayout：指定图像的初始布局</br>

2-ImageView: Image的一个视图，它定义了对 Image 数据的访问方式。ImageView 可以指定图像的特定部分、特定格式等信息，以便在着色器中对图像进行采样或读写操作。</br>
sType：指定结构体类型，通常为 VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO。</br>
pNext：指向附加的创建信息，通常为空。</br>
flags：指定图像视图的标志位，通常为 0。</br>
image：指定图像对象的句柄。</br>
viewType：指定图像视图的类型，如 1D、2D 或 3D。</br>
format：指定图像视图的像素格式，通常与图像的格式相同。</br>
components：指定图像视图的色彩通道映射关系。</br>
subresourceRange：指定图像视图的子资源范围，包括基本数组层级、层级数量、基本 mip 级别和 mip 级别数量。</br>

3-Sampler: Sampler 定义了在着色器中对图像进行采样的方式。它包含了采样过滤器、边界模式、向量规范化等参数，用于控制如何从纹理图像中获取像素值。</br>
sType：指定结构体类型，通常为 VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO。</br>
pNext：指向附加的创建信息，通常为空。</br>
flags：指定采样器的标志位，通常为 0。</br>
magFilter 和 minFilter：指定放大和缩小时的采样过滤器。</br>
mipmapMode：指定多级渐远纹理的采样模式。</br>
addressModeU、addressModeV 和 addressModeW：指定纹理坐标超出边界时的处理方式。</br>
mipLodBias：指定 mip 级别的偏移量。</br>
anisotropyEnable 和 maxAnisotropy：指定是否启用各向异性过滤以及最大各向异性采样率。</br>
compareEnable 和 compareOp：指定是否启用深度比较和深度比较操作。</br>
minLod 和 maxLod：指定 mip 级别的范围。</br>
borderColor：指定纹理边界的颜色。</br>
unnormalizedCoordinates：指定纹理坐标是否为非规范化坐标。</br>

4-Allocation: 是指为图像分配的内存。</br>
sType：指定结构体类型，通常为 VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO。</br>
pNext：指向附加的创建信息，通常为空。</br>
allocationSize：指定要分配的内存大小。</br>
memoryTypeIndex：指定内存类型的索引，用于选择合适的内存堆。</br>

### 42 Push Event 和 Pop Event

在 Vulkan 中，pushEvent 函数中的调试工具标签主要用于在 RenderDoc 中创建一个标记，以便在捕获的渲染帧中标记特定的事件或代码块。这些事件标签可以帮助开发人员在 RenderDoc 中更好地理解和分析渲染过程中的不同阶段。

对应的函数是: 
```cpp
    void VulkanRHI::pushEvent(RHICommandBuffer* commond_buffer, const char* name, const float* color)
    {
        if (m_enable_debug_utils_label)
        {
            VkDebugUtilsLabelEXT label_info;
            label_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
            label_info.pNext = nullptr;
            label_info.pLabelName = name;
            for (int i = 0; i < 4; ++i)
                label_info.color[i] = color[i];
            _vkCmdBeginDebugUtilsLabelEXT(((VulkanCommandBuffer*)commond_buffer)->getResource(), &label_info);
        }
    }
```
传入的就是renderDoc里的color；

那 PopEvent呢，就是为了保证这个调试的事件可以正确的闭合，比如下面的代码就是一个正确的追踪循环:

```cpp
    m_rhi->pushEvent(m_rhi->getCurrentCommandBuffer(), "BasePass", color);

    drawMeshGbuffer();

    m_rhi->popEvent(m_rhi->getCurrentCommandBuffer());
```

### 43 vkCmdNextSubpass
意义是切换到下一个subpass，函数的原型如下:
```cpp
void vkCmdNextSubpass(
    VkCommandBuffer commandBuffer,
    VkSubpassContents contents);
```

### 44 descriptorset 的数量
在渲染过程中，通常情况下，每个材质（material）和每个网格（mesh）和每个drawcall都需要绑定一个描述符集（descriptor set）。

### 45 Dynamic offset
我们在绑定perdrawcall的descriptor Set的时候，会设置一个参数是dynamic offset，如下:

```cpp
// bind perdrawcall
uint32_t dynamic_offsets[3] = {perframe_dynamic_offset,
                                perdrawcall_dynamic_offset,
                                per_drawcall_vertex_blending_dynamic_offset};
                                
m_rhi->cmdBindDescriptorSetsPFN(m_rhi->getCurrentCommandBuffer(),
                                RHI_PIPELINE_BIND_POINT_GRAPHICS,
                                m_render_pipelines[_render_pipeline_type_mesh_gbuffer].layout,
                                0,
                                1,
                                &m_descriptor_infos[_mesh_global].descriptor_set,
                                3,
                                dynamic_offsets);
```

Dynamic offset（动态偏移量）是在图形渲染中使用的一种技术，用于在绑定描述符集（Descriptor Set）时对存储缓冲区进行偏移。动态偏移量允许在每个绘制调用期间动态地调整存储缓冲区的偏移位置，以便使用不同的数据。

1-perframe_dynamic_offset（每帧动态偏移量）：这个偏移量可能用于每帧更新的数据，
例如摄像机的视图投影矩阵、光照信息等。由于这些数据每帧都会发生变化，因此需要在每次绘制调用之前通过动态偏移量更新相关的存储缓冲区。

2-perdrawcall_dynamic_offset（每个绘制调用动态偏移量）：这个偏移量可能用于每个绘制调用独有的数据，
例如每个绘制调用的模型矩阵、材质信息等。每个绘制调用可能使用不同的数据，因此通过动态偏移量可以在每次绘制调用之前更新相关的存储缓冲区。

3-per_drawcall_vertex_blending_dynamic_offset（每个绘制调用顶点混合动态偏移量）：
这个偏移量可能用于启用顶点混合的绘制调用中的顶点混合矩阵数据。顶点混合矩阵通常与每个绘制调用相关，因此可以使用动态偏移量在每次绘制调用之前更新相关的存储缓冲区。

dynamic offset 可以参考这个内容: https://github.com/SaschaWillems/Vulkan/tree/master/examples/dynamicuniformbuffer

### 46 Odd 和 Even的attachment设计

我们在attachments的枚举里可以看到 odd 和 even的设计
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

此处使用odd和even两个attachment,是为了实现不同阶段的渲染 pass 之间的交替读取和写入。

原因是:

1-Vulkan渲染管线中的每个Render Pass,一次只能读取或写入其中的attachment,但不能同时读写。

2-所以不同pass之间要实现输入输出关系,需要使用两个attachment交替进行。

所以，每个渲染pass的input attachment和write attachment(color attachment)是相互交换的。如下: 可以看到tone_mapping 和 color_grading 这两个相邻的pass，他们的input_attachment
和 color_attachment(write_attachment 是相互切换的)

```cpp
        // tone_mapping_pass 的 input_attachment
        RHIAttachmentReference tone_mapping_pass_input_attachment_reference {};
        tone_mapping_pass_input_attachment_reference.attachment = &backup_odd_color_attachment_description - attachments;
        tone_mapping_pass_input_attachment_reference.layout     = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // tone_mapping_pass 的 color_attachment
        RHIAttachmentReference tone_mapping_pass_color_attachment_reference {};
        tone_mapping_pass_color_attachment_reference.attachment = &backup_even_color_attachment_description - attachments;
        tone_mapping_pass_color_attachment_reference.layout     = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // ... ...

        // color_grading_pass 的input attachment
        RHIAttachmentReference color_grading_pass_input_attachment_reference {};
        color_grading_pass_input_attachment_reference.attachment = &backup_even_color_attachment_description - attachments;
        color_grading_pass_input_attachment_reference.layout     = RHI_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        // color_grading_pass 的color attachment
        RHIAttachmentReference color_grading_pass_color_attachment_reference {};
        if (m_enable_fxaa)
        {
            color_grading_pass_color_attachment_reference.attachment = &post_process_odd_color_attachment_description - attachments;
        }
        else
        {
            color_grading_pass_color_attachment_reference.attachment = &backup_odd_color_attachment_description - attachments;
        }
        color_grading_pass_color_attachment_reference.layout = RHI_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
```
使用odd even两个attachment交替写入的好处就是可以节约Image(Buffer)资源,重复利用内存,实现类似内存池的效果。

具体原因如下:

1-每个Image(Buffer)在GPU内存中都需要分配一定的空间。

2-如果每个pass都单独分配input和output的attachment,随着pass增多会消耗很多GPU内存。

3-但实际上任意两个pass之间,一个pass的output就是下一个pass的input。

4-使用odd even机制,只需要两个attachment就可以支持任意数量的pass交替传递数据。

5-这样就可以高效重复利用这两个attachment,不用为每个pass单独分配资源,大幅节省内存。

6-等于实现了一个仅有两个"内存块"的内存池,任何pass都可以从中取用资源,然后归还给池子使用。

### 47 descriptor 和 pipeline的一对多关系

一个PipilineLayout，可以有三个 DescriptorSetLayout作为Parent； 同时一个Pipeline也可以绑定多个DescriptorSet； 

如果一个PipelineLayout有3个DescriptorSetLayout作为parent,那么这个PipelineLayout在绑定Pipeine时,就要求绑定3个DescriptorSet。

具体来说:

PipelineLayout通过其pDescriptorSetLayouts数组指定了支持的DescriptorSetLayout类型。

每个Layout定义了一个DescriptorSet的结构(binding布局)。

那么PipelineLayout总的DescriptorSet数量,就是pDescriptorSetLayouts数组的长度。

在绑定Pipeline时,需要按照数组顺序绑定DescriptorSet,数量必须与Layouts数目一致。

### 48 多个Descriptor Set在Shader里怎么被使用

```cpp

layout(set = 0, binding = 1) readonly buffer _unused_name_per_drawcall
{
    VulkanMeshInstance mesh_instances[m_mesh_per_drawcall_max_instance_count];
};

layout(set = 0, binding = 2) readonly buffer _unused_name_per_drawcall_vertex_blending
{
    highp mat4 joint_matrices[m_mesh_vertex_blending_max_joint_count * m_mesh_per_drawcall_max_instance_count];
};
layout(set = 1, binding = 0) readonly buffer _unused_name_per_mesh_joint_binding
{
    VulkanMeshVertexJointBinding indices_and_weights[];
};

```

比如这样，用Set = 0， Set = 1， 这样来定位资源的位置。

### 49 To be Continue...
Piccolo小引擎自我阅读部分的理解到此结束了，期待有更多的时间和效率区阅读小引擎的其他部分，未完待续，博大精深！