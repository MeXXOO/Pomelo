#ifndef		_FILE_PROTOCOL_DEFINE_H_
#define		_FILE_PROTOCOL_DEFINE_H_

#include	"include.h"

#define		MAKE_TFILEH(app,dformat,cmd)	((int)app<<24|(int)dformat<<16|(int)cmd<<8)
#define		TFILEH_CMD(value)				((value>>8)&0xff)
#define		TFILEH_APP(value)				((value>>24)&0xff)
#define		TFILEH_DFORMAT(value)			((value>>16)&0xff)

/* TFileServer format: */
#define		TFILE_DFORMAT_JSON				1
#define		TFILE_DFORMAT_BINARY			2

/* TFileServer client App: */
#define		TFILE_APP_CLIENT				1
#define		TFILE_APP_SERVER				2

/* TFileServer command: */
#define		TFILE_C2S_ACCOUNT_VERIFY		(0)
#define		TFILE_S2C_ACCOUNT_VERIFY_ACK	(1)
#define		TFILE_C2S_ADD_FILES_INFO		(2)
#define		TFILE_S2C_ADD_FILE_INFO_ACK		(3)
#define		TFILE_C2S_ADD_FILE_START		(4)
#define		TFILE_S2C_ADD_FILE_START_ACK	(5)
#define		TFILE_C2S_ADD_FILE_DATA			(6)
#define		TFILE_C2S_ADD_FILE_END			(7)
#define		TFILE_S2C_ADD_FILE_END_ACK		(8)
#define		TFILE_C2S_ADD_FILES_OVER		(9)
#define		TFILE_C2S_ADD_FILES_CANCEL		(10)


/*	TFileServer Description As Follows:		
	****************************************************************************
	[SERVER]															[CLIENT]
	 |<----------------------(TFILE_C2S_ACCOUNT_VERIFY)-----------------------|
	 |-----------------------(TFILE_S2C_ACCOUNT_VERIFY_ACK)------------------>|
	 |<----------------------(TFILE_C2S_ADD_FILES_INFO)-----------------------|
	 |-----------------------(TFILE_S2C_ADD_FILE_INFO_ACK)------------------->|
	 |<----------------------(TFILE_C2S_ADD_FILE_START)-----------------------|
	 |<----------------------(TFILE_S2C_ADD_FILE_START_ACK)-------------------|
	 |<----------------------(TFILE_C2S_ADD_FILE_DATA)------------------------|
	 |<----------------------(TFILE_C2S_ADD_FILE_END)-------------------------|
	 |-----------------------(TFILE_S2C_ADD_FILE_END_ACK)-------------------->|
	 |<----------------------(TFILE_C2S_ADD_FILES_OVER)-----------------------|
	 |<----------------------(TFILE_C2S_ADD_FILE_CANCEL)----------------------|
	[SERVER]															[CLIENT]
	****************************************************************************
*/


/*	TFileServer	command explain:
	1.TFILE_C2S_ACCOUNT_VERIFY(for check user account):
	  [usernamelen-4Byte][username-raw][passwordlen-4Byte][password-raw]
	2.TFILE_S2C_ACCOUNT_VERIFY_ACK
	  [VerifyRes-1Byte][serverinfo-4Byte][serverinfo-raw]
    3.TFILE_C2S_ADD_FILE_INFO
	  [filetype] [[[filename][filesize][fileid]],[...],[...]]
	4.TFILE_ADD_FILE_INFO_ACK
	  [AckRes-1Byte]
	5.
*/

#endif		//end _FILE_PROTOCOL_DEFINE_H_
