/*************************************************************************
	> File Name: htcomhex.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年03月09日 星期五 10时16分14秒
 ************************************************************************/

#include <stdio.h>
#include <transport/transport.h>
#include <transport/local/localtask.h>
#include <utils/shellcmd.h>
#include <acquisition/modbus.h>
#include "htcomhex.h"

LocalTask *transport;
transmgr_t transmgr;

bool getcomport(const char *line, void *data);

int testcomm(int argc,char *argv[]) {
    transport = new LocalTask();

    transmgr.trans_mode = TM_SEARIAL;

    LOG_INFO("please input hex:");
    CmdValue cmd;

    U32 u = 0;
    int c = 0;
#if 1
    do{

        scanf("%02x",&u);
        cmd +=u;
    }while(((c=getchar())!='\n'));
    cmd.Show();
#endif

    ShellCmd(&getcomport).RunCmd(
                "dmesg | grep ttyS* | awk '{print $2}'",
                &cmd);

    return 0;
}

bool getcomport(const char *line,void *data) {
    CmdValue *cmd = (CmdValue*)data;
    sprintf(transmgr.trans_addr.portaddr.com,"/dev/%s",line);
    int bautrates[] = {600,1200,2400,4800,9600,19200,38400,57600,76800,115200};
    char veries[] = {0,1,2};
    char stopbits[] = {1,2};
    int bautesize = sizeof(bautrates)/sizeof(int);
    int vsize = sizeof(veries)/sizeof(char);
    int stopsize = sizeof(stopbits)/sizeof(char);

    transmgr.trans_addr.portaddr.databit = 8;
    for (int i = 0;i < bautesize;i++) {
        transmgr.trans_addr.portaddr.baudrate=bautrates[i];

        for (int j = 0;j < vsize; j++) {
            transmgr.trans_addr.portaddr.parity = veries[j];
            for (int k = 0;k<stopsize;k++) {
                transmgr.trans_addr.portaddr.stopbit = stopbits[k];

                LOG_DEBUG("baudrate = %d, databit = %d, parify = %d, stopbit = %d",
                          transmgr.trans_addr.portaddr.baudrate,
                          transmgr.trans_addr.portaddr.databit,
                          transmgr.trans_addr.portaddr.parity,
                          transmgr.trans_addr.portaddr.stopbit);

                tConnection pConn = transport->GetConnection(transmgr);
                if (!pConn.valid() || !pConn->IsConnected()){
                    LOG_DEBUG("connection %s invalid or connect failed",line);
                    continue;
                }
                pConn->SendInstruction(cmd->Cmd(),cmd->Length());
                U8 buffer[256] = {0};
                S32 length = 0;
                if (!pConn->ReceiveData(buffer,length)) {
                    LOG_DEBUG("receive failed,%d",length);
                    TRANS_PRINT(buffer,length);
                    continue;
                }
                LOG_DEBUG("receive success,%d",length);
                TRANS_PRINT(buffer,length);
                return false;
            }
        }
    }
    return true;
}
