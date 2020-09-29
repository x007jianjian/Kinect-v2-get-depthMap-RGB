
本工程实现了：
按空格键保存，1080*1920的配准好的RGB和depth。
OpenGL实时显示的还是424*512，也是配准好的。


采坑记录：
在内部直接初始化数组会爆，所以改成malloc初始化。	unsigned short * ffdest = (unsigned short *)malloc(sizeof(unsigned short)* colorwidth*colorheight);
