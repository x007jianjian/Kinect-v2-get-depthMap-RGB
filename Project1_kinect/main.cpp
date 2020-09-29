
#include "main.h"
#include "glut.h"

#include <cmath>
#include <cstdio>

#include <Windows.h>
#include <Ole2.h>


#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <Kinect.h>

#include <fstream>  //ofstream类的头文件
#include <iostream>
using namespace std;
#include <conio.h>//检测按键事件


// We'll be using buffer objects to store the kinect point cloud
GLuint vboId;
GLuint cboId;

// Intermediate Buffers
unsigned char rgbimage[colorwidth*colorheight*4];    // Stores RGB color image
ColorSpacePoint depth2rgb[width*height];             // Maps depth pixels to rgb pixels
DepthSpacePoint rgb2depth[colorwidth*colorheight];

CameraSpacePoint depth2xyz[width*height];			 // Maps depth pixels to 3d coordinates

// Kinect Variables
IKinectSensor* sensor;             // Kinect sensor
IMultiSourceFrameReader* reader;   // Kinect data source
ICoordinateMapper* mapper;         // Converts between depth, color, and 3d coordinates

int flag = 3;
ofstream mycout("temp.txt");
int indexOfPhoto = -1;//使得照片编号从0开始

bool initKinect() {
    if (FAILED(GetDefaultKinectSensor(&sensor))) {
		return false;
	}
	if (sensor) {
		sensor->get_CoordinateMapper(&mapper);

		sensor->Open();
		sensor->OpenMultiSourceFrameReader(
			FrameSourceTypes::FrameSourceTypes_Depth | FrameSourceTypes::FrameSourceTypes_Color,
			&reader);
		return reader;
	} else {
		return false;
	}
}

void getDepthData(IMultiSourceFrame* frame, GLubyte* dest) {
	IDepthFrame* depthframe;
	IDepthFrameReference* frameref = NULL;
	frame->get_DepthFrameReference(&frameref);
	frameref->AcquireFrame(&depthframe);
	if (frameref) frameref->Release();

	if (!depthframe) return;

	// Get data from frame
	unsigned int sz;
	unsigned short* buf;
	depthframe->AccessUnderlyingBuffer(&sz, &buf);


	// Write vertex coordinates
	mapper->MapDepthFrameToCameraSpace(width*height, buf, width*height, depth2xyz);
	float* fdest = (float*)dest;
	for (int i = 0; i < sz; i++) {
		*fdest++ = depth2xyz[i].X;
		*fdest++ = depth2xyz[i].Y;
		*fdest++ = depth2xyz[i].Z;

	}

	// Fill in depth2rgb map
	mapper->MapDepthFrameToColorSpace(width*height, buf, width*height, depth2rgb);

	//将深度图映射到rgb
	// Fill in rgb2depth map
	mapper->MapColorFrameToDepthSpace(width*height, buf, colorwidth*colorheight, rgb2depth);
	unsigned short * ffdest = (unsigned short *)malloc(sizeof(unsigned short)* colorwidth*colorheight);
	if (ffdest == NULL)
		exit(0);
	for (int i = 0; i < colorwidth*colorheight; i++) {
		DepthSpacePoint p = rgb2depth[i];
		// Check if color pixel coordinates are in bounds
		if (p.X < 0 || p.Y < 0 || p.X > width || p.Y > height) {
			ffdest[i] = 0;
		}
		else {
			int idx = (int)p.X + width*(int)p.Y;
			ffdest[i] = buf[idx];
		}
	}
	for (; flag < 2; flag++)
	{
		cv::Mat mDepthImg(colorheight, colorwidth, CV_16UC1, ffdest);
		cv::imwrite("photos\\depth\\" + to_string(indexOfPhoto) + ".pgm", mDepthImg);
	}
	free(ffdest);

	if (depthframe) depthframe->Release();

	//CameraIntrinsics* m_pCameraIntrinsics = new CameraIntrinsics();
	//mapper->GetDepthCameraIntrinsics(m_pCameraIntrinsics);//GetDepthCameraIntrinsics：获得ir相机参数（出厂已写入设备
	//if (m_pCameraIntrinsics->FocalLengthX && flag == 1)
	//{
	//	// 打印
	//	cout << "FocalLengthX : " << m_pCameraIntrinsics->FocalLengthX << endl; //摄像机的X焦距，以像素为单位
	//	cout << "FocalLengthY : " << m_pCameraIntrinsics->FocalLengthY << endl; //相机的Y焦距，以像素为单位
	//	cout << "PrincipalPointX : " << m_pCameraIntrinsics->PrincipalPointX << endl;//相机在X维中的主要点，以像素为单位。
	//	cout << "PrincipalPointY : " << m_pCameraIntrinsics->PrincipalPointY << endl; //相机在Y轴上的主要点，以像素为单位。
	//	cout << "RadialDistortionFourthOrder : " << m_pCameraIntrinsics->RadialDistortionFourthOrder << endl;//相机的四阶径向畸变参数。
	//	cout << "RadialDistortionSecondOrder : " << m_pCameraIntrinsics->RadialDistortionSecondOrder << endl;//相机的二阶径向畸变参数。
	//	cout << "RadialDistortionSixthOrder : " << m_pCameraIntrinsics->RadialDistortionSixthOrder << endl;//摄像机的六阶径向畸变参数。
	//	flag = 0;
	//}

}

void getRgbData(IMultiSourceFrame* frame, GLubyte* dest) {
	IColorFrame* colorframe;
	IColorFrameReference* frameref = NULL;
	frame->get_ColorFrameReference(&frameref);
	frameref->AcquireFrame(&colorframe);
	if (frameref) frameref->Release();

	if (!colorframe) return;

	// Get data from frame
	colorframe->CopyConvertedFrameDataToArray(colorwidth*colorheight*4, rgbimage, ColorImageFormat_Rgba);

	// Write color array for vertices
	float* fdest = (float*)dest;
	for (int i = 0; i < width*height; i++) {
		ColorSpacePoint p = depth2rgb[i];
		// Check if color pixel coordinates are in bounds
		if (p.X < 0 || p.Y < 0 || p.X > colorwidth || p.Y > colorheight) {
			*fdest++ = 0;
			*fdest++ = 0;
			*fdest++ = 0;
		}
		else {
			int idx = (int)p.X + colorwidth*(int)p.Y;
			*fdest++ = rgbimage[4*idx + 0]/255.;
			*fdest++ = rgbimage[4*idx + 1]/255.;
			*fdest++ = rgbimage[4*idx + 2]/255.;
		}
		// Don't copy alpha channel
	}

	if (colorframe) colorframe->Release();


	//保存RGB
	for (; flag < 3; flag++)
	{
		unsigned char * ffdest = (unsigned char *)malloc(sizeof(unsigned char)* colorwidth*colorheight * 3);
		for (int i = 0; i < colorwidth*colorheight; i++) {
			ffdest[3 * i] = rgbimage[4 * i + 2];
			ffdest[3 * i + 1] = rgbimage[4 * i + 1];
			ffdest[3 * i + 2] = rgbimage[4 * i + 0];
		}

		cv::Mat mColorImg(colorheight, colorwidth, CV_8UC3, ffdest);
		cv::imwrite("photos\\color\\" + to_string(indexOfPhoto) + ".png", mColorImg);

	}

	////保存RGB
	//for (; flag < 3; flag++)
	//{
	//	unsigned char ffdest[width*height*3];
	//	for (int i = 0; i < width*height; i++) {
	//		ColorSpacePoint p = depth2rgb[i];
	//		// Check if color pixel coordinates are in bounds
	//		if (p.X < 0 || p.Y < 0 || p.X > colorwidth || p.Y > colorheight) {
	//			ffdest[3*i] = 0;
	//			ffdest[3*i+1] = 0;
	//			ffdest[3*i+2] = 0;
	//		}
	//		else {
	//			int idx = (int)p.X + colorwidth*(int)p.Y;
	//			ffdest[3*i] = rgbimage[4 * idx + 2];
	//			ffdest[3*i+1] = rgbimage[4 * idx + 1];
	//			ffdest[3*i+2] = rgbimage[4 * idx + 0];
	//		}
	//		// Don't copy alpha channel
	//	}

	//	cv::Mat mColorImg(height, width, CV_8UC3);

	//	for (int i = 0; i < height; i++)
	//	{
	//		for (int j = 0; j < width; j++)
	//		{
	//			mColorImg.at<cv::Vec3b>(i, j) = cv::Vec3b(ffdest[3 * (width*i + j)], ffdest[3 * (width*i + j) + 1], ffdest[3 * (width*i + j) + 2]);
	//		}
	//	}

	//	cv::imwrite("my.png", mColorImg);

	//}


}

void getKinectData() {
	if (_kbhit()) {//如果有按键按下，则_kbhit()函数返回真
		int ch = _getch();//使用_getch()函数获取按下的键值
					  //cout << ch <<endl;
		if (ch == 32) {//当按下空格时拍照，空格键的键值时32
			cout << "catch photo：" << indexOfPhoto++ << endl;
			flag = 1;
		}
	}

	IMultiSourceFrame* frame = NULL;
	if (SUCCEEDED(reader->AcquireLatestFrame(&frame))) {
		GLubyte* ptr;
		glBindBuffer(GL_ARRAY_BUFFER, vboId);
		ptr = (GLubyte*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (ptr) {
			getDepthData(frame, ptr);
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);



		glBindBuffer(GL_ARRAY_BUFFER, cboId);
		ptr = (GLubyte*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		if (ptr) {
			getRgbData(frame, ptr);
		}
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	if (frame) frame->Release();
}

void rotateCamera() {
	static double angle = 0.;
	static double radius = 3.;
	double x = radius*sin(angle);
	double z = radius*(1-cos(angle)) - radius/2;
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(x,0,z,0,0,radius/2,0,1,0);
	angle += 0.002;
}

void drawKinectData() {
	getKinectData();
	//rotateCamera();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	glColorPointer(3, GL_FLOAT, 0, NULL);

	glPointSize(1.f);
	glDrawArrays(GL_POINTS, 0, width*height);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

int main(int argc, char* argv[]) {
	if (!init(argc, argv)) return 1;
	if (!initKinect()) return 1;

	// OpenGL setup
	glClearColor(0, 0, 0, 0);
	glClearDepth(1.0f);

	// Set up array buffers
	const int dataSize = width*height * 3 * 4;
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &cboId);
	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);

	// Camera setup
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, width / (GLdouble)height, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 0, 0, 0, 1, 0, 1, 0);




	// Main loop
	execute();

	return 0;

}