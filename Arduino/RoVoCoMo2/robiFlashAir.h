#ifndef _ROBI_FLASH_AIR_H_
#define _ROBI_FLASH_AIR_H_

//void sendRemoteLog(int id);
void sendRemoteLog(int id, int voice_time); //CRAFT 2.1
void changeRemoteLogDir();
void setWriteProtect(int onoff);
void deleteRemoteLog();

#endif
