#ifndef     _ME_SOCKET_EVENT_H_
#define     _ME_SOCKET_EVENT_H_

#define		SEvent_All			(SEvent_Read|SEvent_Write)
#define     SEvent_Read			1<<0     /* socket �ɶ��¼� */
#define     SEvent_Write		1<<1     /* socket ��д�¼� */
#define		SEvent_Exception	1<<2	 /* socket �쳣�¼� */
#define     SEvent_Signal		1<<3     /* socket �ź��¼� */
#define		SEvent_Delete		1<<4	 /* socket ɾ���¼� */

#endif      //end _ME_SOCKET_EVENT_H_
