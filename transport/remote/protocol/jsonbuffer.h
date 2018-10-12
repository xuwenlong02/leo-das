/*************************************************************************
	> File Name: jsonbuffer.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月23日 星期二 16时38分43秒
 ************************************************************************/
#ifndef __JSONBUFFER_H__
#define __JSONBUFFER_H__

#include <json/json.h>
#include <string>
#include <stdio.h>
#include <utils/lock.h>

class JsonBuffer {
 public:
  JsonBuffer();

  bool getJsonFromBuffer(Json::Value &output);
  void addJsonBuffer(const char *pData, int iLength);
  void clearJsonBuffer();
private:

 private:
  std::string m_szBuffer;
  Lock        m_bufferLock;
  int         m_iStartPos;
};

#endif//__JSONBUFFER_H__
