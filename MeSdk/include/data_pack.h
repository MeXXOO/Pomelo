#ifndef     _ME_DATA_PACK_H_
#define     _ME_DATA_PACK_H_

#define     PackCmd_Data   1   /* 数据包中的内容为对方发送的数据 */
#define     PackCmd_Ack    2   /* 数据包中的内容为对方收到该数据包的回应 */

     


typedef struct _IMeDataPacket
{
    uint32_t nPacketID;             /* 数据包ID */
    IMeArray* mListSubPacket;   /* 子数据包列表(子数据包索引index作排序) */
}IMeDataPacket;


/* 数据包拆包后的格式 */
/* | IMePackHeader-[24Bytes] | RawData-[nPacketMaxSize Bytes] | */
IME_EXTERN_C	IMeDataPacket*  IMeDataCreatePacket( IMeMemory* pMemory , uint32_t nPacketID );
IME_EXTERN_C	void            IMeDataDestroyPacket( IMeMemory* pMemory , IMeDataPacket* pDataPacket , uint8_t bSnd );
IME_EXTERN_C	IMeDataPacket*  IMeDataUnPack( IMeMemory* pMemory , uint32_t nPacketID , int nPacketMaxSize , char* lpData , int nLen );
IME_EXTERN_C	uint8_t         IMeDataPack( IMeMemory* pMemory , IMeDataPacket* pDataPacket , char** ppData , int* nLen );

#endif
