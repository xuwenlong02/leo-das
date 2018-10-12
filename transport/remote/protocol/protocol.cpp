/*************************************************************************
	> File Name: protocol.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 16时33分21秒
 ************************************************************************/

#include "protocol.h"
#include<stdio.h>
#include <logger.h>
#include <sstream>

RPCProtocol::RPCProtocol(TransportInterface *interface):
    m_pTransport(interface) {
}

RPCProtocol::~RPCProtocol()
{
}

void RPCProtocol::SetTransport(TransportInterface *interface)
{
    m_pTransport = interface;
}

void RPCProtocol::ClearRecvData()
{
    m_jsonBuffer.clearJsonBuffer();
}

void RPCProtocol::RpcParse()
{
    Json::Value rpc;
    while (m_jsonBuffer.getJsonFromBuffer(rpc)){
        OnMessage(rpc);
    }
}

void RPCProtocol::OnReceiveData(void *data, int length)
{
    m_jsonBuffer.addJsonBuffer((const char*)data,length);
}

void RPCProtocol::OnMessage(Json::Value &jsonObj)
{
    LOG_DEBUG("receive:%s",jsonObj.toStyledString().c_str());

    if (!jsonObj.isMember("id")){
        LOG_ERR("rpc json is incomplete!");
        return;
    }

    int msgId = jsonObj["id"].asInt();

    if(jsonObj.isMember("result")) {
        Json::Value result;
        if (jsonObj.isMember("date")) {
            result["ret"] = jsonObj["result"];
            result["date"] = jsonObj["date"];
        }
        else
            result = jsonObj["result"];
        OnResult(msgId,result);
    }
    else if (jsonObj.isMember("method"))
        OnRequest(msgId,jsonObj);  
    else
        LOG_ERR("json is not irregular!");
}

void RPCProtocol::SendResult(int msgId,const std::string &method,const Json::Value &result) {
  Json::Value root;
  root["id"] = msgId;
  root["jsonrpc"] = "2.0";
  if (!result.isNull() && result.isObject() && result.isMember("inner")) {
      Json::Value inner = result["inner"];
      Json::Value::Members members =
              inner.getMemberNames();
      for (Json::Value::Members::iterator iter = members.begin();
           iter != members.end();iter++) {
          const std::string &name = *iter;
          root[name] = inner[name];
      }
  }
  else
    root["result"] = result;
  root["method"] = method;
  sendJson(root);
}

BOOL RPCProtocol::SendRequest(int msgId, const std::string &method, const Json::Value &params) {
  Json::Value root;
  int id  = msgId;
  root["id"] = id;
  root["jsonrpc"] = "2.0";
  root["method"] = method;
  if (!params.isNull())
    root["params"] = params;
  return sendJson(root);
}

void RPCProtocol::SendError(int msgId,const std::string &method,bool result, const char *error) {
  Json::Value root;
  root["jsonrpc"] = "2.0";
  root["id"] = msgId;
  root["result"] = result;
  root["reason"] = error;
  root["method"] = method;

  sendJson(root);
}

BOOL RPCProtocol::sendJson(Json::Value &data) {
    if (NULL == m_pTransport) {
      return FALSE;
    }
    LOG_DEBUG("---send:%s", data.toStyledString().c_str());
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "";  // or whatever you like
    Json::StreamWriter *writer = builder.newStreamWriter();
    std::ostringstream out;
    writer->write(data,&out);
    std::string json_file = out.str();
    json_file.append("\n");
    delete writer;
    const char *pStr = json_file.c_str();
    return m_pTransport->SendData(pStr, json_file.length());
}

