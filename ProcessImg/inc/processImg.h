#ifndef PROCESSIMG_H
#define PROCESSIMG_H

#include <stdint.h>
#include "ov7725.h"

extern uint32_t starttime;


enum
{
	LINE_STRAIGHT,//直线//0
	LINE_TURN_LEFT_90,//1
	LINE_TURN_RIGHT_90,//2
	LINE_END,//3
	LINE_LOST_ERROR,//4
	LINE_MARK//5
};

#define REPORT_PACKAGE_HEAD 0x23

#pragma pack(push)
#pragma pack(1)

typedef struct
{
	uint8_t head;
	uint8_t frame_cnt;//帧计数
	uint8_t linestate;
	float angle_error;
	//朝向误差
	//                        0 degree
	//                         A
	//                         A
	//                         A
	//                         A
	//                         A
	//                         A
	//-90degree ←←←←←←←←A→→→→→→→→→→→→ +90degree
	int16_t middle_error;
	//离中心线的距离//负值代表线在飞机左边（即飞机需要左移）
	uint8_t checksum;
} report_package_type;


#pragma pack(pop)


uint8_t processImg(uint8_t ImgData[][IMG_WIDTH]);



#endif
