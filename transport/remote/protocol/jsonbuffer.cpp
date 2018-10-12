/*************************************************************************
	> File Name: jsonbuffer.cpp
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 16时38分33秒
 ************************************************************************/

#include<stdio.h>
#include <json/reader.h>
#include <logger.h>
#include "jsonbuffer.h"

JsonBuffer::JsonBuffer() {
  m_iStartPos = 0;
}

bool JsonBuffer::getJsonFromBuffer(Json::Value &output)
{
    AutoLock auto_lock(m_bufferLock);
    if (m_szBuffer.empty())
        return false;

    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    Json::CharReader *reader = builder.newCharReader();
//    int pos = 0;
    int das = 64;

    const char *pStr = m_szBuffer.c_str();
    int   length = m_szBuffer.length();
    while (m_iStartPos <= length) {

        if (m_iStartPos+das > length) {
            das =length-m_iStartPos;
            m_iStartPos = length;
            if (das <= 0)
                break;
        }
        else
            m_iStartPos += das;

        std::string errs;

        if (reader->parse(pStr,pStr+m_iStartPos,&output,&errs)) {
            if (das >= 2) {
                m_iStartPos =m_iStartPos - das;
                das /= 2;
            }
            else {
                m_szBuffer = m_szBuffer.erase(0,m_iStartPos);
                m_iStartPos = 0;
                delete reader;
                return true;
            }
        }

    }
    delete reader;
    return false;
}

void JsonBuffer::addJsonBuffer(const char *pData, int iLength)
{
    m_bufferLock.Acquire();
    if (pData != 0 && iLength > 0){

      m_szBuffer.append(pData, iLength);

    }

    m_bufferLock.Release();
}

void JsonBuffer::clearJsonBuffer()
{
    m_bufferLock.Acquire();
    m_szBuffer.clear();
    m_iStartPos = 0;
    m_bufferLock.Release();
}
