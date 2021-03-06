



#ifndef NeuralNetwork_H
#define NeuralNetwork_H
#include "../common/type.h"
#include "../tools/readUTF8File.h"
/*
在这个部分中，主要集中了有关神经网络部分的代码，包括网络在内存中的存放方式，
网络节点常用操作，还有网络和数据输入模块和数据输出模块的交互代码

神经网络模块：分为俩部分；
1:网络结构---就是数据保存方式
	这里提出一个神经节的概念：神经节是功能相同的神经元细胞体在中枢以外的周围部位集合而成的结节状构造。
	这里忽略“中枢以外的周围部位”，专指具有相似功能的神经细胞的集合
	
	之前给出的
	
typedef struct Neuron
{
int x;
int y;
int z;
int next;
}Neo;
	只是数据在文件中的结构，这里必须给出在内存中的结构：就是Neuron Objects。它可以是一个神经元，也可以是一组
	神经元的集合，他们共同实现一组功能或者表达一个概念。显然他们会有大量的连接，来表示他与其他概念的关系
	就是说这里弱化神经元(在内存中为ActivationNeuron)的概念，就强调一个存储在网络中的一个个对象
	
	但是如何区别神经对象和神经元他们各自的连接的含义呢
	
	：神经对象的连接是不同对象的存在的一种相互关系，或者说逻辑上的联系，而神经元之间的连接表示了数据之间的连接关系
	
	但实际上他们是同一个东西啊啊，因为一个对象也应该是用一个神经元来表示的，不然说不过去啊。

2:操作------对数据的操作方式



*/


/*系统配置结构
对一些全局配置进行统一管理

*/

/*几个有关NeroConfiguration的宏*/
#define  Conf_Modify_addLevelObjAlways    1   //修改addLevelObjAlways
#define  Conf_Modify_CreateNewBaseObjKind    2   //修改CreateNewBaseObjKind

 typedef struct NeroConfiguration
 {
	nero_us32int  addNewObj; /*是否在DataFlowProcess中添加在网络中的没有的数据为一个新概念，
					为1添加否则不添加,这个变量针对，做为整体输入的数据流（如一个词组），
					在网络中的不存在相应概念时是否添加概念
					*/
	nero_us32int  addLevelObj; /*是否在DataFlowProcess中形成层次结构，为1添加否则不添加
					这个变量针对，某次依次输入的的数据流（如一个句子，被分割为词组或单个
					字符时），
					在网络中的不存在由这些子概念组成的相应 高层概念 时是否添加概念
					这个选项激活时会根据当前链接的活跃情况创建新概念，否则不会添加新概念
					*/
	nero_us32int  addLevelObjAlways; /*在DataFlowProcess中总是形成层次结构，为1添加否则不添加
	
					这个选项激活时会不管当前链接的活跃情况和addLevelObj的值
					，总是会创建新  高层概念
					*/
					
	
	nero_us32int CreateNewBaseObjKind;/*是否创建新基类的标志，默认打开为1*/
	
	
	
	nero_us32int  neroTime;     /*系统运行时间单位，初始化为0，隔1秒钟增加1*/
	nero_us32int  ifReCreateLogFile;     /*系统运行时是否重新生成log文件*/
	nero_us32int	  UsedNeroNum;
	nero_us32int	  NewNeroClassID;/*创建新类时候可以使用的编号，它从
	                                #define NeuronNode_MinNewDerivativeClassId  2001
	                                开始
	                                */
 }NeroConf;
/*ActNero只是用来保存基础数据，它对应于一个单纯的神经元*/


/*
将指定位置的位设置为特定的值：
1111
	&
0101



msg：
低位			高位
1-------8  9-----16 17-----24  25----32
1111 1111 1111 1111 1111 1111 1111 1111 
1-16位表示该节点种类,注意，17到24位尽量别用，以免节点种类用16位不够用需要扩展


31：在一些基类中，用来表示该种类别的子类的排列顺序是否固定，1表示顺序固定，0表示顺序无所谓
32位：区别一般的概念和基类，1表示基类，0表示衍生类(网络中真正的数据)
*/

 struct ActivationNeuron;
typedef struct ActivationNeuron  NeuronObject;
typedef struct ActivationNeuron ActNero;
 typedef struct NerveFiber_ NerveFiber;




 struct ActivationNeuron
{
nero_us32int msg;/*记录该nero的种类，性质等信息*/
nero_s32int x;/*取值范围-2147483648 ~ 2147483647*/
nero_s32int y;
nero_s32int z;
struct NerveFiber_  * inputListHead;/*其实究竟对于一个神经元来说是不是需要这个输入神经元的链表，*/
					/*好像其实不是那么必要的是吧，因为，这实际上会造成冗余数据*/
					/*那么如果该神经元是一个NeuronObject，那么inputListHead指向该*/
					/*对象的数据，如果该神经元表示一个存抽象概念，则没有数据*/
					/*如果是一个ActNero，则该神经元的功能是保存数据*/
					/*但是如果是一个NeuronObject，是指向第一个数据呢，还是*/
					/*说inputListHead指向一个数据的链表，显然指向一个数据的神经元*/
					/*然后让这个神经元(纤维)指向后续的数据*/
					
					/*特别的，对于一个基类来说，它的inputListHead为NULL*/
					/*而他的outputListHead链表，则指向所有该类下的所以神经元*/
					
struct NerveFiber_   * outputListHead; 
};





/*神经纤维---用来连接各个神经元*/
/*
msg1：
低位			高位
1-------8  9-----16 17-----24  25----32
1111 1111 1111 1111 1111 1111 1111 1111 
1-8位表示该链接的强度，就是没被搜索并匹配成功到一次，加1,暂时100最大，初始化为0
9-10位表示该神经纤维的类型：
	指向该纤维所属神经元存储数据的神经元      #define	Fiber_PointToData	00
	指向该纤维所属神经元的上层概念的神经元	#define	Fiber_PointToUpperLayer	01
	指向该纤维所属神经元的下层概念的神经元 	#define	Fiber_PointToLowerLayer	10
	指向该纤维所属神经元的同层次概念的神经元	#define	Fiber_PointToSameLayer	11
	就是说如果为Fiber_PointToUpperLayer	01，那么第9位为1，第10位为0
	
11-12位（）：
        有且必须存在一个                         #define Fiber_PointToUniqueObj	00
	可重复，但必须存在一个	                #define	Fiber_PointToMutiObj	01
	可有可没有 	                        #define	Fiber_PointToUnnecessaryObj	10
	就是说如果为Fiber_PointToMutiObj	01，那么第11位为1，第12位为0       

	
time：
低位			高位
1-------8  9-----16 17-----24  25----32
1111 1111 1111 1111 1111 1111 1111 1111 	
1-20位存储上次被访问的时间（包括修改，被成功匹配的情况），初始化为当前系统时间	
		该信息会被修改的时间包括：
			1:创建时候
			2:被成功匹配的时候，相应链接被修改  Process_StrengthenLink=》gainFiberStrengthen




*/

  struct NerveFiber_
 {
struct ActivationNeuron   *obj;
struct NerveFiber_ * next;
nero_us32int msg1;/*存储额外的信息*/
nero_us32int time;/*有关修改时间的信息*/
 };



extern NeuronObject *GodNero;
extern NeroConf neroConf;
#define NeuronNode_ForNone   0    //当一个概念节点的类型为此时表示一个未知对象
#define NeuronNode_ForGodNero   1    //GodNero
#define NeuronNode_ForData   2    //表示是一个数据存储的神经元，不是一个概念
/*强调一点，人的大脑所以的输入信息都是通过各种感觉器官输入的，也就是说
任何信息在输入大脑之前一定是通过一定加工和处理的

*/
#define NeuronNode_ForConnect   10    //当一个概念节点的类型为此时表示一个链接节点
#define NeuronNode_ForImage   50    //当一个概念节点的类型为此时表示一个图像对象
#define NeuronNode_ForLine   51    //当一个概念节点的类型为此时表示一个线条对象


#define NeuronNode_ForChCharacter   61    //当一个概念节点的类型为此时表示一个汉字，包括中文标点符号
#define NeuronNode_ForChWord   62    //当一个概念节点的类型为此时表示一个中文词语
#define NeuronNode_ForChSentence   63    //当一个概念节点的类型为此时表示一个中文句子

#define NeuronNode_ForComplexDerivative  2000    //高级衍生类
#define NeuronNode_MinNewDerivativeClassId  2001


#define NeuronNode_MaxNewDerivativeClassId  65530
#define NeuronNode_Max   65535


/*区别一般的概念和基类*/
#define NeuronNode_BaseObject   1  /*就是一般的概念和基类进行区别*/
#define NeuronNode_DerivativeObject   0  /*就是一般的概念（从基类衍生的概念）和基类进行区别*/


/*神经纤维类型*/
/*有一种特殊的情况，就是一些类的数据就是一些保存数据的神经元，这些神经元用纤维连接起来
组成了一个链表，这么纤维的指向类型是Fiber_PointToData*/
#define	Fiber_PointToData	0
#define	Fiber_PointToUpperLayer	1
#define	Fiber_PointToLowerLayer	2
#define	Fiber_PointToSameLayer	3
/*基类中子类的出现情况定义*/
#define Fiber_PointToUniqueObj	        0
#define	Fiber_PointToMutiObj	        1
#define	Fiber_PointToUnnecessaryObj	2
/*神经纤维链接强度的最大值*/
#define	Fiber_StrengthenMax	200     //在结构体中共用了8位来记录这个值，所以最大为255
/*你需要定义个内存池，来管理使用和未使用的神经元*/





void donother();



nero_s32int initActNero(ActNero * nero,nero_us32int kind,NerveFiber *inputListHead,NerveFiber *outputListHead);
nero_s32int initNeroPool();
/*从神经元库存中获取一个空闲的神经元*/
NeuronObject * getNeuronObject();
/*创建一个指定类型的神经概念*/
NeuronObject *  nero_createNeroObj(nero_s32int kind);
/*创建一个数据存储 神经元,并初始化*/
ActNero * nero_createDataNero();
nero_s32int getFiberType(NerveFiber * fiber);

/*下面是几个处理神经元链表的函数，，，必须快速和方便使用*/


	/*在higher下面增加一个下层概念，仅仅lower指向higher*/
nero_s32int PointingToObject(NeuronObject *lower,NeuronObject *higher,nero_s32int pointTotype);

	/*根据Relationship来增加一个下层概念*/
nero_s32int addNeuronChild(NeuronObject *father,NeuronObject *child,nero_s32int Relationship);

/*给一个神经元添加一条神经纤维,type指出添加的是输出，输出纤维*/
/*返回添加的那个纤维的指针，它已经被添加到ActNero *  n中了，你需要自己继续往里面添加具体的连接对象，就是NerveFiber-》obj*/
//static inline NerveFiber * addNerveFiber(ActNero *  n,nero_s32int type);

/*将新概念加入网络*/
nero_s32int nero_addNeroIntoNet(NeuronObject *GodNero,NeuronObject *newObj);

nero_s32int nero_addNeroIntoBaseObj(NeuronObject *BaseObi,NeuronObject *newObj);
/*从俩个已知道俩个概念中生成一个新的概念，新概念的种类在函数内部自动判断，最后返回新对象指针*/
NeuronObject * nero_createObjFromPair(NeuronObject *Obi1,NeuronObject *Obj2);
/*从多个>=3已知道俩个概念中生成一个新的概念，新概念的种类在函数内部自动判断，最后返回新对象指针*/
/**/
NeuronObject * nero_createObjFromMultiples(NeuronObject *Obis[],nero_s32int objNum);
/*判断是否已经从俩个已知道俩个概念中生成一个了新的概念,有则返回这个对象*/
NeuronObject *  nero_findSameObjFromPair(NeuronObject *Obi1,NeuronObject *Obj2);

/*根据dataKind概念的种类，在增加一个特定种类的数据，在这个函数里面不需要判断是否已经存在这个概念*/
NeuronObject *  nero_addNeroByData(void *Data,nero_s32int dataKind);

/*增强俩个对象的链接强度，如果没有连接就添加一个*/
nero_s32int nero_StrengthenLink(NeuronObject * a,NeuronObject * b);

/*判断tmpFiber2指向的对象是否一个词的概念，并且这个词由Obis里面的字，依次组成*/
/*返回1表示是*/
nero_s32int  nero_ifHasThisData_word(NeuronObject *obj,NeuronObject *childred[],nero_s32int objNum);
/*将词的链表中的每个词加入网络*/
NeuronObject *  nero_AddWordsIntoNet2(NeuronObject *GodNero,NeuronObject  *str[400],nero_s32int strLen);
/*将词的链表中的每个词加入网络*/
nero_s32int  nero_AddWordsIntoNet(NeuronObject *GodNero,Utf8Word * wordsHead);
/*根据给定数据寻找是否网络中已经有该   字   概念了，这里只搜索一个字,找到则返回该概念的指针*/
/*kind  控制搜索的领域*/
NeuronObject * nero_IfHasZhWord(NeuronObject *GodNero,ChUTF8 * word,nero_s32int kind);

nero_s32int nero_addZhCharIntoNet(NeuronObject *GodNero,ChUTF8 chChar[],nero_s32int charCounts);
/*判断是否有这个数据相应类型的概念*/
NeuronObject *nero_IfHasNeuronObject(void *Data,nero_s32int dataKind,NeuronObject *GodNero);


 inline  nero_us32int nero_GetNeroKind(ActNero * nero);
 inline  void  nero_putDataIntoNero(ActNero *n,nero_us32int x,nero_us32int y,nero_us32int z);
#define  nero_printNeroMsg(n)   (printf("kind=%d,xyz=%x %x %x,\n",nero_GetNeroKind(n),n->x,n->y,n->z   )  )
/*判断这俩个概念是不是在网络中存在，如果不存在，0，在返回1*/
nero_s32int nero_isInNet(NeuronObject *Obi);
/*判断是不是基类*/
nero_s32int  nero_isBaseObj(NeuronObject *Obi);
/*判断该神经中的数据是否是指定的数据*/
inline nero_s32int  nero_ifHasThisData(ActNero * nero,nero_s32int x,nero_s32int y,nero_s32int z);


nero_s32int   nero_IfHasObjFromMultiples(NeuronObject *Obis[],nero_s32int objNum);
NeuronObject *   nero_IfHasObjFromMultiples2(NeuronObject *Obis[],nero_s32int objNum);
NeuronObject *   nero_IfHasObjFromMultiples4(NeuronObject *Obis[],nero_s32int objNum);

nero_s32int nero_IfIsThisKind(NeuronObject *Obis[],nero_s32int objNum,NeuronObject * baseKindObj);
NeuronObject * nero_CreateNewBaseObj(NeuronObject * objs[],nero_s32int objNum,NeuronObject  *godNero,NeroConf * conf);
NeuronObject *  getBaseObjName(NeuronObject * baseobj,NeuronObject *);
nero_s32int  nero_ifMakeUpWithTheseObjsInOrder(NeuronObject *obj,NeuronObject *childred[],nero_s32int objNum);
NeuronObject * nero_ModifyBaseKind(NeuronObject * objs[],nero_s32int objNum,NeuronObject  *godNero,NeroConf * conf);



/*遍历网络的代码：*/


/*	nero_us8int tmp;*/
/*	nero_s32int ObjectKind;*/
/*	NeuronObject * BaseObi;*/
/*	NeuronObject * tmpObi;*/
/*	NerveFiber  *  curFiber;*/
/*	NerveFiber  *  outputFiberOfbaseObj;*/
/*	*/
/*	curFiber=GodNero->outputListHead;*/
/*	for (;curFiber !=NULL;curFiber=curFiber->next)*/
/*	{*/
/*		//首先遍历GodNero指向的基类*/
/*		*/
/*		BaseObi=curFiber->obj;*/
/*		ObjectKind=nero_GetNeroKind(BaseObi);*/
/*			*/
/*		//现在遍历每个基类下面的数据：*/
/*		*/
/*		outputFiberOfbaseObj=BaseObi->outputListHead;*/
/*		while(outputFiberOfbaseObj)*/
/*		{*/
/*			tmpObi=outputFiberOfbaseObj->obj;*/
/*			ObjectKind=nero_GetNeroKind(tmpObi);	*/
/*					*/
/*			NeuronObject * tmp=tmpObi->inputListHead->obj;*/

/*			outputFiberOfbaseObj=outputFiberOfbaseObj->next;*/
/*		}*/
/*		*/
/*		*/
/*		*/
/*	}*/
















#endif
