/*************************************************************************
	> File Name: htcomhex.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月16日 星期二 18时27分00秒
 ************************************************************************/
#ifndef __HTCOMHEX_H__
#define __HTCOMHEX_H__

//串口结构
typedef struct{
    char	prompt;		//prompt after reciving data
    int 	baudrate;		//baudrate
    char	databit;		//data bits, 5, 6, 7, 8
    char 	debug;		//debug mode, 0: none, 1: debug
    char 	echo;			//echo mode, 0: none, 1: echo
    char	fctl;			//flow control, 0: none, 1: hardware, 2: software
    char	parity;		//parity 0: none, 1: odd, 2: even
    char	stopbit;		//stop bits, 1, 2
    int     reserved;	//reserved, must be zero
}portinfo_t;
typedef portinfo_t * pportinfo_t;

/*******************************************
 *  波特率转换函数（请确认是否正确）
********************************************/
extern int convbaud(unsigned long int baudrate);

/*******************************************
 *  Setup comm attr
 *  fdcom: 串口文件描述符，pportinfo: 待设置的端口信息（请确认）
 *
********************************************/
extern int PortSet(int fdcom, const pportinfo_t pportinfo);

/*******************************************
 *  flush buffer
 *  fdcom: 串口文件描述符，pportinfo: 待设置的端口信息（请确认）
 *  flag : 操作标志，0清除写缓存，1清除读缓存
********************************************/
extern void FlushBuffer(int fdcom,int flag);

/********************************************
 *  send data
 *  fdcom: 串口描述符，data: 待发送数据，datalen: 数据长度
 *  返回实际发送长度
*********************************************/
extern int PortSend(int fdcom, const char *data, int datalen);


/*******************************************
 *  receive data
 *  返回实际读入的字节数
 *
********************************************/
extern int PortRecv(int fdcom, char *data, int datalen);

/*******************************************
 *  open port
 *
********************************************/
extern int OpenPort(const char *com);

/*******************************************
 *  close port
 *
********************************************/
extern void ClosePort(int com_fd);

#endif
