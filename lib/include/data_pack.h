#ifndef     _ME_DATA_PACK_H_
#define     _ME_DATA_PACK_H_

#define     PackCmd_Data   1   /* ���ݰ��е�����Ϊ�Է����͵����� */
#define     PackCmd_Ack    2   /* ���ݰ��е�����Ϊ�Է��յ������ݰ��Ļ�Ӧ */

     


typedef struct _IMeDataPacket
{
    uint32_t nPacketID;             /* ���ݰ�ID */
    IMeArray* mListSubPacket;   /* �����ݰ��б�(�����ݰ�����index������) */
}IMeDataPacket;


/* ���ݰ������ĸ�ʽ */
/* | IMePackHeader-[24Bytes] | RawData-[nPacketMaxSize Bytes] | */
IME_EXTERN_C	IMeDataPacket*  IMeDataCreatePacket( IMeMemory* pMemory , uint32_t nPacketID );
IME_EXTERN_C	void            IMeDataDestroyPacket( IMeMemory* pMemory , IMeDataPacket* pDataPacket , uint8_t bSnd );
IME_EXTERN_C	IMeDataPacket*  IMeDataUnPack( IMeMemory* pMemory , uint32_t nPacketID , int nPacketMaxSize , char* lpData , int nLen );
IME_EXTERN_C	uint8_t         IMeDataPack( IMeMemory* pMemory , IMeDataPacket* pDataPacket , char** ppData , int* nLen );

#endif
