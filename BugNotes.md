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


