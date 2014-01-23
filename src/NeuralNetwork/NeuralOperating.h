#ifndef NeuralOperating_H
#define NeuralOperating_H
#include "../common/type.h"
//#include "../tools/readUTF8File.h"

struct ZhCharArg
{
	ChUTF8 *chChar;
	nero_s32int charCounts;

};
struct DataFlowProcessArg
{
void **DataFlow;
nero_s32int *dataKind;
nero_s32int dataNum;
//NeuronObject  *GodNero;
NeroConf * conf;

};


struct NeroObjForecastList
{
	struct list_head p;
	NeuronObject * obj;
	nero_s32int Strengthen;//在一次预测过程中可能一个对象被多次预测,初始化为0
	nero_s32int times;//在整个预测成功中，该节点存在的时间长度,初始化为0
};

struct DataFlowForecastInfo
{
	NeuronObject ** objs;//实际对象指针
	nero_s32int objNum;//实际对象个数，也是objs这个数组的有效长度，数组长度必须大于objNum，不然越界	
	nero_s32int objPoint;//指向一个objs中可以读取的位置,初始为0，最大值为objNum
        struct NeroObjForecastList   headOfUpperLayer;//指向第一个预测对象	
	struct NeroObjForecastList   headOfLowerLayer;//指向第一个预测对象
	struct NeroObjForecastList   headOfSameLayer;//指向第一个预测对象
	
	nero_s32int start;//start end 是objs中某个子集的起始位置，用来指示该位置有衍生概念
	nero_s32int end;
			
};




void *thread_for_Operating_Pic(void *arg);
void * thread_for_Sys_Pic(void *arg);


/*这里的所有函数定位为神经系统与外界的数据沟通接口*/
/*理想的接口是你的这个机制最好符合所有的概念类型*/

/*
输入接口


输出接口
*/


/*从数据流中分离出俩个或者多个概念，并组合成一个新的概念，这个新的概念的类型由子概念决定*/
nero_s32int DataFlowProcess(void *DataFlow[],nero_s32int dataKind[],nero_s32int dataNum,NeuronObject  *GodNero,NeroConf * conf);




nero_s32int Process_StrengthenLink(NeuronObject * objs[],nero_s32int objNum,NeuronObject  *godNero,NeroConf * conf);


NeuronObject * Process_IfHasNextObjToread(struct DataFlowForecastInfo  * forecastInfo);


struct NeroObjForecastList   * Process_CompareWithForecastList(struct DataFlowForecastInfo  * forecastInfo,NeuronObject * findObi);


struct NeroObjForecastList   *  FindObjInForecastList(struct NeroObjForecastList   *head,NeuronObject * findObi);

NeuronObject * Process_IfFindDerivativeObj(struct DataFlowForecastInfo  * forecastInfo);

nero_s32int Process_UpdataForecastList(struct DataFlowForecastInfo  * forecastInfo,NeuronObject * newObj);

void  UpdataLastTimeINForecastList(struct DataFlowForecastInfo  * forecastInfo);

void AddNewObjToForecastList(struct DataFlowForecastInfo  * forecastInfo,NeuronObject * newObj);

void AddNodeIntoForecastList(struct list_head  * listHead,NeuronObject * Obj);

void CleanForecastList(struct DataFlowForecastInfo  * forecastInfo);











#endif
