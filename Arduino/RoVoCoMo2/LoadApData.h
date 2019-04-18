#ifndef __LOADAPDATA_H__
#define __LOADAPDATA_H__

#define SOFT_AP_NUM 0
#define STATION_AP_NUM 1
#define FLASHAIR_AP_NUM 2

bool SetPreferFileAp(int type);
bool beginWiFi(int type);

bool beginFlashAir();

#endif
