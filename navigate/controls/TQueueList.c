#include <AEEStdLib.h>

#include "TQueueList.h"


boolean TQueueList_Init(TQueueList **pTQueueList)
{
	TQueueList *pHead;

	pHead = *pTQueueList;
	if (pHead != NULL)
	{
		FREE(pHead);
		pHead = NULL;
	}

// 	pHead = MALLOC(sizeof(TQueueList));
// 	if (pHead == NULL)
// 	{
// 		DBGPRINTF("malloc failed!");
// 		return FALSE;
// 	}
// 
// 	MEMSET(pHead, 0, sizeof(TQueueList));
// 
// 	*pTQueueList = pHead;

    DBGPRINTF("Init Success!");

	return TRUE;
}

void TQueueList_Destroy(TQueueList **pTQueueList)
{
	TQueueList *pHead;
	TQueueList *pTmp;
    TQueueList *ppNext;            //定义一个与pHead相邻节点

	pHead = pTmp = *pTQueueList;

    if(pHead == NULL)
    {
        DBGPRINTF("clearList函数执行，链表为空\n");
        return;
    }

    while(pHead->pNext != NULL)
    {
		ppNext = pHead->pNext;
		
		if (pHead->pData)
		{
			FREE(pHead->pData);
			pHead->pData = NULL;
		}

        FREE(pHead);
        pHead = ppNext;
    }

	if (pHead->pData)
	{
		FREE(pHead->pData);
		pHead->pData = NULL;
	}

	//FREE(pTmp);
	//pTmp = NULL;

	*pTQueueList = NULL;

    DBGPRINTF("TQueueList_Destroy Over!");
}

int TQueueList_GetCount(TQueueList *pHead)
{
	int size = 0;

    while(pHead != NULL)
    {
        size ++;         //遍历链表size大小比链表的实际长度小1
        pHead = pHead->pNext;
    }

    DBGPRINTF("List Count:%d", size);

    return size;    //链表的实际长度
}


int TQueueList_Insert(TQueueList **pTQueueList, elemType elem)
{
    TQueueList *pInsert;
    TQueueList *pHead;
    TQueueList *pTmp; 
	int		pos = 0;

    pHead = *pTQueueList;
    pTmp = pHead;

    pInsert = (TQueueList *)MALLOC(sizeof(TQueueList)); 
    MEMSET(pInsert,0,sizeof(TQueueList));
	pInsert->pData = elem;

	if (pHead == NULL)
	{
		pHead = pInsert;
		*pTQueueList = pHead;
		return 0;
	}

	pos ++;

    while(pHead->pNext != NULL)
    {
        pHead = pHead->pNext;
    }

    pHead->pNext = pInsert;
    *pTQueueList = pTmp;

    DBGPRINTF("insert with pos :%d", pos);

    return pos;
}

elemType TQueueList_GetElem(TQueueList *pHead, uint32 pos)
{
	uint32 i = 0;

    if(pHead == NULL)
    {
        DBGPRINTF("LIST IS EMPTY");
        return NULL;
    }

    while(pHead !=NULL)
    {
        if(i == pos)
            break;

		i ++;
        pHead = pHead->pNext;
    }

    if(i < pos)
    {
        DBGPRINTF("NOT FOUND");
        return NULL;
    } 

    return pHead->pData;
}

void TQueueList_Print(TQueueList *pHead)
{
    if(NULL == pHead)
    {
        DBGPRINTF("LIST IS NULL!");
    }
    else
    {
        while(NULL != pHead)
        {
            DBGPRINTF("%x ",pHead->pData);
            pHead = pHead->pNext;
        }
    }
}

// 
// /* 2.创建线性表，此函数输入负数终止读取数据*/
// TQueueList *creatList(TQueueList *pHead)
// {
//     TQueueList *p1;
//     TQueueList *p2;
// 
//     p1=p2=(TQueueList *)malloc(sizeof(TQueueList)); //申请新节点
// 
//     if(p1 == NULL || p2 ==NULL)
//     {
//         DBGPRINTF("内存分配失败\n");
// 		return NULL;
//     }
// 
//     memset(p1,0,sizeof(TQueueList));
// 
//     scanf("%d",&p1->pData);    //输入新节点
// 
//     p1->pNext = NULL;         //新节点的指针置为空
// 
//     while(p1->pData > 0)        //输入的值大于0则继续，直到输入的值为负
//     {
//         if(pHead == NULL)       //空表，接入表头
//         {
//             pHead = p1;
//         }
//         else               
//         {
//             p2->pNext = p1;       //非空表，接入表尾
//         }
// 
//         p2 = p1;
//         p1=(TQueueList *)malloc(sizeof(TQueueList));    //再重申请一个节点
//         if(p1 == NULL || p2 ==NULL)
//         {
// 			DBGPRINTF("内存分配失败\n");
// 			return NULL;
//         }
//         memset(p1,0,sizeof(TQueueList));
//         scanf("%d",&p1->pData);
//         p1->pNext = NULL;
// 
//     }
// 
//     DBGPRINTF("creatList函数执行，链表创建成功\n");
// 
//     return pHead;           //返回链表的头指针
// }
// 
//  
// 
// /* 3.打印链表，链表的遍历*/
// void printList(TQueueList *pHead)
// {
//     if(NULL == pHead)   //链表为空
//     {
//         DBGPRINTF("PrintList函数执行，链表为空\n");
//     }
//     else
//     {
//         while(NULL != pHead)
//         {
//             DBGPRINTF("%d ",pHead->pData);
//             pHead = pHead->pNext;
//         }
//         DBGPRINTF("\n");
//     }
// 
// }
// 
//  
// 
// /* 4.清除线性表L中的所有元素，即释放单链表L中所有的结点，使之成为一个空表 */
// void clearList(TQueueList *pHead)
// {
//     TQueueList *ppNext;            //定义一个与pHead相邻节点
// 
//     if(pHead == NULL)
//     {
//         DBGPRINTF("clearList函数执行，链表为空\n");
//         return;
//     }
// 
//     while(pHead->pNext != NULL)
//     {
//         ppNext = pHead->pNext;//保存下一结点的指针
//         free(pHead);
//         pHead = ppNext;      //表头下移
//     }
// 
//     DBGPRINTF("clearList函数执行，链表已经清除\n");
// }
// 
//  
// 
// /* 5.返回单链表的长度 */
// int sizeList(TQueueList *pHead)
// {
// 	int size = 0;
// 
//     while(pHead != NULL)
//     {
//         size++;         //遍历链表size大小比链表的实际长度小1
//         pHead = pHead->pNext;
//     }
// 
//     DBGPRINTF("sizeList函数执行，链表长度 %d \n",size);
// 
//     return size;    //链表的实际长度
// }
// 
// /* 6.检查单链表是否为空，若为空则返回１，否则返回０ */
// int isEmptyList(TQueueList *pHead)
// {
//     if(pHead == NULL)
//     {
//         DBGPRINTF("isEmptyList函数执行，链表为空\n");
//         return 1;
//     }
// 
// 	DBGPRINTF("isEmptyList函数执行，链表非空\n");
// 
//     return 0;
// }
// 
// /* 7.返回单链表中第pos个结点中的元素，若pos超出范围，则停止程序运行 */
// elemType getpData(TQueueList *pHead, int pos)
// {
//     int i=0;
// 
// 	if(pos < 1)
//     {
//         DBGPRINTF("getpData函数执行，pos值非法\n");
//         return 0;
//     }
// 
//     if(pHead == NULL)
//     {
//         DBGPRINTF("getpData函数执行，链表为空\n");
//         return 0;
//         //exit(1);
//     }
// 
//     while(pHead !=NULL)
//     {
//         ++i;
//         if(i == pos)
//         {
//             break;
//         }
// 
//         pHead = pHead->pNext; //移到下一结点
//     }
// 
//     if(i < pos)                  //链表长度不足则退出
//     {
//         DBGPRINTF("getpData函数执行，pos值超出链表长度\n");
//         return 0;
//     } 
// 
//     return pHead->pData;
// }
// 
//  
// 
// /* 8.从单链表中查找具有给定值x的第一个元素，若查找成功则返回该结点pData域的存储地址，否则返回NULL */
// elemType *getElemAddr(TQueueList *pHead, elemType x)
// {
//     if(NULL == pHead)
//     {
//         DBGPRINTF("getElemAddr函数执行，链表为空\n");
//         return NULL;
//     }
// 
//     if(x < 0)
//     {
//         DBGPRINTF("getElemAddr函数执行，给定值X不合法\n");
//         return NULL;
//     }
// 
//     while((pHead->pData != x) && (NULL != pHead->pNext)) //判断是否到链表末尾，以及是否存在所要找的元素
//     {
//         pHead = pHead->pNext;
//     }
// 
//     if((pHead->pData != x) && (pHead != NULL))
//     {
//         DBGPRINTF("getElemAddr函数执行，在链表中未找到x值\n");
//         return NULL;
//     }
// 
//     if(pHead->pData == x)
//     {
//         DBGPRINTF("getElemAddr函数执行，元素 %d 的地址为 0x%x\n",x,&(pHead->pData));
//     }
// 
//     return &(pHead->pData);//返回元素的地址
// }
// 
//  
// 
// /* 9.把单链表中第pos个结点的值修改为x的值，若修改成功返回１，否则返回０ */
// int modifyElem(TQueueList *pTQueueList,int pos,elemType x)
// {
//     TQueueList *pHead;
//     pHead = pTQueueList;
//     int i = 0; 
// 
//     if(NULL == pHead)
//     {
//         DBGPRINTF("modifyElem函数执行，链表为空\n");
//     }
// 
//     if(pos < 1)
//     {
//         DBGPRINTF("modifyElem函数执行，pos值非法\n");
//         return 0;
//     }
// 
//     while(pHead !=NULL)
//     {
//         ++i;
//         if(i == pos)
//             break;
// 
//         pHead = pHead->pNext; //移到下一结点
// 
//     }
// 
//     if(i < pos)                  //链表长度不足则退出
//     {
//         DBGPRINTF("modifyElem函数执行，pos值超出链表长度\n");
//         return 0;
//     }
// 
//     pTQueueList = pHead;
//     pTQueueList->pData = x;
//     DBGPRINTF("modifyElem函数执行\n");    
// 
//     return 1;
// 
// }
// 
//  
// 
// /* 10.向单链表的表头插入一个元素 */
// int insertHeadList(TQueueList **pTQueueList,elemType insertElem)
// {
//     TQueueList *pInsert;
// 
//     pInsert = (TQueueList *)malloc(sizeof(TQueueList));
//     memset(pInsert,0,sizeof(TQueueList));
//     pInsert->pData = insertElem;
//     pInsert->pNext = *pTQueueList;
//     *pTQueueList = pInsert;
//     DBGPRINTF("insertHeadList函数执行，向表头插入元素成功\n");
// 	
// 	return 1;
// }


/* 12.向单链表中第pos个结点位置插入元素为x的结点，若插入成功返回１，否则返回０ */
/* 13.向有序单链表中插入元素x结点，使得插入后仍然有序 */
/* 14.从单链表中删除表头结点，并把该结点的值返回，若删除失败则停止程序运行 */
/* 15.从单链表中删除表尾结点并返回它的值，若删除失败则停止程序运行 */
/* 16.从单链表中删除第pos个结点并返回它的值，若删除失败则停止程序运行 */
/* 17.从单链表中删除值为x的第一个结点，若删除成功则返回1,否则返回0 */
/* 18.交换2个元素的位置 */
/* 19.将线性表进行快速排序 */

/******************************************************************/
/*
int main()
{
    TQueueList *pList=NULL;
    int length = 0;
    elemType posElem;
    initList(&pList);       //链表初始化

    printList(pList);       //遍历链表，打印链表
    pList=creatList(pList); //创建链表
    printList(pList);
    sizeList(pList);        //链表的长度
    printList(pList);
    isEmptyList(pList);     //判断链表是否为空链表
    posElem = getpData(pList,3);  //获取第三个元素，如果元素不足3个，则返回0
    DBGPRINTF("getpData函数执行，位置 3 中的元素为 %d\n",posElem);   
    printList(pList);
    getElemAddr(pList,5);   //获得元素5的地址

    modifyElem(pList,4,1);  //将链表中位置4上的元素修改为1
    printList(pList);

    insertHeadList(&pList,5);   //表头插入元素12
    printList(pList);

    insertLastList(&pList,10);  //表尾插入元素10
    printList(pList);
    clearList(pList);       //清空链表
	system("pause");
}*/