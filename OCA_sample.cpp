/***********************************************************************************************************************
* Copyright (C) 2024 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : sample_app_cam.cpp
* Version      : 1.10
* Description  : RZ/V2L OpenCV Accelerator Sample Application
***********************************************************************************************************************/

/*****************************************
* Includes
******************************************/
/*Definition of Macros & other variables*/
#include "define.h"
/*OpenCV*/
#include <opencv2/opencv.hpp>

/*****************************************
* Global Variables
******************************************/
#define SRC_WIDTH       (1920)
#define SRC_HEIGHT      (1080)

#define C_DELAY         (0)

static uint8_t out_data[SRC_WIDTH * SRC_HEIGHT * 4];
static int src_size;
static int dst_size;

/* for suppress optimization */
volatile int oca_s;

/* for time count */
struct timespec start_time;
struct timespec end_time;

/* OpenCVA Circuit Number */
#define DRP_FUNC_NUM            (16)
#define DRP_FUNC_RESIZE         (0)
#define DRP_FUNC_CVT_YUV2BGR    (2)
#define DRP_FUNC_CVT_NV2BGR     (2)
#define DRP_FUNC_GAUSSIAN       (4)
#define DRP_FUNC_DILATE         (5)
#define DRP_FUNC_ERODE          (6)
#define DRP_FUNC_FILTER2D       (7)
#define DRP_FUNC_SOBEL          (8)
#define DRP_FUNC_A_THRESHOLD    (9)
#define DRP_FUNC_TMPLEATMATCH   (10)
#define DRP_FUNC_AFFINE         (11)
#define DRP_FUNC_PYR_DOWN       (12)
#define DRP_FUNC_PYR_UP         (13)
#define DRP_FUNC_PERSPECTIVE    (14)

/* OpenCVA Activate */
#define OPENCVA_FUNC_DISABLE    (0)
#define OPENCVA_FUNC_ENABLE     (1)
#define OPENCVA_FUNC_NOCHANGE   (2)


/*****************************************
* Function Name : timedifference_msec
* Description   : compute the time diffences in ms between two moments
* Arguments     : t0 = start time
*                 t1 = stop time
* Return value  : the time diffence in ms
******************************************/
double timedifference_msec(struct timespec t0, struct timespec t1) {
	return float((t1.tv_sec - t0.tv_sec) * 1000.0 + (t1.tv_nsec - t0.tv_nsec) / 1000000.0);
}

/* main */
int32_t main(int32_t argc, char *argv[]) {
	double cpu_time, oca_time;
	unsigned long OCA_f[16];
	printf("RZ/V2MA OPENCV SAMPLE\n");
	printf("[1] resize              FHD(BGR) -> XGA(BGR)     \n");
	printf("[2] cvtColor            FHD(YUV) -> FHD(BGR)     \n");
	printf("[3] cvtColorTwoPlane    FHD(NV)  -> FHD(BGR)     \n");
	printf("[4] GaussianBlur        FHD(BGR) [7x7]           \n");
	printf("[5] dilate              FHD(BGR) [iteration=200] \n");
	printf("[6] erode               FHD(BGR) [iteration=100] \n");
	printf("[7] morphologyEX        FHD(BGR) [iteration= 50] \n");
	printf("[8] filter2D            FHD(BGR)                 \n");
	printf("[9] Sobel               FHD(BGR)                 \n");
	printf("[10] adaptiveThreshold  FHD(gray)[kernel= 99x99] \n");
	printf("[11] matchTemplate  640x360(BGR) [template 16x16]\n");
	printf("[12] warpAffine         FHD(BGR) [rotate PI/4]   \n");
	printf("[13] warpPerspective    FHD(BGR)                 \n");
	printf("[14] pyrDown            FHD(BGR) -> QFHD(BGR)    \n");
	printf("[15] pyrUp              QFHD(BGR) -> FHD(BGR)    \n");
	printf("\n\n");

	for (int i = 0; i < DRP_FUNC_NUM; i++) {
		OCA_f[i] = OPENCVA_FUNC_NOCHANGE;
	}

	/********************/
	/* Dummy(filter2D)  */
	/********************/
	{
		float kernel[9] = {0.0f, 1.0f, 0.0f, 1.0f, -4.0, 1.0f, 0.0f, 1.0f, 0.0f};
		cv::Mat unsharp(3, 3, CV_32FC1, kernel);
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();
		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_FILTER2D] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		cv::filter2D(src_image, dst_image, -1, unsharp);
		oca_s = out_data[0]; //for suppress optimization

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_FILTER2D] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		cv::filter2D(src_image, dst_image, -1, unsharp);
		oca_s = out_data[0]; //for suppress optimization
		OCA_f[DRP_FUNC_FILTER2D] = OPENCVA_FUNC_NOCHANGE;
	}


	/**************************************/
	/* [1]  resize   FHD(BGR) -> XGA(BGR) */
	/**************************************/
	{
		printf("[1] resize             FHD(BGR) -> XGA(BGR)      \n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(768, 1024, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = 1024 * 768 * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_RESIZE] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::resize(src_image, dst_image, {1024, 768}, 0, 0, cv::INTER_LINEAR);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA1_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_RESIZE] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::resize(src_image, dst_image, {1024, 768}, 0, 0, cv::INTER_LINEAR);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA1_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Result */
		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_RESIZE] = OPENCVA_FUNC_NOCHANGE;
	}


	/****************************************/
	/* [2]  cvtColor   FHD(YUV) -> FHD(BGR) */
	/****************************************/
	{
		uint8_t in_data[SRC_WIDTH * SRC_HEIGHT * 2];
		int src_counter;
		printf("[2] cvtColor           FHD(YUV) -> FHD(BGR)\n");
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		cv::Mat tmp_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat3b tmp2;
		cv::Vec3b bgr;
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		tmp_image = imread("image.png", cv::IMREAD_COLOR);
		tmp2 = tmp_image;
		sync();

		/* Convert BGR to UYVY */
		src_counter = 0;
		for (int y = 0; y < SRC_HEIGHT; y++) {
			for (int x = 0; x < SRC_WIDTH; x += 2) {
				float y0, y1, u, v;
				int r0, g0, b0, r1, g1, b1;
				bgr = tmp2(cv::Point(x, y));
				r0 = bgr[2];
				g0 = bgr[1];
				b0 = bgr[0];
				bgr = tmp2(cv::Point(x + 1, y));
				r1 = bgr[2];
				g1 = bgr[1];
				b1 = bgr[0];

				y0 = r0 * 0.299 + g0 * 0.587 + b0 * 0.114;
				y1 = r1 * 0.299 + g1 * 0.587 + b1 * 0.114;
				u = (r0 + r1) * (-0.169) / 2.0 + (g0 + g1) * (-0.331) / 2.0 + (b0 + b1) * 0.500 / 2.0 + 128.0;
				v = (r0 + r1) * 0.500 / 2.0 + (g0 + g1) * (-0.419) / 2.0 + (b0 + b1) * (-0.081) / 2.0 + 128.0;
				in_data[src_counter + 0] = y0 > 255.0 ? 255 : y0 < 0.0 ? 0 : (unsigned char) y0;
				in_data[src_counter + 1] = u > 255.0 ? 255 : u < 0.0 ? 0 : (unsigned char) u;
				in_data[src_counter + 2] = y1 > 255.0 ? 255 : y1 < 0.0 ? 0 : (unsigned char) y1;
				in_data[src_counter + 3] = v > 255.0 ? 255 : v < 0.0 ? 0 : (unsigned char) v;
				src_counter += 4;
			}
		}
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC2, in_data);

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_CVT_YUV2BGR] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::cvtColor(src_image, dst_image, cv::COLOR_YUV2BGR_YUYV);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA2_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_CVT_YUV2BGR] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::cvtColor(src_image, dst_image, cv::COLOR_YUV2BGR_YUYV);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA2_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Result */
		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_CVT_YUV2BGR] = OPENCVA_FUNC_NOCHANGE;
	}


	/***********************************************/
	/* [3]  cvtColorTwoPlane   FHD(NV) -> FHD(BGR) */
	/***********************************************/
	{
		uint8_t in_data0[SRC_WIDTH * SRC_HEIGHT];
		uint8_t in_data1[(SRC_WIDTH / 2) * (SRC_HEIGHT / 2) * 2];
		printf("[3] cvtColorTwoPlane   FHD(NV) -> FHD(BGR)\n");
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		cv::Mat tmp_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat3b tmp2;
		cv::Vec3b bgr;
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		tmp_image = imread("image.png", cv::IMREAD_COLOR);
		tmp2 = tmp_image;
		sync();

		/* Convert BGR to NV21 */
		for (int y = 0; y < SRC_HEIGHT; y += 2) {
			for (int x = 0; x < SRC_WIDTH; x += 2) {
				float y00, y01, y10, y11, u, v;
				int r00, g00, b00, r01, g01, b01, r10, g10, b10, r11, g11, b11;
				bgr = tmp2(cv::Point(x, y));
				r00 = bgr[2];
				g00 = bgr[1];
				b00 = bgr[0];
				bgr = tmp2(cv::Point(x + 1, y));
				r01 = bgr[2];
				g01 = bgr[1];
				b01 = bgr[0];
				bgr = tmp2(cv::Point(x, y + 1));
				r10 = bgr[2];
				g10 = bgr[1];
				b10 = bgr[0];
				bgr = tmp2(cv::Point(x + 1, y + 1));
				r11 = bgr[2];
				g11 = bgr[1];
				b11 = bgr[0];

				y00 = r00 * 0.299 + g00 * 0.587 + b00 * 0.114;
				y01 = r01 * 0.299 + g01 * 0.587 + b01 * 0.114;
				y10 = r10 * 0.299 + g10 * 0.587 + b10 * 0.114;
				y11 = r11 * 0.299 + g11 * 0.587 + b11 * 0.114;
				u = (r00 + r01 + r10 + r11) * (-0.169) / 4.0 + (g00 + g01 + g10 + g11) * (-0.331) / 4.0 + (
						b00 + b01 + b10 + b11) * 0.500 / 4.0 + 128.0;
				v = (r00 + r01 + r10 + r11) * 0.500 / 4.0 + (g00 + g01 + g10 + g11) * (-0.419) / 4.0 + (
						b00 + b01 + b10 + b11) * (-0.081) / 4.0 + 128.0;
				in_data0[y * SRC_WIDTH + x] = y00 > 255.0 ? 255 : y00 < 0.0 ? 0 : (unsigned char) y00;
				in_data0[y * SRC_WIDTH + x + 1] = y01 > 255.0 ? 255 : y01 < 0.0 ? 0 : (unsigned char) y01;
				in_data0[(y + 1) * SRC_WIDTH + x] = y10 > 255.0 ? 255 : y10 < 0.0 ? 0 : (unsigned char) y10;
				in_data0[(y + 1) * SRC_WIDTH + x + 1] = y11 > 255.0 ? 255 : y11 < 0.0 ? 0 : (unsigned char) y11;
				in_data1[y * (SRC_WIDTH / 2) + x] = u > 255.0 ? 255 : u < 0.0 ? 0 : (unsigned char) u;
				in_data1[y * (SRC_WIDTH / 2) + x + 1] = v > 255.0 ? 255 : v < 0.0 ? 0 : (unsigned char) v;
			}
		}
		cv::Mat src_image1(SRC_HEIGHT,SRC_WIDTH, CV_8UC1, in_data0);
		cv::Mat src_image2((SRC_HEIGHT / 2), (SRC_WIDTH / 2), CV_8UC2, in_data1);

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_CVT_NV2BGR] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::cvtColorTwoPlane(src_image1, src_image2, dst_image, cv::COLOR_YUV2RGB_NV21);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA3_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_CVT_NV2BGR] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::cvtColorTwoPlane(src_image1, src_image2, dst_image, cv::COLOR_YUV2RGB_NV21);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA3_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Result */
		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_CVT_YUV2BGR] = OPENCVA_FUNC_NOCHANGE;
	}


	/**************************************/
	/* [4]  GaussianBlur   FHD(BGR) [7x7] */
	/**************************************/
	{
		printf("[4] GaussianBlur       FHD(BGR) [7x7]\n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_GAUSSIAN] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::GaussianBlur(src_image, dst_image, {7, 7}, 0, 0);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA4_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_GAUSSIAN] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::GaussianBlur(src_image, dst_image, {7, 7}, 0, 0);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA4_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_GAUSSIAN] = OPENCVA_FUNC_NOCHANGE;
	}


	/******************************************/
	/* [5]  dilate   FHD(BGR) [iteration=200] */
	/******************************************/
	{
		printf("[5] dilate             FHD(BGR) [iteration=200]\n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_DILATE] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::dilate(src_image, dst_image, cv::Mat(), cv::Point(-1, -1), 200);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA5_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_DILATE] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::dilate(src_image, dst_image, cv::Mat(), cv::Point(-1, -1), 200);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA5_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_DILATE] = OPENCVA_FUNC_NOCHANGE;
	}


	/*****************************************/
	/* [6]  erode   FHD(BGR) [iteration=100] */
	/*****************************************/
	{
		printf("[6] erode              FHD(BGR) [iteration=100]\n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_ERODE] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::erode(src_image, dst_image, cv::Mat(), cv::Point(-1, -1), 100);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA6_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_ERODE] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::erode(src_image, dst_image, cv::Mat(), cv::Point(-1, -1), 100);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA6_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_ERODE] = OPENCVA_FUNC_NOCHANGE;
	}


	/***********************************************/
	/* [7]  morphologyEX   FHD(BGR) [iteration=50] */
	/***********************************************/
	{
		printf("[7] morphologyEX       FHD(BGR) [iteration= 50] \n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_ERODE] = OPENCVA_FUNC_DISABLE;
		OCA_f[DRP_FUNC_DILATE] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::morphologyEx(src_image, dst_image, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), 50);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA7_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_ERODE] = OPENCVA_FUNC_ENABLE;
		OCA_f[DRP_FUNC_DILATE] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::morphologyEx(src_image, dst_image, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), 50);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA7_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_ERODE] = OPENCVA_FUNC_NOCHANGE;
		OCA_f[DRP_FUNC_DILATE] = OPENCVA_FUNC_NOCHANGE;
	}


	/****************************/
	/* [8]  filter2D   FHD(BGR) */
	/****************************/
	{
		float kernel[9] = {-0.2, -0.2, -0.2, -0.2, 2.6, -0.2, -0.2, -0.2, -0.2};
		cv::Mat unsharp(3, 3, CV_32FC1, kernel);
		printf("[8] filter2D           FHD(BGR)\n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_FILTER2D] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::filter2D(src_image, dst_image, -1, unsharp);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA8_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_FILTER2D] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::filter2D(src_image, dst_image, -1, unsharp);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA8_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_FILTER2D] = OPENCVA_FUNC_NOCHANGE;
	}


	/*************************/
	/* [9]  Sobel   FHD(BGR) */
	/*************************/
	{
		printf("[9] Sobel              FHD(BGR)\n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_SOBEL] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::Sobel(src_image, dst_image, -1, 1, 0);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA9_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_SOBEL] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::Sobel(src_image, dst_image, -1, 1, 0);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA9_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_SOBEL] = OPENCVA_FUNC_NOCHANGE;
	}


	/*****************************************************/
	/* [10]  adaptiveThreshold  FHD(gray)[kernel= 99x99] */
	/*****************************************************/
	{
		printf("[10] adaptiveThreshold FHD(gray)[kernel= 99x99]\n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC1);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC1, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 1;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 1;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_GRAYSCALE);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_A_THRESHOLD] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::adaptiveThreshold(src_image, dst_image, 0xFF, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 99, 0);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA10_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_A_THRESHOLD] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::adaptiveThreshold(src_image, dst_image, 0xFF, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 99, 0);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA10_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_A_THRESHOLD] = OPENCVA_FUNC_NOCHANGE;
	}


	/****************************************************/
	/* [11] matchTemplate 640x360(BGR) [template 16x16] */
	/****************************************************/
	{
		printf("[11] matchTemplate 640x360(BGR) [template 16x16]\n");
		cv::Mat tmp_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat src_image(360, 640, CV_8UC3);
		cv::Mat tpl_image(16, 16, CV_8UC3);
		cv::Mat dst_image(360 - 16 + 1, 640 - 16 + 1, CV_32FC1, out_data);
		cv::Mat out_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		tmp_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Crop src image(640x360) */
		src_image = cv::Mat(tmp_image, cv::Rect(800, 400, 640, 360));
		src_image = src_image.clone();

		/* Crop tpl image(16x16) */
		tpl_image = cv::Mat(tmp_image, cv::Rect(1200, 560, 16, 16));
		tpl_image = tpl_image.clone();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_TMPLEATMATCH] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::matchTemplate(src_image, tpl_image, dst_image, cv::TM_SQDIFF);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time); {
			double min, max;
			cv::Point min_p, max_p, maxr_p;
			cv::minMaxLoc(dst_image, &min, &max, &min_p, &max_p);
			out_image = imread("image.png", cv::IMREAD_COLOR);
			cv::rectangle(out_image, {min_p.x + 800, min_p.y + 400}, {min_p.x + 816, min_p.y + 416}, (128, 128, 128),
						  2);
		}
		oca_s = out_data[0]; //for suppress optimization

		/* [CPU]Write image data */
		imwrite("OCA11_cpu_out.png", out_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_TMPLEATMATCH] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::matchTemplate(src_image, tpl_image, dst_image, cv::TM_SQDIFF);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time); {
			double min, max;
			cv::Point min_p, max_p, maxr_p;
			cv::minMaxLoc(dst_image, &min, &max, &min_p, &max_p);
			out_image = imread("image.png", cv::IMREAD_COLOR);
			cv::rectangle(out_image, {min_p.x + 800, min_p.y + 400}, {min_p.x + 816, min_p.y + 416}, (128, 128, 128),
						  2);
		}

		/* [OCA]Write image data */
		imwrite("OCA11_oca_out.png", out_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_TMPLEATMATCH] = OPENCVA_FUNC_NOCHANGE;
	}


	/*******************************/
	/* [12]  warpAffine   FHD(BGR) */
	/*******************************/
	{
		float kernel[6] = {0.7071, -0.7071, 649, 0.7071, 0.7071, 510};
		cv::Mat rotate45(2, 3, CV_32FC1, kernel);
		printf("[12] warpAffine          FHD(BGR)\n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_AFFINE] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::warpAffine(src_image, dst_image, rotate45, {SRC_WIDTH,SRC_HEIGHT});
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA12_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_AFFINE] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::warpAffine(src_image, dst_image, rotate45, {SRC_WIDTH,SRC_HEIGHT});
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA12_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_AFFINE] = OPENCVA_FUNC_NOCHANGE;
	}


	/************************************/
	/* [13]  warpPerspective   FHD(BGR) */
	/************************************/
	{
		float kernel[9] = {0.5, 0.2, 20, -0.1, 0.8, 50, -0.001, 0.001, 1.0};
		cv::Mat perspective(3, 3, CV_32FC1, kernel);
		printf("[13] warpPerspective         FHD(BGR)\n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_PERSPECTIVE] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::warpPerspective(src_image, dst_image, perspective, {SRC_WIDTH,SRC_HEIGHT});
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA13_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_PERSPECTIVE] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::warpPerspective(src_image, dst_image, perspective, {SRC_WIDTH,SRC_HEIGHT});
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA13_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_PERSPECTIVE] = OPENCVA_FUNC_NOCHANGE;
	}


	/****************************************/
	/* [14]  pyrDown  FHD(BGR) -> QFHD(BGR) */
	/****************************************/
	{
		printf("[14] pyrDown           FHD(BGR) -> QFHD(BGR)    \n");
		cv::Mat src_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3);
		cv::Mat dst_image((SRC_HEIGHT / 2), (SRC_WIDTH / 2), CV_8UC3, out_data);
		src_size = SRC_WIDTH * SRC_HEIGHT * 3;
		dst_size = (SRC_WIDTH / 2) * (SRC_HEIGHT / 2) * 3;

		/* Read image data */
		src_image = imread("image.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_PYR_DOWN] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::pyrDown(src_image, dst_image);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA14_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_PYR_DOWN] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::pyrDown(src_image, dst_image);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA14_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_PYR_DOWN] = OPENCVA_FUNC_NOCHANGE;
	}


	/**************************************/
	/* [15]  pyrUp  QFHD(BGR) -> FHD(BGR) */
	/**************************************/
	{
		printf("[15] pyrUp              QFHD(BGR) -> FHD(BGR)    \n");
		cv::Mat src_image((SRC_HEIGHT / 2), (SRC_WIDTH / 2), CV_8UC3);
		cv::Mat dst_image(SRC_HEIGHT,SRC_WIDTH, CV_8UC3, out_data);
		src_size = (SRC_WIDTH / 2) * (SRC_HEIGHT / 2) * 3;
		dst_size = SRC_WIDTH * SRC_HEIGHT * 3;

		/* Read image data */
		src_image = imread("OCA14_cpu_out.png", cv::IMREAD_COLOR);
		sync();

		/* Disable Opencv Accelerator */
		OCA_f[DRP_FUNC_PYR_UP] = OPENCVA_FUNC_DISABLE;
		OCA_Activate(&OCA_f[0]);

		/* [CPU]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::pyrUp(src_image, dst_image);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		cpu_time = timedifference_msec(start_time, end_time);
		printf("[CPU]%fmsec\n", cpu_time);

		/* [CPU]Write image data */
		imwrite("OCA15_cpu_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		/* Enable Opencv Accelerator */
		OCA_f[DRP_FUNC_PYR_UP] = OPENCVA_FUNC_ENABLE;
		OCA_Activate(&OCA_f[0]);

		/* [OCA]Opencv start */
		timespec_get(&start_time, TIME_UTC);
		cv::pyrUp(src_image, dst_image);
		oca_s = out_data[0]; //for suppress optimization
		timespec_get(&end_time, TIME_UTC);
		oca_time = timedifference_msec(start_time, end_time);
		printf("[OCA]%fmsec\n", oca_time);

		/* [OCA]Write image data */
		imwrite("OCA15_oca_out.png", dst_image);
		sync();
		/* Wait to complete writing to storage */
#if C_DELAY
		sleep(C_DELAY);
#endif

		printf("[CPU] / [OCA] = %f times\n\n", cpu_time / oca_time);
		OCA_f[DRP_FUNC_PYR_UP] = OPENCVA_FUNC_NOCHANGE;
	}


	printf("[END] Complete!!\n");
	return 0;
}
