/*************************************************************************
	> File Name: tcpserver.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月22日 星期一 15时27分51秒
 ************************************************************************/
#include <defines.h>
#include "../transport.h"
#include <utils/lock.h>
#include <utils/conditional_variable.h>
#include <vector>
#include <queue>
#include <logger.h>

using namespace std;

class RemoteConnect
{
public:
    RemoteConnect(TransportInterface *interface);
    virtual ~RemoteConnect();

    BOOL SetupConnection(socketinfo_t &si);
    BOOL ReConnect();
    void CloseConnection();
    BOOL IsConnected();

    void MonitorSocketRead();

    /*******************************************
      *发送数据
      *
     *******************************************/
    BOOL SendData(const char *pData, S32 length);
    BOOL RecvData();
private:
    S32 canSend();
    S32 canRead();
    BOOL sendData(const void* pData,S32 iLength);
private:
    LOG_MODULE_TRACE(RemoteConnect);
    S32                  m_iSockfd;
    socketinfo_t         m_iSockInfo;

//    Lock                 m_iSendLock;  /* send data lock */
    struct senddata_t{
    public:
        senddata_t(const char* buffer,int length) {
            len = length;
            pBuffer = malloc(length);
            memcpy(pBuffer,buffer,length);
        }

        senddata_t(const senddata_t& senddata) {
            len = 0;
            pBuffer = NULL;
            operator =(senddata);
        }

        senddata_t& operator=(const senddata_t& senddata) {
            if (pBuffer)
                free(pBuffer);
            len = senddata.len;
            pBuffer = malloc(len);
            memcpy(pBuffer,senddata.pBuffer,len);
            return *this;
        }

        virtual ~senddata_t() {
            if (pBuffer)
                free(pBuffer);
        }

        void *pBuffer;
        S32  len;
    };

    TransportInterface  *m_pTransportInterface;

    ConditionalVariable  m_iWriteCondition;
    S8                   m_pRecBuf[MAX_RECV_SIZE];
    S32                  m_iBufSize;

    Lock                 m_iConnLock;
    BOOL                 m_bIsConnected;
};
