#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>

int com_fd;

//���ڽṹ
typedef struct{
	char	prompt;		//prompt after reciving data
	int 	baudrate;		//baudrate
	char	databit;		//data bits, 5, 6, 7, 8
	char 	debug;		//debug mode, 0: none, 1: debug
	char 	echo;			//echo mode, 0: none, 1: echo
	char	fctl;			//flow control, 0: none, 1: hardware, 2: software
	char	parity;		//parity 0: none, 1: odd, 2: even
	char	stopbit;		//stop bits, 1, 2
	const int reserved;	//reserved, must be zero
}portinfo_t;
typedef portinfo_t *pportinfo_t;


/*******************************************
 *  ������ת����������ȷ���Ƿ���ȷ��
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
 *  fdcom: �����ļ���������pportinfo: �����õĶ˿���Ϣ����ȷ�ϣ�
 *
********************************************/
int PortSet(int fdcom, const pportinfo_t pportinfo)
{
    struct termios termios_old, termios_new;
    int     baudrate, tmp;
    char    databit, stopbit, parity, fctl;

    memset(&termios_old,0, sizeof(termios_old));
    memset(&termios_new,0, sizeof(termios_new));
    cfmakeraw(&termios_new);
    tcgetattr(fdcom, &termios_old);         //get the serial port attributions
    /*------------���ö˿�����----------------*/
    //baudrates
    baudrate = convbaud(pportinfo -> baudrate);
    cfsetispeed(&termios_new, baudrate);        //���봮������˵Ĳ�����
    cfsetospeed(&termios_new, baudrate);        //���봮������˵Ĳ�����
    termios_new.c_cflag |= CLOCAL;          //����ģʽ����֤���򲻻��Ϊ�˿ڵ�ռ����
    termios_new.c_cflag |= CREAD;           //����ģʽ��ʹ�ܶ˿ڶ�ȡ���������

    // ����ģʽ��flow control
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

    //����ģʽ��data bits
    termios_new.c_cflag &= ~CSIZE;      //����ģʽ�������ַ���Сλ
    databit = pportinfo -> databit;
    switch(databit){
        case '5':
            termios_new.c_cflag |= CS5;
        case '6':
            termios_new.c_cflag |= CS6;
        case '7':
            termios_new.c_cflag |= CS7;
        default:
            termios_new.c_cflag |= CS8;
    }

    //����ģʽ parity check
    parity = pportinfo -> parity;
    switch(parity){
        case '0':{
            termios_new.c_cflag &= ~PARENB;     //no parity check
        }break;
        case '1':{
            termios_new.c_cflag |= PARENB;      //odd check
            termios_new.c_cflag |= PARODD;
        }break;
        case '2':{
            termios_new.c_cflag |= PARENB;      //even check
            termios_new.c_cflag &= ~PARODD;
        }break;
    }

    //����ģʽ��stop bits
    stopbit = pportinfo -> stopbit;
    if(stopbit == '2'){
        termios_new.c_cflag |= CSTOPB;  //2 stop bits
    }
    else{
        termios_new.c_cflag &= ~CSTOPB; //1 stop bits
    }

    //other attributions default
    termios_new.c_oflag &= ~OPOST;          //���ģʽ��ԭʼ�������
    termios_new.c_cc[VMIN]  = 1;            //�����ַ�, ��Ҫ��ȡ�ַ�����С����
    termios_new.c_cc[VTIME] = 1;            //�����ַ�, ��ȡ��һ���ַ��ĵȴ�ʱ��    unit: (1/10)second

    tcflush(fdcom, TCIFLUSH);               //��������ݿ��Խ��գ�������
    tmp = tcsetattr(fdcom, TCSANOW, &termios_new);  //���������ԣ�TCSANOW�����иı�������Ч    tcgetattr(fdcom, &termios_old);
    return(tmp);
}

/********************************************
 *  send data
 *  fdcom: ������������data: ���������ݣ�datalen: ���ݳ���
 *  ����ʵ�ʷ��ͳ���
*********************************************/
int PortSend(int fdcom, char *data, int datalen)
{
    int len = 0;

    len = write(fdcom, data, datalen);  //ʵ��д��ĳ���
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
 *  ����ʵ�ʶ�����ֽ���
 *
********************************************/
int PortRecv(int fdcom, char *data, int datalen)
{
    int readlen, fs_sel;
    fd_set  fs_read;
    struct timeval tv_timeout;

    FD_ZERO(&fs_read);
    FD_SET(fdcom, &fs_read);
    tv_timeout.tv_sec = 10;
    tv_timeout.tv_usec = 0;

    fs_sel = select(fdcom+1, &fs_read, NULL, NULL, &tv_timeout);
    if(fs_sel){
        readlen = read(fdcom, data, datalen);
        return(readlen);
    }
    else{
        return(-1);
    }

    return (readlen);
}

#if 0
int main(int argc, char *argv[])
{
    char chn[20];
	int Txlen,i;
	int tmpData[100],k;
	char sendData[100];
	int RxLen;
	char rxData[100];
	
	portinfo_t portinfo ={   
        '0',                            // print prompt after receiving   
         9600,                         // baudrate: 9600   
        '8',                            // databit: 8   
        '0',                            // debug: off   
        '0',                            // echo: off   
        '2',                            // flow control: software     
        '0',                            // parity: none   
        '1',                            // stopbit: 1   
         0                          // reserved   
  };
	
	int ch;
	
	while ((ch = getopt(argc,argv,"c:b:d:f:p:s:e:h")) != -1) {
		printf("optind:%d optarg:%s\n",optind,optarg);
		switch(ch) {
			case 'h':
			{
			    printf("-c  : ��������\n");
				printf("-b  : ������\n");
				printf("-d  : debug\n");
				printf("-f  : ������\n");
				printf("-e  : echo\n");
				printf("-s  : ֹͣλ\n");
				printf("-p  : У�鷽ʽ\n");
				printf("-h  : ������Ϣ\n");
				return 0;
			}
			break;
			case 'c':
			strcpy(chn,optarg);
			break;
			case 'b':
			portinfo.baudrate=atoi(optarg);
			break;
			case 'd':
			portinfo.databit='0'+atoi(optarg);
			break;
			case 'f':
			portinfo.fctl='0'+atoi(optarg);
			break;
			case 'e':
			portinfo.echo='0'+atoi(optarg);
			break;
			case 's':
			portinfo.stopbit='0'+atoi(optarg);
			break;
			case 'p':
			portinfo.parity='0'+atoi(optarg);
			break;
		}
			
	} 
	
	printf("COM=%s,baud=%d,parity=%c,databit=%c,stopbit=%c,flow=%c\n",
	chn,portinfo.baudrate,portinfo.parity,portinfo.databit,
	portinfo.stopbit,portinfo.fctl);
	
	com_fd = open(chn, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(com_fd<0)
  {
		printf("Open Com %s failed,exit!\n",chn);
	  exit(1);
  }
  
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
	else 
	    printf("recv no data");
	close(com_fd);
	return 0;
}
#endif
