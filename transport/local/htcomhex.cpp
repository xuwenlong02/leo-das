#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "htcomhex.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <logger.h>
#include <string.h>


/*******************************************
 *  波特率转换函数（请确认是否正确）
********************************************/
int convbaud(unsigned long int baudrate)
{
    switch(baudrate){
        case 300:
            return B300;
        case 600:
            return B600;
        case 1200:
            return B1200;
        case 2400:
            return B2400;
        case 4800:
            return B4800;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        default:
            return B9600;
    }
}

/*******************************************
 *  Setup comm attr
 *  fdcom: 串口文件描述符，pportinfo: 待设置的端口信息（请确认）
 *
********************************************/
int PortSet(int fdcom, const pportinfo_t pportinfo)
{
    struct termios termios_old, termios_new;
    int     baudrate, tmp;
    char    databit, stopbit, parity, fctl;

    bzero(&termios_old, sizeof(termios_old));
    bzero(&termios_new, sizeof(termios_new));
    cfmakeraw(&termios_new);
    tcgetattr(fdcom, &termios_old);         //get the serial port attributions
    /*------------设置端口属性----------------*/
    //baudrates

    tcflush(fdcom, TCIOFLUSH);
    baudrate = convbaud(pportinfo -> baudrate);
    cfsetispeed(&termios_new, baudrate);        //填入串口输入端的波特率
    cfsetospeed(&termios_new, baudrate);        //填入串口输出端的波特率
    tcflush(fdcom, TCIOFLUSH);

    termios_new.c_cflag |= CLOCAL;          //控制模式，保证程序不会成为端口的占有者
    termios_new.c_cflag |= CREAD;           //控制模式，使能端口读取输入的数据 

#if 1
    //by xuwenlong
//    termios_new.c_lflag &=~(ECHO   |   ICANON   |   IEXTEN   |   ISIG);
//    termios_new.c_iflag   &=   ~(BRKINT   |   ICRNL   |   INPCK   |   ISTRIP   |   IXON);
    // 控制模式，flow control
    fctl = pportinfo-> fctl;
    switch(fctl){
        case '0':{
            termios_new.c_cflag &= ~CRTSCTS;        //no flow control
        }break;
        case '1':{
            termios_new.c_cflag |= CRTSCTS;         //hardware flow control
        }break;
        case '2':{
            termios_new.c_iflag |= IXON | IXOFF |IXANY; //software flow control
        }break;
    }
#endif

    //控制模式，data bits
    termios_new.c_cflag &= ~CSIZE;      //控制模式，屏蔽字符大小位
    databit = pportinfo -> databit;
    switch(databit){
        case '5':
            termios_new.c_cflag |= CS5;
            break;
        case '6':
            termios_new.c_cflag |= CS6;
            break;
        case '7':
            termios_new.c_cflag |= CS7;
            break;
        default:
            termios_new.c_cflag |= CS8;
    }

    //控制模式 parity check
    parity = pportinfo -> parity;
    switch(parity){
        case '0':{
            termios_new.c_cflag &= ~PARENB;     //no parity check
        }break;
        case '1':{
            termios_new.c_cflag |= PARENB;      //odd check
            termios_new.c_cflag |= PARODD;
//            termios_new.c_iflag |= INPCK|ISTRIP;
        }break;
        case '2':{
            termios_new.c_cflag |= PARENB;      //even check
            termios_new.c_cflag &= ~PARODD;
//            termios_new.c_iflag |= INPCK|ISTRIP;
        }break;
    }

    //控制模式，stop bits
    stopbit = pportinfo -> stopbit;
    if(stopbit == '1'){
        termios_new.c_cflag &= ~CSTOPB;  //2 stop bits
    }
    else{
        termios_new.c_cflag |= CSTOPB; //1 stop bits
    }

    //other attributions default
    termios_new.c_oflag &= ~OPOST;          //输出模式，原始数据输出   delete by @xuwenlong
    termios_new.c_cc[VMIN]  = 1;            //控制字符, 所要读取字符的最小数量  modify by @xuwenlong
    termios_new.c_cc[VTIME] = 1;            //控制字符, 读取第一个字符的等待时间    unit: (1/10)second

    tcflush(fdcom, TCIFLUSH);               //溢出的数据可以接收，但不读
    tmp = tcsetattr(fdcom, TCSANOW, &termios_new);  //设置新属性，TCSANOW：所有改变立即生效    tcgetattr(fdcom, &termios_old);
    return(tmp);
}

void FlushBuffer(int fdcom,int flag)
{
    tcflush(fdcom,flag==0?TCIFLUSH:TCOFLUSH);
}

/********************************************
 *  send data
 *  fdcom: 串口描述符，data: 待发送数据，datalen: 数据长度
 *  返回实际发送长度
*********************************************/
int PortSend(int fdcom,const char *data, int datalen)
{
    int len = 0;

    len = write(fdcom, data, datalen);  //实际写入的长度
    if(len == datalen){
        return (len);
    }
    else{
        tcflush(fdcom, TCOFLUSH);
        return -1;
    }
}

/*******************************************
 *  receive data
 *  返回实际读入的字节数
 *
********************************************/
int PortRecv(int fdcom, char *data, int datalen)
{
    return read(fdcom, data, datalen);
}

/*******************************************
 *  open port
 *
********************************************/
int OpenPort(const char *com)
{
    int fd;

//    fd = open(com,O_RDWR | O_NOCTTY | O_NONBLOCK);O_NDELAY//O_NONBLOCK
    fd = open(com,O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (fd < 0)
        return fd;
    if (fcntl(fd, F_SETFL, 0)<0)
        return -1;
    return fd;
}

/*******************************************
 *  close port
 *
********************************************/
void ClosePort(int com_fd)
{
    close(com_fd);
}

#ifdef TRANSPORT_TEST
int main(int argc, char *argv[])
{
    
	int Txlen,i;
	int tmpData[100],k;
	char sendData[100];
	int RxLen;
	char rxData[100];
    int com_fd;
	
	portinfo_t portinfo ={   
        '0',                            // print prompt after receiving   
        9600,                         // baudrate: 9600   
        '8',                            // databit: 8   
        '0',                            // debug: off   
        '0',                            // echo: off   
        '0',                            // flow control: none     
        '0',                            // parity: none   
        '1',                            // stopbit: 1   
         0                          // reserved   
  }; 
	
	if(argc != 3)
	{
		printf("Wrong argc number,exit!\n");
		exit(1);
	}	

	com_fd = open(argv[1], O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(com_fd<0)
  {
		printf("Open Com failed,exit!\n");
	  exit(1);
  }
  
  portinfo.baudrate = atoi(argv[2]);
  PortSet(com_fd, &portinfo);
	
	printf("Please input send data in Hex:\n");

  for(i=0;i<100;i++)
  {
          
		scanf("%x",&tmpData[i]);
    k=getchar();
    if(k=='\n')
    {
     	Txlen=i+1;
      break;
    }
  }
  
  printf("Prepare to send data:\n");
  for(i=0;i<Txlen;i++)
  {
  	printf("%02x ",tmpData[i]);
  }
  printf("\n");
  
  for(i=0;i<Txlen;i++)
  {
  	sendData[i] = (unsigned char)tmpData[i];
  }
  
  PortSend(com_fd,sendData,Txlen);
  
  printf("Receive data in Hex:\n");
  
  while(1)
  {
  	RxLen = PortRecv(com_fd,rxData,100);
  	//printf("recv %d data \n",RxLen);
  	if(RxLen > 0)
  	{
  		//printf("recv %d data \n",RxLen);
  		for(i=0;i<RxLen;i++)
  		{
  			printf("%02x ",rxData[i]);
  			fflush(stdout);
  		}
  	}
	}
}
#endif
