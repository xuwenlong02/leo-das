/*************************************************************************
	> File Name: deviceinfo.c
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 12时02分29秒
 ************************************************************************/
#include "deviceinfo.h"
#include<stdio.h>
#include <devicemanager.h>
#include <utils/sqlite.h>
#include <utils/custom.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dbms/devices.h>
#include <dbms/transports.h>
#include <dbms/models.h>

/**************************************
 *
 *
***************************************/
DeviceInfo::DeviceInfo(deviceinfo_t &di,
                       BOOL add) {

    m_iDevInfo = di;

    if (add)
        addSqlDevice(di);

}

DeviceInfo::DeviceInfo(S32 id) {

    devices dev(id);
    transports trans(id);

    m_iDevInfo.id = id;

    m_iDevInfo.dev_id = dev["devId"].c_str();
    m_iDevInfo.model_id = atoi(dev["model"].c_str());
    models md(m_iDevInfo.model_id);
    m_iDevInfo.model_name = md["name"];
    m_iDevInfo.name = dev["name"];
    m_iDevInfo.dev_type = (DM_TYPE)atoi(dev["type"].c_str());



    m_iDevInfo.start = atoll(trans["regtime"].c_str());
    m_iDevInfo.sample = atof(trans["sample"].c_str());
    m_iDevInfo.trans.timeout = atoll(trans["timeout"].c_str());
    m_iDevInfo.trans.trans_mode = (TM_MODE)atoi(trans["mode"].c_str());

    string addr=trans["addr"];
    std::vector<std::string> list = utils::Split(addr.c_str(),":");
    if (m_iDevInfo.trans.trans_mode == TM_SEARIAL && list.size()==5){
        strcpy(m_iDevInfo.trans.trans_addr.portaddr.com,list.at(0).c_str());
        m_iDevInfo.trans.trans_addr.portaddr.baudrate=atoi(list.at(1).c_str());
        m_iDevInfo.trans.trans_addr.portaddr.databit=atoi(list.at(2).c_str());
        m_iDevInfo.trans.trans_addr.portaddr.parity=atoi(list.at(3).c_str());
        m_iDevInfo.trans.trans_addr.portaddr.stopbit=atoi(list.at(4).c_str());
    }
    else if(m_iDevInfo.trans.trans_mode == TM_TCP && list.size() == 2){
        strcpy(m_iDevInfo.trans.trans_addr.socketaddr.ip_addr,list.at(0).c_str());
        m_iDevInfo.trans.trans_addr.socketaddr.port = atoi(list.at(1).c_str());
    }
}

DeviceInfo::~DeviceInfo()
{
}

ID_DEV& DeviceInfo::getDeviceId()
{
    return m_iDevInfo.dev_id;
}

S32 DeviceInfo::Id()
{
    return m_iDevInfo.id;
}

deviceinfo_t &DeviceInfo::GetInfo()
{
    return m_iDevInfo;
}

string DeviceInfo::getChannel()
{
    if (m_iDevInfo.trans.trans_mode == TM_SEARIAL)
        return m_iDevInfo.trans.trans_addr.portaddr.com;
    else
        return "tcp";
}

tConnection DeviceInfo::GetConnection()
{
    tAcquisitor acquisitor =
            ((AcquisitionManager*)GetTask(DAS_ACQUISITOR))->GetAcquisitor(m_iDevInfo);
    if (acquisitor.valid())
        return acquisitor->getConnector();
    return NULL;
}

string DeviceInfo::getStatus()
{
    tConnection connector = GetConnection();
    return connector.valid()?(connector->IsConnected()?"added":"unadd"):"unknown";
}

bool DeviceInfo::status()
{
    Acquisitor *acquisitor = (Acquisitor*)getAcquisitor();
    if (acquisitor)
        return acquisitor->IsConnected();
    return FALSE;
}

void* DeviceInfo::getAcquisitor()
{
    return ((AcquisitionManager*)GetTask(DAS_ACQUISITOR))->GetAcquisitor(m_iDevInfo).get();
}

void DeviceInfo::updareAcquisitor()
{
    ((AcquisitionManager*)GetTask(DAS_ACQUISITOR))->UpdateAcquisitor(m_iDevInfo);
//    if (acquisitor.valid())
//        return acquisitor->updateConnector();
}

/*
 * 删除数据库中设备信息和通讯信息
 */
BOOL DeviceInfo::removeSqlDevice()
{
    devices(m_iDevInfo.id).deletethis();
    transports(m_iDevInfo.id).deletethis();
}

BOOL DeviceInfo::addSqlDevice(deviceinfo_t &devinfo)
{
    devices dev(devinfo.id);
    if(!dev.inorup(devinfo.name,devinfo.dev_id.c_str(),devinfo.model_id,devinfo.dev_type))
        return FALSE;

    char values[MAX_COND_LEN] = {0};

    if (devinfo.trans.trans_mode == TM_SEARIAL)
        sprintf(values,"%s:%d:%d:%d:%d",devinfo.trans.trans_addr.portaddr.com,
                devinfo.trans.trans_addr.portaddr.baudrate,
                devinfo.trans.trans_addr.portaddr.databit,
                devinfo.trans.trans_addr.portaddr.parity,
                devinfo.trans.trans_addr.portaddr.stopbit);
    else if(devinfo.trans.trans_mode == TM_TCP)
        sprintf(values,"%s:%d",devinfo.trans.trans_addr.socketaddr.ip_addr,
                devinfo.trans.trans_addr.socketaddr.port);

    transports trans(devinfo.id);
    if (!trans.inorup(devinfo.sample,devinfo.start,devinfo.trans.timeout,devinfo.trans.trans_mode,
                 values))
        return FALSE;
    models md(devinfo.model_id);
    md.updateName(devinfo.model_name);
    return TRUE;
}

BOOL DeviceInfo::updateSqlDevice(deviceinfo_t &devinfo)
{
    m_iDevInfo = devinfo;
    return addSqlDevice(devinfo);
}



