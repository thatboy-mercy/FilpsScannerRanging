#ifndef __EQUIPMENTCOMMDLL_H__
#define __EQUIPMENTCOMMDLL_H__

#include <WINDOWS.H>
#include <stdio.h>

#ifdef EQUIPMENTDLL
#define EQUIPMENTCOMM_DLL_API extern "C" __declspec(dllexport)
#else
#define EQUIPMENTCOMM_DLL_API extern "C" __declspec(dllimport)
#endif
/*************************************************************************
 接收数据回调函数
void (CALLBACK *EQCOMMDataCallBack)(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr)
_cid：连接编号
_lim_code：LIM Code (LIM.nCode)
_lim：LIM
_lim_len：LIM包长度 (LIM.nLIMLen)
_paddr：用户数据
*************************************************************************/
typedef void (CALLBACK *EQCOMMDataCallBack)(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr); 

/*************************************************************************
 连接状态回调函数
(CALLBACK *EQCOMMSTATECallBack)(int _cid, unsigned int _state_code, char* _ip, int _port, int _paddr)
_cid：连接编号
_state_code：连接状态 (EQCOMM_STATE_XXX)
_ip：IP地址
_port：端口号
_paddr：用户数据
*************************************************************************/
#define EQCOMM_STATE_OK					2001	//通讯库连接雷达成功
#define EQCOMM_STATE_ERR				2002	//通讯库连接雷达失败
#define EQCOMM_STATE_LOST				2003	//通讯库连接雷达连接断开
#define EQCOMM_STATE_HBACK				2004	//雷达心跳回复
#define EQCOMM_STATE_HBACK_TIMEOUT		2005	//雷达心跳回复超时

typedef void (CALLBACK *EQCOMMStateCallBack)(int _cid, unsigned int _state_code, const char* _ip, int _port, int _paddr); 

/*************************************************************************
获取版本号
int __stdcall		GetEquipmentCommVersion()
返回值：版本号
*************************************************************************/
EQUIPMENTCOMM_DLL_API int __stdcall	GetEquipmentCommVersion();

/*************************************************************************
初始化连接动态库
EQUIPMENTCOMM_DLL_API bool __stdcall EquipmentCommInit(int _paddr, EQCOMMDataCallBack _feqdata, EQCOMMStateCallBack _feqstate)
_paddr：用户数据，在回调函数会把此数据传出
_feqdata：接收数据回调函数
_feqstate：连接状态回调函数
返回值：false：失败 true：成功
*************************************************************************/
EQUIPMENTCOMM_DLL_API bool __stdcall EquipmentCommInit(int _paddr, EQCOMMDataCallBack _feqdata, EQCOMMStateCallBack _feqstate);

/*************************************************************************
退出连接动态库
bool __stdcall EquipmentCommDestory();
返回值：false：失败 true：成功
*************************************************************************/
EQUIPMENTCOMM_DLL_API bool __stdcall EquipmentCommDestory();

/*************************************************************************
打开设备连接
bool __stdcall	OpenEquipmentComm(int _cid, char* _ip, int _port);
_cid：连接编号
_ip：连接IP
_port：连接端口
返回值：false：失败 true：成功
*************************************************************************/
EQUIPMENTCOMM_DLL_API bool __stdcall OpenEquipmentComm(int _cid, const char* _ip, int _port);

/*************************************************************************
关闭设备连接
bool __stdcall	CloseEquipmentComm(int _cid);
_cid：连接编号
返回值：false：失败 true：成功
*************************************************************************/
EQUIPMENTCOMM_DLL_API bool __stdcall CloseEquipmentComm(int _cid);

/*************************************************************************
发送TIP
bool __stdcall	SendLIM(int _cid, void* _lim, int _lim_len);
_cid：连接编号
_lim：LIM
_lim_len：LIM包长度 (LIM.nLIMLen)
返回值：false：失败 true：成功
*************************************************************************/
EQUIPMENTCOMM_DLL_API bool __stdcall SendLIM(int _cid, void* _lim, int _lim_len);

#endif


