#ifndef     _ME_SOCKET_EVENT_H_
#define     _ME_SOCKET_EVENT_H_

#define		SEvent_All			(SEvent_Read|SEvent_Write)
#define     SEvent_Read			1<<0     /* socket 可读事件 */
#define     SEvent_Write		1<<1     /* socket 可写事件 */
#define		SEvent_Exception	1<<2	 /* socket 异常事件 */
#define     SEvent_Signal		1<<3     /* socket 信号事件 */
#define		SEvent_Delete		1<<4	 /* socket 删除事件 */

#endif      //end _ME_SOCKET_EVENT_H_
