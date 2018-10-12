/*************************************************************************
	> File Name: acquisition.c
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 13时43分36秒
 ************************************************************************/
#include "acquisition.h"
#include<stdio.h>
#include <json/json.h>
#include <logger.h>
#include <transport/globalmsg.h>
#include <utils/socket_request.h>
#include <md5cpp/md5.h>
#include <utils/config.h>

AcquisitionManager::AcquisitionManager():
    m_pTimer(NULL),
    m_bConnected(FALSE) {
    addReference();
    m_pTimer = new Timer("acquisition",this);
}

AcquisitionManager::~AcquisitionManager()
{
    m_bConnected = FALSE;
    delete m_pTimer;
    dropReference();
}

void AcquisitionManager::RunTimer()
{
    if (!m_bConnected)
        return;

    {
        AutoLock autolock(m_ilock);
        if (!GetMsgStatus(DAS_REGISTERACQUISITOR)) {
            RegisterAcquisitor();
            return;
        }
    }while(0);

//    AutoLock lock(m_iAcquisitorLock);
    for (vector<tAcquisitor>::iterator iter = m_listAcquisitor.begin();
         iter != m_listAcquisitor.end();
         iter++) {
        tAcquisitor &acquitor = *iter;
        if (acquitor.valid()) {
            acquitor->commimtData();
        }
    }

}

void AcquisitionManager::OnReceiveJson(
        DAS_RESULT result,
        const string &method,
        const Json::Value &json)
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

void AcquisitionManager::RegisterAcquisitor()
{
    Json::Value json;
    std::string strId = utils::GetConfigFileStringValue("GATEWAY","id","");
    std::string strSecretkey = "31cccf7264444398";

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
}

DAS_RESULT AcquisitionManager::connectServer(const Json::Value &params)
{
    m_pRPCInterface->TransportHandle()->ChangeStatus(false);
    return DAS_SUCCESS;
}

DAS_RESULT AcquisitionManager::clearData()
{
    for (vector<tAcquisitor>::iterator iter = m_listAcquisitor.begin();
         iter != m_listAcquisitor.end();
         iter++) {
        tAcquisitor &acquitor = *iter;
        if (acquitor.valid()) {
            acquitor->clearData();
        }
    }
}

DAS_RESULT AcquisitionManager::startAcquire()
{
    for (vector<tAcquisitor>::iterator iter = m_listAcquisitor.begin();
         iter != m_listAcquisitor.end();
         iter++) {
        tAcquisitor &acquitor = *iter;
        if (acquitor.valid()) {
            acquitor->updateConnector();
        }
    }
}

DAS_RESULT AcquisitionManager::stopAcquire()
{
    for (vector<tAcquisitor>::iterator iter = m_listAcquisitor.begin();
         iter != m_listAcquisitor.end();
         iter++) {
        tAcquisitor &acquitor = *iter;
        if (acquitor.valid()) {
//            acquitor->updateConnector();
            acquitor->StopTimer();
        }
    }
}

MODULE_DEFINE_BEGIIN(AcquisitionManager)
if (ret) {
    InitMsg();

#ifndef LOCAL_TEST
    m_pTimer->Start(10000,kPeriodic);
#else
    m_pTimer->Start(10000,kPeriodic);
#endif
}
else {
    m_pTimer->Stop();
    m_bConnected = FALSE;

//    m_iAcquisitorLock.Acquire();
    m_listAcquisitor.clear();
//    m_iAcquisitorLock.Release();

//    m_iConfigLock.Acquire();
    m_listTemplate.clear();
//    m_iConfigLock.Release();

}
MODULE_DEFINE_END

MODULE_NETWORK_BEGIN(AcquisitionManager)
if (isconected) {
    AutoLock autolock(m_ilock);
    m_bConnected = TRUE;
}
else {
    AutoLock autolock(m_ilock);
    SetMsgStatus(DAS_REGISTERACQUISITOR,FALSE);
    m_bConnected = FALSE;
}
MODULE_NETWORK_END

tAcquisitor AcquisitionManager::GetAcquisitor(const deviceinfo_t& devinfo)
{
//    AutoLock autolock(m_iAcquisitorLock);

    for (vector<tAcquisitor>::iterator iter = m_listAcquisitor.begin();
         iter != m_listAcquisitor.end();) {
        tAcquisitor &acquitor = *iter;
        if (!acquitor.valid())
            m_listAcquisitor.erase(iter);
        else {
            if (acquitor->IsEqualId(devinfo.id))
                return acquitor;
            iter++;
        }
    }

    tAcquisitor tAcquitor = new Acquisitor(devinfo);
    m_listAcquisitor.push_back(tAcquitor);
    return tAcquitor;
}

void AcquisitionManager::UpdateAcquisitor(const deviceinfo_t &devinfo)
{
//    AutoLock autolock(m_iAcquisitorLock);

    for (vector<tAcquisitor>::iterator iter = m_listAcquisitor.begin();
         iter != m_listAcquisitor.end();) {
        tAcquisitor &acquitor = *iter;
        if (!acquitor.valid())
            m_listAcquisitor.erase(iter);
        else {
            if (acquitor->IsEqualId(devinfo.id)) {
                acquitor->updateAquireData();
                return;
            }
            iter++;
        }
    }

    tAcquisitor tAcquitor = new Acquisitor(devinfo);
    m_listAcquisitor.push_back(tAcquitor);
}

BOOL AcquisitionManager::ExistTemplate(S32 model)
{
//    AutoLock autolock(m_iConfigLock);

    for (vector<tTemplate>::iterator iter = m_listTemplate.begin();
         iter != m_listTemplate.end();) {
        tTemplate &tmp = *iter;
        if (!tmp.valid())
            m_listTemplate.erase(iter);
        else {
            if (tmp->IsEqual(model)) {

                return TRUE;
            }
            iter++;
        }
    }
    return FALSE;
}


tTemplate AcquisitionManager::GetTemplate(S32 model)
{
//    AutoLock autolock(m_iConfigLock);

    for (vector<tTemplate>::iterator iter = m_listTemplate.begin();
         iter != m_listTemplate.end();) {
        tTemplate &tmp = *iter;
        if (!tmp.valid())
            m_listTemplate.erase(iter);
        else {
            if (tmp->IsEqual(model)) {
                return tmp;
            }
            iter++;
        }
    }

    tTemplate ttemp = new Template(model);
    if (!ttemp->Parse())
        return NULL;

    m_listTemplate.push_back(ttemp);
    return ttemp;
}

void AcquisitionManager::UpdateTemplate(S32 model)
{
//    AutoLock autolock(m_iConfigLock);

    for (vector<tTemplate>::iterator iter = m_listTemplate.begin();
         iter != m_listTemplate.end();) {
        tTemplate &tmp = *iter;
        if (!tmp.valid())
            m_listTemplate.erase(iter);
        else {
            if (tmp->IsEqual(model)) {
                tmp->UpdateTemplate();
                return;
            }
            iter++;
        }
    }

    tTemplate ttemp = new Template(model);
    if (!ttemp->Parse())
        return;

    m_listTemplate.push_back(ttemp);
}







