/*************************************************************************
	> File Name: deviceinfo.h
	> Author: xuwenlong
	> Mail: myxuan475@126.com 
	> Created Time: 2018年01月17日 星期三 12时02分54秒
 ************************************************************************/
#ifndef __DEVICEINFO_H__
#define __DEVICEINFO_H__

#include <defines.h>
#include <transport/transport.h>
#include <utils/sqlite.h>

/******************************
 *device type
 *
*******************************/
typedef enum {
    DM_GETADDR = 0,
    DM_WATERMETER,
    DM_ELECTRICMETER,
    DM_MAX
}DM_TYPE;

/******************************
 *device info
 *
*******************************/
class deviceinfo {
public:
    S32   id;
    ID_DEV  dev_id;  /* 16进制,具体由解析插件决定 */
    DM_TYPE dev_type;
    time_t  start; /* seconds since 1970-1-1  00:00:00 +000 */
    F32     sample; /* sample minutes */
    std::string name;
    std::string  model_name;
    S32     model_id;
    transmgr_t trans;/* 通讯 */
};
typedef class deviceinfo deviceinfo_t;
typedef class deviceinfo *pdeviceinfo_t;

class DeviceInfo
{
public:
    DeviceInfo(deviceinfo_t &di,BOOL add = TRUE);
    DeviceInfo(S32 id);
    virtual ~DeviceInfo();
    ID_DEV &getDeviceId();
    S32 Id();
    deviceinfo_t &GetInfo();
    tConnection GetConnection();
    std::string getStatus();
    bool status();

    void *getAcquisitor();
    void updareAcquisitor();

    /*
     * 数据库中设备信息和通讯信息
     */
    BOOL removeSqlDevice();

    static BOOL addSqlDevice(deviceinfo_t &devinfo);

    BOOL updateSqlDevice(deviceinfo_t &devinfo);

    std::string getChannel();

private:
    deviceinfo_t m_iDevInfo;
};

#endif//__DEVICEINFO_H__
