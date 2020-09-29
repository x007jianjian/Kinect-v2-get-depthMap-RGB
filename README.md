[toc]

# Kinect-v2-get-depthMap-RGB

# 本工程实现了：
按空格键拍照，保存`1080*1920`的配准好的RGB和depth。  
OpenGL实时显示的还是`424*512`，也是配准好的。

# 环境配置
## 系统环境
windows
## 首先参考官方tutorials
SDK的安装和环境配置都先参考官方
https://kinect-tutorials-zh.readthedocs.io/zh_CN/latest/SDKInstallation.html#kinect-v2-0-sdk
## 这里再梳理（赘述）一下在vs上配置
ref：https://kinect-tutorials-zh.readthedocs.io/zh_CN/latest/kinect2/0_Setup.html#id2  
==这个教程是很值得读和运行的==

我没有把`.h .lib .dll`添加到系统目录，是直接在vs中添加到项目（具体配置方式我是结合下面这个链接），下载的话还是按照上面那个教程下载的`freeglut`。

https://www.jianshu.com/p/cf2803898418

### 添加库（glut和opencv添加方式都是这套流程）
在这儿还是再赘述一下
#### 添加什么
比如说下载的`freeglut`包，看一下结构。  
![image](https://s1.ax1x.com/2020/09/25/0CdEu9.png)

无非就是`.dll  .h  .lib`这三个


#### 往哪添加
属性管理器新建的项目属性表：“Kinect”  
![image](https://s1.ax1x.com/2020/09/25/0CaJYT.png)  

我选择的是更改`Debug x64`里面的“Kinect”，将编译器也选择成与之对应相同的，如下。  
![image](https://s1.ax1x.com/2020/09/25/0CaWXd.png)

#### 具体添加

##### .h
- `VC++目录`中的包含目录，路径字符串写到`include`就ok，include后面的就不用写了。
![image](https://s1.ax1x.com/2020/09/25/0CdGDA.png)

##### .lib
- （同上图）`VC++目录`中库目录，这个的路径字符串写到`.lib`文件所在的目录。注意选择`x64  还是x86`
- `链接器`的`常规`中的附加库目录。也需要添加`.lib`文件所在的目录（路径同上）。  
![image](https://s1.ax1x.com/2020/09/25/0CwiIP.png)

- `链接器`的`输入`中的附加依赖项，把所有`.lib`文件的名字都写上去，回车隔开。
![image](https://i.loli.net/2020/09/25/DwWbQfKs38MUg4z.png)

##### .dll
选择对应系统版本(64 or x86)的`.dll`粘贴到main的目录。

### glew配置
见这节教程 https://kinect-tutorials-zh.readthedocs.io/zh_CN/latest/kinect2/3_PointCloud.html
# 采坑记录：
在内部直接初始化数组会爆，所以改成malloc初始化。	unsigned short * ffdest = (unsigned short *)malloc(sizeof(unsigned short)* colorwidth*colorheight);
