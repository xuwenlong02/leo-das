/*************************************************************************
	> File Name: acquisitor.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年02月02日 星期五 14时22分15秒
 ************************************************************************/

#include<stdio.h>
#include <dlfcn.h>
#include <time.h>
#include <utils/sqlite.h>
#include <dbms/acquire.h>
#include <dbms/exprs.h>
#include "acqdata.h"
#include "acquisitor.h"
#include "acquisition.h"

Acquisitor::Acquisitor(const deviceinfo_t& devinfo):
    m_idevinfo(devinfo),
    m_pTimer(NULL),
    m_iDataLock(TRUE),
    m_iCommitId(-1),
    m_fixlistData(devinfo.id) {

    char name[MAX_ID_LEN+20];
    sprintf(name,"acquisitor%04d",devinfo.id);
    m_pTimer = new Timer(name,this);

    updateAquireData();
}

Acquisitor::~Acquisitor()
{
    m_pTimer->Stop();
    delete m_pTimer;
}

void Acquisitor::RunTimer()
{
    /* 获取指令 */
    LOG_TRACE();

    {
        AutoLock lock(m_iTmpLock);
        m_listAcquire.clear();
    }while(0);

    time_t acqtime=time(NULL);

    CommitTimer commitTimer(m_pTimer,m_idevinfo.sample);//修改下次时间

    Operator iOperator(this);
    OpValue opValue = iOperator.GetFirstCmd(100);

    if (opValue.IsEmpty()) {
        commimtError("NO INSTRUCTION");
        return;
    }

    tConnection connector = getConnector();
    if (!connector.valid())
        return;
    while (!opValue.IsEmpty()) {

        while (!opValue.IsEmpty()) {
            U8 buffer[512];
            S32 byte = 0;
            CmdValue cmdValue = opValue.GenCmd();
            if (connector->AtomGetInstruction(m_idevinfo.trans ,cmdValue.Cmd(),cmdValue.Length(),buffer,byte)) {

                RECV_RESULT r;
                CmdValue recv;

                recv = CmdValue(buffer,byte);

                r = iOperator.ParseRecvCmd(recv);
#ifndef LOCAL_TEST

                if (RECV_COMPLETE != r) {
                    commimtError("PARSE ERROR");
                    AutoLock lock(m_iTmpLock);
                    m_listAcquire.clear();
                    return;
                }
#endif
                opValue = iOperator.GenerateNext();

            }
            else {
                commimtError("RECEIVE TIMEOUT");
                AutoLock lock(m_iTmpLock);
                m_listAcquire.clear();
                return;
            }
        }
#ifndef LOCAL_TEST
        OpValue data = iOperator["data"];
#else
        OpValue data = 27.73;
#endif
        AcqData acqdata = data;
        acqdata.SetTime(acqtime);
        acqdata.SetId(iOperator.GetItemId());

        AutoLock lock(m_iTmpLock);
        m_listAcquire.push_back(acqdata);

        opValue= iOperator.GetNextCmd();
    }

    AutoLock autolock(m_iDataLock);
    AutoLock lock(m_iTmpLock);
    for (vector<AcqData>::iterator iter = m_listAcquire.begin();
         iter != m_listAcquire.end();iter++) {
        m_fixlistData.push_back((AcqData &)*iter);
    }

}

void Acquisitor::StopTimer()
{
    m_pTimer->Stop();
}

BOOL Acquisitor::IsEqualId(S32 id)
{
    return id==m_idevinfo.id;
}

OpValue Acquisitor::IndicateDataItem(U16 itemid)
{
    OpValue data;
    Operator iOperator(this);
    OpValue csend = iOperator.GenerateCmd(itemid);
    U8 recv[MAX_RECV_SIZE];
    S32 length = 0;

    if (csend.IsEmpty()) {
        LOG_ERR("cmd is empty");
        return data;
    }

    while (!csend.IsEmpty()) {
        CmdValue cmdValue = csend.GenCmd();
        if (getConnector()->AtomGetInstruction(m_idevinfo.trans,
                                               cmdValue.Cmd(),cmdValue.Length(),recv,length)) {

            CmdValue rcm= CmdValue(recv,length);

            RECV_RESULT r = iOperator.ParseRecvCmd(rcm);

            if (RECV_COMPLETE == r ) {
                if (!csend.Next()) {
                    data = iOperator["data"];
                    break;
                }
                else
                    csend = iOperator.GenerateNext();
            }
            else
                break;
        }
    }
    return data;
}

DataItem *Acquisitor::GetDataItem(U16 itemid)
{
    AutoLock aulock(m_iLock);
    DataItem* item = NULL;

    for (int i = 0;i < m_listDataItems.size();i++) {
        DataItem& dateItem = m_listDataItems.at(i);
        if (item != NULL) {
            if (dateItem.GetItemId() < item->GetItemId() &&
                    dateItem.GetItemId() >= itemid)
                item = &dateItem;
        }
        else {
            if (dateItem.GetItemId() >= itemid) {
                item = &dateItem;
            }
        }
    }
    return item;
}

tExpress Acquisitor::LocateFunction(const char *def,int funcId)
{
    Template* temp = getTemplate();
    if (temp)
        return temp->LocateFunction(def,funcId);
    return NULL;
}

const ID_DEV &Acquisitor::DevAddr()
{
    return m_idevinfo.dev_id;
}

S32 Acquisitor::commitId()
{
    return m_iCommitId;
}

void Acquisitor::handleJsonResult(BOOL ret)
{
    LOG_TRACE();
    AutoLock autolock(m_iDataLock);
    int cmmid = m_iCommitId;
    m_iCommitId = -1;
    if (m_fixlistData.isEmpty())
        return;

    int index;
    int size = m_fixlistData.size();

    for (index = size-1;index>=0;index--) {
        AcqData tmp = m_fixlistData.at(index);
        if (tmp.GetFlag() == 1) {
            if (ret) {
                m_fixlistData.remove(index);
            }
            else {
                tmp.SetFlag(0);
                m_fixlistData.replace(index,tmp);
            }
        }
    }
    LOG_DEBUG("ret = %d,commitId = %d,from size = %d to %d",
              ret,cmmid,size,m_fixlistData.size());
}

void Acquisitor::OnReceiveJson(DAS_RESULT result, const string &method, const Json::Value &json)
{
    if (method == "acquireData") {
        if (result == DAS_SUCCESS && json.asBool() == true) {
            handleJsonResult(true);
        }
        else {
            handleJsonResult(false);
        }
    }
}

BOOL Acquisitor::updateAquireData()
{
    AutoLock aulock(m_iLock);
    m_listDataItems.clear();
    acquire aq(m_idevinfo.model_id);

    aq.FetchItems();

    vector<acquire::acquiredata>& list=aq.getAcqParams();

    for (int i = 0;i < list.size();i++) {
        DataItem item;
        acquire::acquiredata& data = list.at(i);
        item.SetItemId(data.itemId);
        item.SetRegLength(data.reglen);
        item.ParseDataExpr(data.dataexpr,data.fRate);
        item.ParseParams(data.listparams);
        m_listDataItems.push_back(item);
    }
//    commimtData();
#ifndef LOCAL_TEST
    updateConnector();
#endif
}

Template* Acquisitor::getTemplate()
{
    tTemplate tconfig = ((AcquisitionManager*)GetTask(DAS_ACQUISITOR))->GetTemplate(m_idevinfo.model_id);
    if (tconfig.valid())
        return tconfig.get();
    return NULL;
}

BOOL Acquisitor::IsConnected()
{
    return (!m_listAcquire.empty());
}

vector<AcqData> Acquisitor::getAcquireData()
{
    AutoLock lock(m_iTmpLock);
    return m_listAcquire;
}

tConnection Acquisitor::getConnector()
{
    return ((AcquisitionManager*)GetTask(DAS_ACQUISITOR))->
            DevInterface()->GetConnection(m_idevinfo.trans);
}

BOOL Acquisitor::updateConnector()
{
    double diff = time(NULL);//difftime(,0);

    S64 secs = (S64)(60*m_idevinfo.sample);

    if (secs == 0)
        return FALSE;
    S64 wait = secs-((S64)(diff))%secs;
//    LOG_DEBUG("diff=%lld,secs=%lld,wait=%lld",(S64)diff,secs,wait);

    LOG_INFO("time:%s,wait %lld s",utils::timetostr(time(NULL)).c_str(),wait);
    m_pTimer->Start(wait*1000,kPeriodic);

}

void Acquisitor::commimtData()
{
    if (!GetMsgStatus(DAS_REGISTERACQUISITOR)){
        return;
    }
    AutoLock autolock(m_iDataLock);
    if (m_fixlistData.isEmpty()) {
//        LOG_DEBUG("%04d(%d) is empty",m_idevinfo.id,m_iCommitId);
        return;
    }
    if (m_iCommitId >= 0) {
//        LOG_DEBUG("%04d(%d) has not been completed",m_idevinfo.id,m_iCommitId);
        return;
    }
    Json::Value params;

    resetCommit();

    int size = m_fixlistData.size();
    int total = 0;
    for (int i = 0;i < size;i++) {
        AcqData tmp = m_fixlistData.at(i);

        Json::Value jsondata;
        Json::Value records;
        records["devId"] = m_idevinfo.id;
        records["time"] = utils::timetostr(tmp.GetTime());

        jsondata.append(tmp.ToJson());
        tmp.SetFlag(1);
        m_fixlistData.replace(i,tmp);
        total++;

        int k = i;
        while (++k < size) {
            AcqData item = m_fixlistData.at(k);

            if (item.GetTime() != tmp.GetTime()) {
                --k;
                break;
            }
            jsondata.append(item.ToJson());
            item.SetFlag(1);
            m_fixlistData.replace(k,item);
            total++;
        }
        records["data"]=jsondata;
        params.append(records);
        i = k;
        if (total>20)
            break;
    }

//    m_fixlistData.clear();
    m_iCommitId = ((AcquisitionManager*)GetTask(DAS_ACQUISITOR))->RpcInterface()->SendJsonRpc(this,"acquireData",params);
    if (-1 == m_iCommitId) {
        resetCommit();
        LOG_ERR("commit failed:devId = %04d",m_idevinfo.id);
        return;
    }
    LOG_DEBUG("new commit:devId = %04d,msgId = %d ",m_idevinfo.id,m_iCommitId);
}

void Acquisitor::clearData()
{
    AutoLock autolock(m_iDataLock);
    m_fixlistData.clear();
}

void Acquisitor::resetCommit()
{
    int index = 0;
    int size = m_fixlistData.size();

    while (index < size ) {
        AcqData tmp = m_fixlistData.at(index);
        if (tmp.GetFlag() == 1) {
            tmp.SetFlag(0);
            m_fixlistData.replace(index,tmp);
        }
        index++;
    }
}

void Acquisitor::commimtError(const string &err)
{
    if (!GetMsgStatus(DAS_REGISTERACQUISITOR))
        return;
    Json::Value params;
    params["devId"] = m_idevinfo.id;
    params["id"] = m_idevinfo.dev_id.c_str();
    params["error"] = err;
    ((AcquisitionManager*)GetTask(DAS_ACQUISITOR))->RpcInterface()->SendJsonRpc(this,"acquireData",params);
}

DataIterface *Acquisitor::GetSpecifyItem(U16 itemid)
{
    AutoLock aulock(m_iLock);
    DataItem* item = NULL;

    for (int i = 0;i < m_listDataItems.size();i++) {
        DataItem& dateItem = m_listDataItems.at(i);
        if (dateItem.GetItemId() == itemid) {
            item = &dateItem;
            break;
        }
    }
    return item;
}


