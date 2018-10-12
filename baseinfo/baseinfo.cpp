/*************************************************************************
	> File Name: baseinfo.c
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 13时44分00秒
 ************************************************************************/

#include<stdio.h>
#include <defines.h>
#include <md5cpp/md5.h>
#include <utils/sqlite.h>
#include <utils/socket_request.h>
#include "static_config.h"
#include "baseinfo.h"
#include "ifconfig.h"
#include <sys/statfs.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <utils/config.h>
#include <acquisition/acquisition.h>
#include "sysinfo.h"
#include <fcntl.h>


/***************************************************
  *@函数:InitConfig
  *@作用：读网关信息
  *@返回：TRUE，获取成功；FALSE，获取失败
 **************************************************/

MODULE_DEFINE_BEGIIN(BaseInfo)
if (ret){
    InitMsg();
    m_pTimer->Start(10000,kPeriodic);
}
else{
    m_pTimer->Stop();
    m_bConnected = FALSE;
}
//m_pTimer->Notify();
MODULE_DEFINE_END

MODULE_NETWORK_BEGIN(BaseInfo)
if (isconected) {
    m_bConnected = TRUE;
}
else{
    AutoLock autolock(m_ilock);
    SetMsgStatus(DAS_REGISTERACQUISITOR,FALSE);
//    m_pTimer->Notify();
    m_bConnected = FALSE;
}
MODULE_NETWORK_END


BaseInfo::BaseInfo():
    m_pTimer(new Timer("baseinfo",this)),
    m_bConnected(FALSE)
{
    addReference();
//    if((m_ifdw=open("/dev/watchdog",O_RDWR))==-1)
//    {
//        LOG_ERR("Can not open /dev/watchdog\n");
//    }
//    CLOSE_WDT;
}

BaseInfo::~BaseInfo()
{
//    CLOSE_WDT;
    delete m_pTimer;
    dropReference();
}

void BaseInfo::RunTimer()
{
    if (m_bConnected) {
        AutoLock autolock(m_ilock);
        if (!GetMsgStatus(DAS_REGISTERACQUISITOR)) {
            RegisterAcquisitor();
        }
    }

//    m_pTimer->set_timeout(20000);
}

BOOL BaseInfo::IsGateWayIdEqual(const char *id)
{
    return (!strcmp(id,getConfigInfo("id").c_str()));
}

void BaseInfo::OnReceiveJson(DAS_RESULT result,const std::string &method, const Json::Value &json)
{
    if (method == "registerAcquisitor"){
        if (result == DAS_SUCCESS){
            if (json.isBool()) {
                m_ilock.Acquire();
                SetMsgStatus(DAS_REGISTERACQUISITOR,TRUE);
                LOG_DEBUG("Register Acquisitor success");
                m_ilock.Release();
            }
            else if ( json["ret"].asBool() == true){
                m_ilock.Acquire();
                SetMsgStatus(DAS_REGISTERACQUISITOR,TRUE);
                LOG_DEBUG("Register Acquisitor success");
                m_ilock.Release();

                string date = json["date"].asString();
                string script = "date -s '"+date+"'";

                SocketRequest().scriptRequest(script,1);
                SocketRequest().scriptRequest("hwclock -u -w",1);
                return;
            }
        }
        else {
            AutoLock autolock(m_ilock);
            SetMsgStatus(DAS_REGISTERACQUISITOR,FALSE);
            LOG_DEBUG("Register Acquisitor failed");
        }
//        m_pTimer->Notify();
    }
}

void BaseInfo::RegisterAcquisitor()
{
    Json::Value json;
    std::string strId = getConfigInfo("id");
    std::string strSecretkey = getConfigInfo("secretkey");

    json["company"]=utils::GetConfigFileStringValue("BASE","company","");
    json["gateway_id"]= strId.c_str();
    CMD5 md5;
    char md5buffer[MAX_ID_LEN+MAX_SECRETKEY_LEN];
    strcpy(md5buffer,strId.c_str());
    strcat(md5buffer,"{");
    strcat(md5buffer,strSecretkey.c_str());
    strcat(md5buffer,"}");

    int len  = strId.length()+strSecretkey.length()+2;
    md5.GenerateMD5((unsigned char*)md5buffer,len);
    json["secrectkey"]=md5.ToString().c_str();
    json["version"] = VERSION;
    m_pRPCInterface->SendJsonRpc(this,"registerAcquisitor",json);
//    checkUpdateFlag();
}

DAS_RESULT BaseInfo::setDataCenter(const Json::Value &params)
{
    utils::WriteConfigFileStringValue("GATEWAY","id",params["gateway_id"].asString());
    utils::WriteConfigFileStringValue("GATEWAY","center",params["dataCenter"].asString());
    utils::WriteConfigFileIntValue("GATEWAY","port",params["dataPort"].asInt());
    utils::WriteConfigFileStringValue("GATEWAY","secretkey",params["secretkey"].asString());

    SocketRequest(4750).sendRequest("connectServer",Json::Value());
    return DAS_SUCCESS;
}

DAS_RESULT BaseInfo::getDataCenter(Json::Value &result)
{
    result["gateway_id"] = getConfigInfo("id");
    result["dataCenter"] = getConfigInfo("center");
    result["dataPort"] = atoi(getConfigInfo("port").c_str());
    result["secretkey"] = getConfigInfo("secretkey");
    result["heartbeat"] = atoi(getConfigInfo("heartbeat").c_str());
    return DAS_SUCCESS;
}

DAS_RESULT BaseInfo::setAcquisitorInfo(const Json::Value &params)
{
    const S8 *id = params["gateway_id"].asCString();
    if (!IsGateWayIdEqual(id)){
        return DAS_GATEWAYID_ERROR;
    }
//    m_sProperty = params["property"].asCString();
    return DAS_SUCCESS;
}

DAS_RESULT BaseInfo::getAcquisitorInfo(Json::Value &result)
{
    struct statfs diskinfo;

    statfs("/",&diskinfo);
    char freeDisk[MAX_NAME_LEN];

    double fused = diskinfo.f_bfree/1024.0*diskinfo.f_bsize/1024.0;
    double ftotal = diskinfo.f_blocks/1024.0*diskinfo.f_bsize/1024.0;
    sprintf(freeDisk,"Avail %lld MB/Total %lld MB",(U64)fused,(U64)ftotal);
    result["totalDisk"] = freeDisk; //单位MB

    result["gatewayId"] = getConfigInfo("id");

    result["version"] = VERSION;

    //内存
    char freeMemory[MAX_NAME_LEN];
    MEM_OCCUPY mc;
    sysinfo::get_mem_occupy(&mc);
    sprintf(freeMemory,"%ld/%ld kB",mc.total-mc.free,mc.total);

    result["freeMemory"] = freeMemory;

    struct utsname sys;

    uname(&sys);

    char sysver[MAX_NAME_LEN];
    result["hostname"]=sys.nodename;
    sprintf(sysver,"%s %s %s",sys.sysname,sys.release,sys.machine);
    result["sysVer"] = sysver;

#ifdef USE_HTNICE
    Json::Value netcard;
    char macaddr[MAX_NETADDR_LEN];
    string script;

    script = getenv(CONFIG_DIR_ENV);
    script +="/script/getip.sh";
    GetLocalMac(macaddr);
    result["macAddr"] = macaddr;
    netcard = SocketRequest().scriptRequest(script);
    result["netcard"] = netcard;
#else
    int count = GetNetCardCount(NULL);

    if (count > 0 ) {
        NetParm *card = (NetParm*)malloc(count*sizeof(NetParm));

        memset(card,0,count*sizeof(NetParm));
        GetNetCardCount(card);

        char ipaddr[MAX_NETADDR_LEN];
        char macaddr[MAX_NETADDR_LEN];
        char mask[MAX_NETADDR_LEN];
        char gateway[MAX_NETADDR_LEN];

        GetLocalMac(macaddr);
        Json::Value netcard;
        for (int i = 0;i<count;i++) {
            GetIfAddr(card[i],ipaddr,mask);
            GetGateWay(gateway);

            Json::Value config;
            config["card"] = card[i];
            config["ipAddr"] = ipaddr;
            config["mask"] = mask;
            config["gateway"] = gateway;
            netcard.append(config);
        }
        free(card);
        result["netcard"] = netcard;
        result["macAddr"] = macaddr;
    }
#endif
    result["heartbeat"] = atoi(getConfigInfo("heartbeat").c_str());
    result["backduration"] = atoi(getConfigInfo("backupduration").c_str());
    return DAS_SUCCESS;
}

DAS_RESULT BaseInfo::setGateway(const Json::Value &params)
{
#ifdef USE_HTNICE
    string script;

    script = getenv(CONFIG_DIR_ENV);
    script +="/script/setip.sh";

    for (int i = 0;i<params.size();i++) {
        const Json::Value &netcard = params[i];

        string sp = script;
        sp +=" ";
        sp += netcard["dhcp"].asBool()?"1":"0";
        sp +=" ";
        sp += netcard["card"].asString();
        sp +=" ";
        sp += netcard["ipAddr"].asString();
        sp +=" ";
        sp += netcard["mask"].asString();
        sp +=" ";
        sp += netcard["gateway"].asString();

        if (SocketRequest().scriptRequest(sp).empty())
            return DAS_FAILDSET;
    }
#else
    for (int i = 0;i<params.size();i++) {
        const Json::Value &netcard = params[i];

        if (0 != SetIfAddr(netcard["card"].asCString(),netcard["ipAddr"].asCString(),
                  netcard["mask"].asCString(),netcard["gateway"].asCString())){
            return DAS_FAILDSET;
        }
    }
#endif

    return DAS_SUCCESS;
}

DAS_RESULT BaseInfo::getGateway(Json::Value &result)
{
#ifdef USE_HTNICE
    string script;

    script = getenv(CONFIG_DIR_ENV);
    script +="/script/getip.sh";
    result = SocketRequest().scriptRequest(script);
    if (result.empty())
        return DAS_FAILDSET;
#else
    int count = GetNetCardCount(NULL);

    if (count > 0 ) {
        NetParm *card = (NetParm*)malloc(count*sizeof(NetParm));

        memset(card,0,count*sizeof(NetParm));
        GetNetCardCount(card);

        char ipaddr[MAX_NETADDR_LEN];
        char macaddr[MAX_NETADDR_LEN];
        char mask[MAX_NETADDR_LEN];
        char gateway[MAX_NETADDR_LEN];

        GetLocalMac(macaddr);
        for (int i = 0;i<count;i++) {
            GetIfAddr(card[i],ipaddr,mask);
            GetGateWay(gateway);

            Json::Value config;
            config["card"] = card[i];
            config["ipAddr"] = ipaddr;
            config["mask"] = mask;
            config["gateway"] = gateway;
            config["dhcp"] = false;
            result.append(config);
        }

        free(card);
    }
#endif
    return DAS_SUCCESS;
}

void BaseInfo::getGatewayConfig(gateway_config_t &gateway)
{
    strcpy(gateway.id, getConfigInfo("id").c_str());
    strcpy(gateway.datacenter.netaddr,getConfigInfo("center").c_str());
    gateway.datacenter.port = atoi(getConfigInfo("port").c_str());
    strcpy(gateway.secretkey, getConfigInfo("secretkey").c_str());
    gateway.heartbeat=atoi(getConfigInfo("heartbeat").c_str());
}

string BaseInfo::getConfigInfo(const char *ref)
{
#if 0
    SqliteHelper helper(NULL,"gateway");
    string result;

    helper.Select(0,&result,ref);
    return result;
#else
    string result;
    if (!strcmp(ref,"secretkey")) {
        result = "31cccf7264444398";
    }
    else
        result = utils::GetConfigFileStringValue("GATEWAY",ref,"");
    return result;
#endif
}



