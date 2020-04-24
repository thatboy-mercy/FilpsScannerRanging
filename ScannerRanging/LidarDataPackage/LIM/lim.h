#ifndef __LIM_H__
#define __LIM_H__

#if defined (LINUX)

#ifndef STRUCTPACKED
#define STRUCTPACKED __attribute__((__packed__))
#endif

#elif defined(_WIN32)||defined(WINNT)

#ifndef STRUCTPACKED
#define STRUCTPACKED
#endif

#endif

/*
	Device network configuration.
*/
#define LIM_DT_IP			"237.1.1.200"		//IP for LIM multicasting.
#define LIM_DT_PORT			2111				//UDP port for LIM multicasting.
#define LIM_USER_PORT		2112				//Socket port for LIM communication.

/*
	Lidar Interaction Message (LIM) Structure:
	LIM ::= LIM_HEAD + Extended_Data
	Remark:
	1) Extended_Data : optional.
	
	Extended_Data:
	1) Lidar Measurement Data (LMD) Structure:
	   LMD ::= LMD_INFO + Data_Array
	   Remark:
	   1) Lidar mesaurement: distance value, unit: cm;
	   2) Data_Array: array of measurement; type: LMD_D_Type; length: LMD_INFO.nMDataNum.
*/

/*
	Head info of LIM.
*/
#define LIM_TAG				0xF5EC96A5
#define	LIM_VER				0x01000000
#define LIM_DATA_LEN		4
typedef struct
{
	unsigned int			TAG;				//LIM_TAG.
	unsigned int			VER;				//Version.
	unsigned int			nCID;				//Application-specified network communication ID for ack LIM in multi-thread applications.
	unsigned int			nCode;				//LIM code.
	unsigned int			Data[LIM_DATA_LEN];	//LIM Data.
	unsigned int			nLIMLen;			//Length of LIM, including Extended_Data.
	unsigned int			CheckSum;			//Checksum including LIM_Head & Extended LIM Data.
}STRUCTPACKED LIM_HEAD;

/*
	LIM Code.
*/
#define LIM_CODE_LDBCONFIG				111		//Device configuration.
#define LIM_CODE_START_LDBCONFIG		110		//Start device configuration broadcasting.
#define LIM_CODE_STOP_LDBCONFIG			112		//Stop device configuration broadcasting.
#define LIM_CODE_GET_LDBCONFIG			114		//Get Device configuration.

#define LIM_CODE_SYS_REBOOT				1102	//System reboot.
#define LIM_CODE_SYS_REBOOT_ACK			1103	//System reboot ACK.

#define LIM_CODE_FIRMWARE_VER			121		//Device firmware version.
#define LIM_CODE_FIRMWARE_VER_QUERY		122		//Querry device firmware version.

#define LIM_CODE_HB						10		//Heartbeat.
#define LIM_CODE_HBACK					11		//Heartbeat ACK.

#define LIM_CODE_LMD					901		//LMD.
#define LIM_CODE_LMD_RSSI				911		//LMD with RSSI.	
#define LIM_CODE_START_LMD				1900	//Start LMD transfering.
#define LIM_CODE_STOP_LMD				1902	//Stop LMD transfering.
#define LIM_CODE_NATBL					1915	//Scanning angle table of LMD.
#define LIM_CODE_NATBL_QUERY			1916	//Querry scanning angle table.

#define LIM_CODE_ALARM					9001	//Device alarming.
#define LIM_CODE_DISALARM				9003	//Device disalarming.
#define LIM_CODE_ALARM_QUERY			9000	//Querry device alarming.
#define LIM_CODE_DEVICE_STATUS			9005	//Device status.
#define LIM_CODE_DEVICE_STATUS_QUERY	9004	//Querry device status.

#define LIM_CODE_FMSIG					1911	//Field monitoring (FM) signal.
#define LIM_CODE_FMSIG_QUERY			1912	//Querry Field monitoring (FM) signal.

#define LIM_CODE_IOSTATUS				1921	//I/O port status.
#define LIM_CODE_IOREAD					1920	//Read I/O port status.
#define LIM_CODE_IOSET					1922	//Set I/O output port status.
#define LIM_CODE_IOSET_RELEASE			1924	//Release I/O output port setting.

/*
Device function switch status (ON/OFF).
*/
#define LIM_CODE_RAINDUST_FLT_SWICTH			801		//Rain-fog-dust filtering.
#define LIM_CODE_STATIC_APP_SWICTH				811		//Temporal filtering.
#define LIM_CODE_SPATIAL_FLT_SWICTH				821		//Spatial filtering.
#define LIM_CODE_FIELD_MNT_SWICTH				831		//Field monitoring (FM).
#define LIM_CODE_MEASURE_SWICTH					841		//Measuring.
#define LIM_CODE_FOGCHK_SWICTH					851		//Fog-occlusion checking.
/*
Querry device function switch status.
*/
#define LIM_CODE_RAINDUST_FLT_SWICTH_STS_QUERY  800
#define LIM_CODE_STATIC_APP_SWICTH_STS_QUERY    810
#define LIM_CODE_SPATIAL_FLT_SWICTH_STS_QUERY   820
#define LIM_CODE_FIELD_MNT_SWICTH_STS_QUERY     830
#define LIM_CODE_MEASURE_SWICTH_STS_QUERY       840
#define LIM_CODE_FOGCHK_SWICTH_STS_QUERY        850
/*
Set device function switch status.
*/
#define LIM_CODE_RAINDUST_FLT_SWICTH_STS_SET	802
#define LIM_CODE_STATIC_APP_SWICTH_STS_SET		812
#define LIM_CODE_SPATIAL_FLT_SWICTH_STS_SET		822
#define LIM_CODE_FIELD_MNT_SWICTH_STS_SET		832
#define LIM_CODE_MEASURE_SWICTH_STS_SET			842
#define LIM_CODE_FOGCHK_SWICTH_STS_SET			852

/*
	LMD description.
	Info of LMD.
*/
typedef struct
{
	unsigned int			nRange;				//Measuring range. Unit: cm.
	int						nBAngle;			//Beginning angle. Unit: 1/1000 degree.
	int						nEAngle;			//Ending angle. Unit: 1/1000 degree.
	unsigned int			nAnglePrecision;	//Angle precision. Unit: 1/1000 degree.
	unsigned int			nRPM;				//Scanning frequency. Unit: RPM (round per minute).
	unsigned int			nMDataNum;			//Number of measurement data.
}STRUCTPACKED LMD_INFO;
/*
	LMD data type.
*/
typedef unsigned short LMD_D_Type;
typedef unsigned short LMD_D_RSSI_Type;

/*
	INFO of LMD scanning angle table.
*/
typedef LMD_INFO NA_INFO;

/*
	Device-alarming code.
*/
#define LIM_DATA_ALARMCODE_INTERNAL				1		//Internal error.
#define LIM_DATA_ALARMCODE_Occluded				101		//Device occluded / optical cover contaminated.
#define LIM_DATA_ALARMCODE_High_Temperature		1001	//Internal temperature too high.
#define LIM_DATA_ALARMCODE_Low_Temperature		1002	//Internal temperature too low.
#define LIM_DATA_ALARMCODE_Measurement_Failure	1005	//Measurement failure.
#define LIM_DATA_ALARMCODE_Measurement_Stopped	1007	//Measurement stopped.
#define LIM_DATA_ALARMCODE_Fog_Occluding		1009	//Fog-occlusion.
#define LIM_DATA_ALARMCODE_MNT_Field_Occluded	1011	//FM field(s) occluded.

/*
Device configuration.
*/
#define	ULDINI_MAX_ATTR_STR_LEN	0x20
typedef struct 
{
	// Product Info
	char szType[ULDINI_MAX_ATTR_STR_LEN];
	char szManufacturer[ULDINI_MAX_ATTR_STR_LEN];
	char szReleaseDate[ULDINI_MAX_ATTR_STR_LEN];
	char szSerialNo[ULDINI_MAX_ATTR_STR_LEN];

	// Network Configuration
	char szMAC[ULDINI_MAX_ATTR_STR_LEN];
	char deviceIP[ULDINI_MAX_ATTR_STR_LEN];
	char szMask[ULDINI_MAX_ATTR_STR_LEN];
	char szGate[ULDINI_MAX_ATTR_STR_LEN];
	char szDNS[ULDINI_MAX_ATTR_STR_LEN];

	// Measurement Parameters
	int nMR;
	int nESAR;
	int nESA[2];
	int nSAR;
	int nSA[2];
	int nSAV;
	int nSAP;
	int nPF;

}ULDINI_Type;

#ifdef __cplusplus
extern "C"{
#endif

/*
	LIM utilities.
*/

/*
	LIM_CheckSum:
	Function: calculating the checksum of LIM.
	Return value: checksum of _lim.
	Remark:
	1) Checksum calculation for LIM creating & sending;
	2) Checksum checking for received LIM.
*/
unsigned int LIM_CheckSum(LIM_HEAD * _lim);

/*
LIM_ExData:
Function: memory address of Extended_Data of LIM.
Return value: pointer to Extended_Data.
Remark:
1) When a LIM has extended data, e.g., LMD LIM, use LIM_ExData to obtain the memory address;
2) LIM_HEAD and Extended_Data locate in continuous memory, so the address of Extended_Data equals to (void*)(_lim + 1).
*/
void* LIM_ExData(LIM_HEAD* _lim);

/*
LMD_Info & LMD_D:
Function: memory address of LMD_INFO & measurement data array in LIM.
Return value: pointer to LMD_INFO & measurement data array.
Remark:
1) For an LMD LIM, address of LMD_INFO equals to (LMD_INFO*)LIM_ExData(_lim);
2) The whole LMD LIM locates in continuous memory, so the address of measurement data equals to (LMD_D_Type*)(LMD_Info(_lim) + 1).
*/
LMD_INFO* LMD_Info(LIM_HEAD* _lim);
LMD_D_Type* LMD_D(LIM_HEAD* _lim);
LMD_D_RSSI_Type* LMD_D_RSSI(LIM_HEAD* _lim);

/*
NA_Info & NA_D:
Function: memory address of NA_INFO & scanning angle table in LIM.
Return value: pointer to NA_INFO & scanning angle table.
Remark:
1) For an LMD-NA LIM, address of NA_INFO equals to (NA_INFO*)LIM_ExData(_lim);
2) The whole LMD-NA LIM locates in continuous memory, so the address of scanning angle table equals to (int*)(NA_Info(_lim) + 1).
*/
NA_INFO* NA_Info(LIM_HEAD* _lim);
int* NA_D(LIM_HEAD* _lim);

/*
LIM_Pack:
Function: compose a LIM.
return value: true / false
Remark:
1) composed LIM is returned in _lim.
*/
bool LIM_Pack(LIM_HEAD*& _lim, unsigned int _cid, unsigned int _code, unsigned int* _data = NULL, unsigned int _ext_data_len = 0, void* _ext_data = NULL);
/*
LIM_Copy:
Function: copy a LIM.
return value: true / false
Remark:
1) LIM copied from _slim is returned in _dlim.
*/
bool LIM_Copy(LIM_HEAD*& _dlim, LIM_HEAD* _slim);
/*
LIM_Release:
Function: release a LIM.
Remark:
1) memory of _lim is released, and _lim is cleared to NULL.
*/
void LIM_Release(LIM_HEAD*& _lim);

#ifdef __cplusplus
}
#endif

#endif
