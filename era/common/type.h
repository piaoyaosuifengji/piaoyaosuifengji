
#ifndef MYTYPE_H
#define MYTYPE_H


#include "list.h"




#define NeroOK   1
#define NeroError   -1
#define NeroErrorMsg   printf("something wrong!\n")
#define NerOkMsg   	printf("exit right way!\n")
#define NerReportMsgError(id)   	printf("something wrong with ID:%d!\n",id)

typedef int  nero_s32int;
//typedef int  nero_s32int;
typedef unsigned int  nero_us32int;
typedef unsigned char  nero_us8int;
typedef signed char  nero_s8int;
typedef  char  nero_8int;



#define Nero_DeBuging1
#define Nero_DeBuging2
#define NowIdDeBug
#define NoIncludeG_return_if_fail 


typedef struct ChineseUtf8Code
{
nero_us8int first;
nero_us8int second;
nero_us8int third;
nero_us8int fourth;//下面是高位

}ChUTF8;
typedef struct ChineseUnicode16Code   //16位的Unicode编码
{
//	union{
	nero_us8int first;
	nero_us8int second;
//	}
}Unicode16;

typedef struct ChineseUnicode16DigitalCode//五位拼起来就是Unicode16码
{
nero_us8int first;
nero_us8int second;
nero_us8int third;
nero_us8int fourth;//下面是高位
nero_us8int fifth;

}DigitalUnicode16;


typedef struct POSITIOM
{
int x;
int y;

}POS;

//这个结构用来生成一个以坐标表示的像素的集合，显然在函数getLineFromBMP
//中，生成similarityAroundOnePoint矩阵中的链表和，根据S点，可以将这个
//链表最终生成BMP结构的实际位置的链表
typedef struct ListPos
{
struct list_head p;
POS pos;
}PosList;


typedef struct COLOUR
{
unsigned char r;
unsigned char g;
unsigned char b;
}Color;


typedef struct COLOURCount
{
struct list_head p;
unsigned char r;
unsigned char g;
unsigned char b;
int count;
}ColorCnt;

typedef struct  lineManagement
{
struct list_head p;//用来维持所有线条表头的结构--就是维持LineMan本身的链表
ColorCnt * cntHead;//指向用COLOURCount表示的线条链表的表头的指针
PosList * posHead;//指向用PosList表示的线条链表的表头的指针
int count;//如果是表头：记录多少条线
		//如果是节点就记录该条线上有多少个点
}LineMan;




#define printColorCNT(Col)  (printf("	col is %x %x %x ,count is %d\n",(Col->r),(Col->g),(Col->b),(Col->count)))

#define printColor(Col)  (printf("col is %x %x %x \n",(Col->r),(Col->g),(Col->b)))

#define equalCOlVsColorCnt(col,cnt)  ((col->r == cnt->r)  && (col->g == cnt->g) &&(col->b == cnt->b))


#define InitialPOS(c)  ((c.x)=(c.y)=0)
#define InitialCOLOUR(c)  (c.r=c.g=c.b=0)

//#define InitialCOLOURWithCNT(col,cnt)  ((col->r = cnt->r)  ; (col->g == cnt->g) ;(col->b = cnt->b);)

//#define InitialColorCnt(c)  (c.r=0;c.g=0;c.b=0;c.count;c.prev=NULL;c.p.next=NULL;)

#define putNumTwoINOne(high,low,type) ( (((type)high) << (sizeof(high)*8)) +low )

#define putNumFourINOne(high2,high1,low2,low1,type) ( (((type)(unsigned char)high2) << (sizeof(high1)*24)) + \
							(((type)(unsigned char)high1) << (sizeof(high1)*16))+\
							(((type)(unsigned char)low2) << (sizeof(high1)*8))+\
										(unsigned char)low1 )	\

//#define BYTEOS32 11
//#ifdef BYTEOS32
//#endif


//#ifdef BYTEOS64
//#endif









#endif
