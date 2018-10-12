/*************************************************************************
	> File Name: devicemanager.c
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 12时03分16秒
 ************************************************************************/

#include <stdio.h>
#include "devicemanager.h"
#include <sqlite3.h>
#include <utils/custom.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dbms/models.h>
#include <dbms/acquire.h>
#include <utils/config.h>
#include <vector>

MODULE_DEFINE_BEGIIN(DeviceManager)
if (ret){
    SqliteHelper(this,"devices").Select(1,NULL,"id");
//    m_thread_->StartThread();
}
else {
//    m_thread_->StopThread();

    AutoLock autolock(m_lockDevInfo);

    while(!m_listDeviceInfo.empty()) {
        DeviceInfo *devInfo = *m_listDeviceInfo.begin();
        delete devInfo;
        m_listDeviceInfo.erase(m_listDeviceInfo.begin());
    }
}
MODULE_DEFINE_END

MODULE_NETWORK_BEGIN(DeviceManager)
if (isconected) {

}
MODULE_NETWORK_END

class DebugOpInterface:public OpInterface
{
public:
    DebugOpInterface(Template &temp,DataIterface &dItem,ID_DEV &devAddr):
        m_temp(temp),
        m_dItem(dItem),
        m_devAddr(devAddr){

    }

    virtual tExpress LocateFunction(const char* def,int funcId = -1) {
        return m_temp.LocateFunction(def,funcId);
    }

    virtual const ID_DEV& DevAddr() {
        return m_devAddr;
    }

    virtual DataIterface *GetDataItem(U16 itemid){
        if (itemid == m_dItem.GetItemId())
            return &m_dItem;
        return NULL;
    }

    virtual DataIterface *GetSpecifyItem(U16 itemid) {
        if (itemid == m_dItem.GetItemId())
            return &m_dItem;
        return NULL;
    }

private:
    Template &m_temp;
    DataIterface &m_dItem;
    ID_DEV &m_devAddr;

};

DeviceManager::DeviceManager():
    m_lockDevInfo(true)
{

}

DeviceManager::~DeviceManager()
{

}

int DeviceManager::OnSqliteResult(int searchId, void *data, int argc, char **argv, char **azColName)
{
#define STR_EQUAL(s1,s2) !strcmp(s1,s2)

    LOG_TRACE();
    if (searchId == 1){
        int id;
        if (STR_EQUAL(azColName[0],"id")){
            id = atoi(argv[0]);

            LOG_DEBUG("devId = %d, new deviceinfo",id);
            DeviceInfo *pDev = new DeviceInfo(id);
            pDev->updareAcquisitor();
            AutoLock autolock(m_lockDevInfo);
            m_listDeviceInfo.push_back(pDev);
        }
        else{
            LOG_WARN("not used:%s = %s",azColName[0],argv[0]?argv[0]:"NULL");
        }

    }
    return 0;

#undef STR_EQUAL
}

DAS_RESULT DeviceManager::updateDeviceList(const Json::Value &params,Json::Value& result)
{
    const Json::Value &jmodels = params["model"];
    DAS_RESULT dr = DAS_SUCCESS;

    Json::Value merror;
    for (int i = 0;i < jmodels.size();i++) {
        const Json::Value& jm = jmodels[i];

        S32 id = jm["id"].asInt();

        acquire acqm(id);
        acqm.deletethis();

        const Json::Value& jas = jm["acquire"];
        Json::Value acqerror;

        for ( int j = 0;j<jas.size();j++) {
            const Json::Value& ja = jas[j];
            S32 acqid = ja["acqid"].asInt();
            string name = ja["name"].asString();
            S32 reglen = ja["reglen"].asInt();
            string acqparams;

            if (ja.isMember("parms")) {
                acqparams = ja["parms"].asString();
            }
            string expr = ja["expr"].asString();
            F32 rate = ja["rate"].asFloat();

            acquire acq(id,acqid);
            if (!acq.inorup(name,reglen,acqparams,expr,rate))
                acqerror.append(acqid);
        }


        string temp = jm["temp"].asString();

        models dm(id);
        if (!dm.inorup(temp)) {
            Json::Value moderror;
            moderror["id"] = id;
            moderror["acqid"] = acqerror;
            merror.append(moderror);
        }
        else
            ((AcquisitionManager*)GetTask(DAS_ACQUISITOR))->UpdateTemplate(id);
    }


    const Json::Value &devices = params["device"];
    Json::Value deverror;

    for (int i = 0;i < devices.size();i++) {
        const Json::Value &jsonDev = devices[i];
        S32 id = jsonDev["devId"].asInt();

        if (jsonDev["opt"].asString() == "up") {

            if (itemOfContain(id))
                dr = modifyDevice(jsonDev);
            else
                dr = addDevice(jsonDev);
        }
        else if (jsonDev["opt"].asString() == "del") {
            dr = deleteDevice(jsonDev);
        }
        if (DAS_SUCCESS != dr) {
            Json::Value de;
            de["devId"] = id;
            deverror.append(de);
        }
    }

    if (merror.empty() && deverror.empty()) {
        return DAS_SUCCESS;
    }
    else {
        if (!merror.empty())
            result["modelError"] = merror;
        if (!deverror.empty())
            result["deviceError"] = deverror;
        return DAS_STOREERROR;
    }
}

DAS_RESULT DeviceManager::getDeviceIds(const Json::Value &params, Json::Value &result)
{
    if (!params.isMember("model"))
        return DAS_INVALIDPARAMETER;
    return DAS_INVALID;
}

DAS_RESULT DeviceManager::getDeviceStatus(const Json::Value &params, Json::Value &result)
{
    AutoLock autolock(m_lockDevInfo);
    if (params.isMember("devId")) {
        S32 id = params["devId"].asInt();
        DeviceInfo *di = itemOfContain(id);
        if (!di)
            return DAS_DEVICENOTEXIST;

        Acquisitor *acquisitor = (Acquisitor*)di->getAcquisitor();
        if (!acquisitor) {
            result = di->status();
            return DAS_SUCCESS;
        }
        Json::Value inner;
        inner["result"] = di->status();
        OpValue data = acquisitor->IndicateDataItem(6);

        if (!data.IsEmpty())
            inner["status"]=(bool)(data.Integer() != 0);
        result["inner"] = inner;
    }
    else {
        for (int i = 0;i<m_listDeviceInfo.size();i++) {
            DeviceInfo *di = m_listDeviceInfo.at(i);
            Json::Value dev;
            dev["devId"] = di->Id();
            dev["status"] = di->status();
            result.append(dev);
        }
    }
    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::getAcquireValues(const Json::Value &params,Json::Value& result)
{
    AutoLock autolock(m_lockDevInfo);
    if (!params.isMember("devId"))
        return DAS_INVALIDPARAMETER;
    S32 id = params["devId"].asInt();
    DeviceInfo *di = itemOfContain(id);
    if (!di)
        return DAS_DEVICENOTEXIST;
    vector<AcqData> list = ((Acquisitor *)di->getAcquisitor())->getAcquireData();
    for (vector<AcqData>::iterator iter = list.begin();iter != list.end();iter++) {
        AcqData acq = *iter;
        result.append(acq.ToJson());
    }
    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::getChnList(Json::Value &result)
{
    map<string,string> maplist;
    utils::GetSectionList("COM",maplist);

    for (map<string,string>::iterator iter=maplist.begin();
         iter!=maplist.end();iter++) {
        const string chn = iter->first;
        result.append(chn);
    }
    result.append("tcp");
    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::getDeviceList(const Json::Value &params,Json::Value& result)
{
    AutoLock autolock(m_lockDevInfo);
    if (!params.isMember("chn"))
        return DAS_JSON_PARSEFAILD;

    if (params["chn"].asString() == "all" ) {
        map<string,string> maplist;
        utils::GetSectionList("COM",maplist);
        maplist.insert(std::pair<string,string>("tcp","tcp"));

        for (map<string,string>::iterator iter=maplist.begin();
             iter!=maplist.end();iter++) {
            const string chn = iter->first;
            Json::Value list;
            list["chn"] = chn;
            for (int i = 0;i<m_listDeviceInfo.size();i++) {
                DeviceInfo *di = m_listDeviceInfo.at(i);
                if (di->getChannel() == chn) {
                    list["devices"].append(getDeviceInfo(di->Id()));
                }
            }
            result.append(list);
        }
    }
    else {
        const string chn = params["chn"].asString();
        result["chn"]=chn;
        for (int i = 0;i<m_listDeviceInfo.size();i++) {
            DeviceInfo *di = m_listDeviceInfo.at(i);
            if (di->getChannel() == chn) {
                result["devices"].append(getDeviceInfo(di->Id()));
            }
        }
    }

    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::getDeviceInfo(const Json::Value &params, Json::Value &result)
{
    AutoLock autolock(m_lockDevInfo);
    if (!params.isMember("devId"))
        return DAS_JSON_PARSEFAILD;
    S32 id = params["devId"].asInt();

    DeviceInfo *di = itemOfContain(id);
    if (!di)
        return DAS_DEVICENOTEXIST;

    result=getDeviceInfo(di->Id());

    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::addDevice(const Json::Value &params)
{
    if (!params.isMember("devId"))
        return DAS_JSON_PARSEFAILD;

    S32 id = params["devId"].asInt();
    ID_DEV devId = params["id"].asCString();

    DeviceInfo *di;
    deviceinfo_t devinfo;
    devinfo.id = id;
    devinfo.dev_id = devId;
    devinfo.sample = params["sample"].asFloat();
    devinfo.start = time(NULL);
    devinfo.trans = getCommunication(params["communication"]);
    devinfo.model_name = params["model"]["name"].asString();
    devinfo.model_id = params["model"]["id"].asInt();
    devinfo.name = params["name"].asString();
    devinfo.dev_type = (DM_TYPE)params["type"].asInt();

    if (itemOfContain(id))
        return DAS_DEVICEEXIST;

    di = new DeviceInfo(devinfo);

    AutoLock autolock(m_lockDevInfo);
    m_listDeviceInfo.push_back(di);

    /*
     * 获取配置
     */
    di->updareAcquisitor();

    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::deleteDevice(const Json::Value &params)
{
    if (!params.isMember("devId"))
        return DAS_JSON_PARSEFAILD;
    S32 id = params["devId"].asInt();
    AutoLock autolock(m_lockDevInfo);
    int index;
    if (-1 != (index = indexOfContain(id)))
        return DAS_DEVICENOTEXIST;
    DeviceInfo *di = m_listDeviceInfo.at(index);

    di->removeSqlDevice();

    /*
     * 删除内存中设备信息
     */
    m_listDeviceInfo.erase(m_listDeviceInfo.begin()+index);
    delete di;

    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::modifyDevice(const Json::Value &params)
{
    if (!params.isMember("devId"))
        return DAS_JSON_PARSEFAILD;
    S32 id = params["devId"].asInt();

    DeviceInfo *di;

    if (!(di = itemOfContain(id)))
        return DAS_DEVICENOTEXIST;

    deviceinfo_t &devinfo = di->GetInfo();
    if (params.isMember("id"))
        devinfo.dev_id = params["id"].asCString();
    if (params.isMember("sample"))
        devinfo.sample = params["sample"].asFloat();
    if (params.isMember("communication"))
        devinfo.trans = getCommunication(params["communication"]);

    if (params.isMember("model")) {
        devinfo.model_name = params["model"]["name"].asString();
        devinfo.model_id = params["model"]["id"].asInt();
    }
    if (params.isMember("name"))
        devinfo.name = params["name"].asString();
    /*
     * 数据库信息更新
     */
    BOOL ret1;
    ret1 = di->updateSqlDevice(devinfo);

    if (!ret1){
        return DAS_STOREERROR;
    }

    di->updareAcquisitor();

    /*
     * 获取配置
     */
    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::debugDevice(const Json::Value &params, Json::Value &result)
{
    if (!params.isMember("communication"))
        return DAS_INVALIDADDR;
    transmgr_t tranmgr = getCommunication(params["communication"]);
    tConnection connector = m_pDevInterface->GetConnection(tranmgr);

    if (!connector.valid())
        return DAS_MEM_NOTALLOC;
    if (params.isMember("template")){
        Template temp;
        if (!temp.Parse(params["template"].asString()))
            return DAS_TEMPLATE_ERROR;

        DataItem item;
        S32 acqid = params["acqid"].asInt();
        item.SetItemId(acqid==0?100:acqid);
        item.SetRegLength(params["reglen"].asInt());
        item.ParseDataExpr(params["expr"].asString(),params["rate"].asFloat());
        item.ParseParams(utils::Split(params["params"].asCString(),";"));

        ID_DEV devAddr = params["devId"].asCString();

        DebugOpInterface opInterface(temp,item,devAddr);

        Operator iOperator(&opInterface);
        OpValue csend = iOperator.GenerateCmd(item.GetItemId());

        if (csend.IsEmpty()) {
            result["send"]="empty";
            result["recv"]="empty";
            return DAS_SUCCESS;
        }

        Json::Value jsend;
        Json::Value jrecv;
        while (!csend.IsEmpty()) {
            U8 buffer[1024];
            S32 byte = 0;

            CmdValue send = csend.GenCmd();

            jsend.append(utils::hexcmdtostr(send.Cmd(),send.Length()));
            if (!connector->AtomGetInstruction(tranmgr,send.Cmd(),send.Length(),buffer,byte)) {
                jrecv = GetError(DAS_TIMEOUT);
                result["send"] = jsend;
                result["recv"] = jrecv;
                return DAS_SUCCESS;
            }
            jrecv.append(utils::hexcmdtostr(buffer,byte));
            CmdValue recv(buffer,byte);
            iOperator.ParseRecvCmd(recv);

            csend = iOperator.GenerateNext();
        }
        result["send"] = jsend;
        result["recv"] = jrecv;
        OpValue data = iOperator["data"];
        switch(data.type) {
        case TYPE_S8:
            result["value"]=data.data.s8;
            break;
        case TYPE_U8:
            result["value"] = data.data.u8;
            break;
        case TYPE_S16:
            result["value"] = data.data.s16;
            break;
        case TYPE_U16:
            result["value"] = data.data.u16;
            break;
        case TYPE_S32:
            result["value"] = data.data.s32;
            break;
        case TYPE_U32:
            result["value"] = data.data.u32;
            break;
        case TYPE_S64:
            result["value"] = data.data.s64;
            break;
        case TYPE_U64:
            result["value"] = data.data.u64;
            break;
        case TYPE_F32:
            result["value"] = data.data.f32;
            break;
        case TYPE_F64:
            result["value"] = data.data.f64;
            break;
        case TYPE_STR:
            result["value"] = data.data.str;
            break;
        case TYPE_CMD:
            result["value"] = utils::hexcmdtostr(data.cmd.Cmd(),data.cmd.Length());
            break;
        default:
            result["value"] = "none";
        }
        return DAS_SUCCESS;
    }
    else if(params.isMember("command")){
        U8 *cmd;
        S32 len;

        if ((len = utils::strtohexcmd(params["command"].asCString(),cmd)) == 0)
            return DAS_INVALIDPARAMETER;
        U8 buffer[256];
        S32 byte = 0;

        result["send"] = utils::hexcmdtostr(cmd,len);
        if (!connector->AtomGetInstruction(tranmgr,cmd,len,buffer,byte)) {
            free(cmd);
            result["recv"] = GetError(DAS_TIMEOUT);
            return DAS_SUCCESS;
        }
        free(cmd);
        result["recv"] = utils::hexcmdtostr(buffer,byte);
        return DAS_SUCCESS;
    }
    return DAS_INVALIDPARAMETER;

}

DAS_RESULT DeviceManager::indicateDevice(const Json::Value &params, Json::Value &result)
{
    if (!params.isMember("devId"))
        return DAS_JSON_PARSEFAILD;
    S32 id = params["devId"].asInt();

    DeviceInfo *di;

    if (!(di = itemOfContain(id)))
        return DAS_DEVICENOTEXIST;

    deviceinfo_t &devinfo = di->GetInfo();

    tConnection connector = m_pDevInterface->GetConnection(devinfo.trans);

    if (!connector.valid())
        return DAS_MEM_NOTALLOC;
    if (!params.isMember("acqid"))
        return DAS_INVALIDPARAMETER;

    S32 acqId = params["acqid"].asInt();

    Acquisitor *acquisitor = (Acquisitor*)di->getAcquisitor();
    if (!acquisitor)
        return DAS_INVALIDPARAMETER;
    OpValue data = acquisitor->IndicateDataItem(acqId);

    result = params;

    if (!data.IsEmpty())
        result["ret"]=(bool)(data.Integer() != 0);
    else
        result["ret"]=false;
//    result["ret"]=true;

    return DAS_SUCCESS;
}

DAS_RESULT DeviceManager::getAcquireData(const Json::Value &params, Json::Value &result)
{
    if (!params.isMember("model"))
        return DAS_INVALID;
    S32 model_id=params["model"].asInt();
    acquire aq(model_id);

    aq.FetchItems();

    vector<acquire::acquiredata>& list=aq.getAcqParams();

    Json::Value acquireDatas;
    for (int i = 0;i < list.size();i++) {
        Json::Value acqData;
        acquire::acquiredata& data = list.at(i);

        acqData["acqid"] = data.itemId;
        acqData["reglen"] = data.reglen;
        acqData["expr"] = data.dataexpr;
        acqData["rate"] = data.fRate;

        string listparams;
        for (int j=0;j<data.listparams.size();j++) {
            listparams +=data.listparams[j];
            if (j!=data.listparams.size()-1)
                listparams += ";";
        }
        acqData["params"] = listparams;
        acquireDatas.append(acqData);
    }
    result["acquireData"]=acquireDatas;

    models md(model_id);
    result["template"] = md["template"];
    return DAS_SUCCESS;
}

Json::Value DeviceManager::getCommunication(const transmgr_t &trans)
{
    Json::Value communication;
    if (trans.trans_mode == TM_TCP){
        communication["mode"] = "tcp";
        Json::Value addr;
        addr["ip"]=trans.trans_addr.socketaddr.ip_addr;
        addr["port"]=trans.trans_addr.socketaddr.port;
        communication["addr"]=addr;
    }
    else if(trans.trans_mode == TM_SEARIAL){
        communication["mode"] = "serial";
        Json::Value addr;
        addr["chn"]=trans.trans_addr.portaddr.com;
        addr["baudRate"]=trans.trans_addr.portaddr.baudrate;
        addr["dataBits"]=trans.trans_addr.portaddr.databit;
        addr["parity"]=trans.trans_addr.portaddr.parity;
        addr["stopBits"]=trans.trans_addr.portaddr.stopbit;
        communication["addr"]=addr;
    }
    communication["delay"] = trans.timeout;
    return communication;
}

transmgr_t DeviceManager::getCommunication(const Json::Value &jsonTrans)
{
    transmgr_t trans;

    if (!jsonTrans.isMember("mode"))
        return trans;
    if (!strcmp(jsonTrans["mode"].asCString(),"serial")) {
        trans.trans_mode = TM_SEARIAL;
        Json::Value jsonAddr = jsonTrans["addr"];
        strcpy(trans.trans_addr.portaddr.com,jsonAddr["chn"].asCString());
        trans.trans_addr.portaddr.baudrate = jsonAddr["baudRate"].asInt();
        trans.trans_addr.portaddr.databit = jsonAddr["dataBits"].asInt();
        trans.trans_addr.portaddr.stopbit = jsonAddr["stopBits"].asInt();
        trans.trans_addr.portaddr.parity = jsonAddr["parity"].asInt();
    }
    else if (!strcmp(jsonTrans["mode"].asCString(),"tcp")){
        trans.trans_mode = TM_TCP;
        const Json::Value &jsonAddr = jsonTrans["addr"];
        strcpy(trans.trans_addr.socketaddr.ip_addr,jsonAddr["ip"].asCString());
        trans.trans_addr.socketaddr.port=jsonAddr["port"].asInt();
    }
    trans.timeout = jsonTrans["delay"].asInt();
    return trans;
}

DeviceInfo* DeviceManager::itemOfContain(S32 id)
{
    AutoLock autolock(m_lockDevInfo);
    int index = 0;
    while(index<m_listDeviceInfo.size()){
        DeviceInfo *di = m_listDeviceInfo.at(index);
        if (di->Id() == id)
            return di;
        ++index;
    }
    return NULL;
}

int DeviceManager::indexOfContain(S32 id)
{
    AutoLock autolock(m_lockDevInfo);
    int index = 0;
    while(index<m_listDeviceInfo.size()){
        DeviceInfo *di = m_listDeviceInfo.at(index);
        if (di->Id()==id)
            return index;
        ++index;
    }
    return -1;
}

Json::Value DeviceManager::getDeviceInfo(S32 devId)
{
    DeviceInfo *di = itemOfContain(devId);
    Json::Value result;
    if (!di)
        return result;
    result["devId"] = di->GetInfo().id;
    result["name"]=di->GetInfo().name;
    result["id"]=di->getDeviceId().c_str();
    result["status"]=
            di->status();
//    result["chn"]=di->getChannel();
    result["sample"]=di->GetInfo().sample;
    Json::Value jm;
    jm["id"] = di->GetInfo().model_id;
    jm["name"] = di->GetInfo().model_name;
    result["model"]= jm;
    result["type"] = di->GetInfo().dev_type;

    result["communication"]=getCommunication(di->GetInfo().trans);
    return result;
}
