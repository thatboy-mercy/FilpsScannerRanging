// TryLIM.cpp
//

#include "stdafx.h"

#include "EquipmentComm/EquipmentComm.h"
#include "LIM/lim.h"

//Remarks: IO_OUTNUM and IO_INNUM should be assigned according to the specific lidar type, which can be obtained from specific product manual.
#define IO_OUTNUM  3					//For C/B-typed lidars: 3; for FIT/M-typed lidars: 4.
#define IO_INNUM   2					//For C/B-typed lidars: 2; for FIT/M-typed lidars: 4.
bool g_bRebootFlag = false;				// Demonstration switch for reboot.
bool g_bTestRTFuncSwitchFlag = false;	// Demonstration switch for some function-switch.
char *szIP = "192.168.1.111";			// IP information of lidar.

int g_IO_OutSts[IO_OUTNUM];
int g_IO_InSts[IO_INNUM];

bool connected = false;
int nFrameNum = 0;
unsigned int g_RAINDUST_FLT_SWICTH = 0xFFFFFFFF;
unsigned int g_STATIC_APP_SWICTH = 0xFFFFFFFF;
unsigned int g_SPATIAL_FLT_SWICTH = 0xFFFFFFFF;
unsigned int g_FIELD_MNT_SWICTH = 0xFFFFFFFF;
unsigned int g_MEASURE_SWICTH = 0xFFFFFFFF;
unsigned int g_FOGCHK_SWICTH = 0xFFFFFFFF;

#define	MAX_FRAME_NUM    1000
#define	USER_DATA        NULL
#define	USER_TASK_CID    1


//Parsing LIM_CODE_SYS_REBOOT_ACK LIM.
void LIM_CODE_SYS_REBOOT_ACK_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_SYS_REBOOT_ACK)
		return;

	printf("#%d LMD Frame: LIM_CODE_SYS_REBOOT_ACK--------------------\n", nFrameNum);
	printf("\n\t------------------------------------------\n\n");
}

//Parsing LIM_CODE_FIRMWARE_VER LIM.
void LIM_CODE_FIRMWARE_VER_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_FIRMWARE_VER)
		return;

	unsigned int ver = lim->Data[0];

	printf("#%d LMD Frame: LIM_CODE_FIRMWARE_VER: 0x%08X--------------------\n", nFrameNum, ver);
	printf("\n\t------------------------------------------\n\n");
}

//Parsing LIM_CODE_DEVICE_STATUS LIM.
void LIM_CODE_DEVICE_STATUS_Decoding(LIM_HEAD *lim)
{
	char sastr[40];
	if (lim->nCode != LIM_CODE_DEVICE_STATUS)
		return;
	unsigned int sts = lim->Data[0];
	int stsalm = (sts >> 1);
	itoa(stsalm, sastr, 2);

	printf("#%d LMD Frame: LIM_CODE_DEVICE_STATUS(%d.%03d s)--------------------\n", nFrameNum, lim->Data[1], lim->Data[2]);
	if ((sts & 1) == 0)
		printf("Éè±¸¾ÍÐ÷×´Ì¬ / Status of device-ready signal: ÎÞÐ§ / OFF [%.8s] ", sastr);
	else
		printf("Éè±¸¾ÍÐ÷×´Ì¬ / Status of device-ready signal: ÓÐÐ§ / ON [%.8s] ", sastr);

	if ((stsalm & 0x1) == 0x1)
	{
		printf("[ÄÚ²¿´íÎó] [Internal Error]");
	}
	if ((stsalm & 0x2) == 0x2)
	{
		printf("[ÕÚµ² / Í¸¹âÕÖÔàÎÛ] [Occluded / Optical-cover-contaminated]");
	}
	if ((stsalm & 0x4) == 0x4)
	{
		printf("[¸ßÎÂ] [High-temperature]");
	}
	if ((stsalm & 0x8) == 0x8)
	{
		printf("[µÍÎÂ] [Low-temperature]");
	}
	if ((stsalm & 0x10) == 0x10)
	{
		printf("[²âÁ¿Ê§Ð§] [Measurement-failure]");
	}
	if ((stsalm & 0x20) == 0x20)
	{
		printf("[²âÁ¿Í£Ö¹] [Measuring-stopped]");
	}
	if ((stsalm & 0x40) == 0x40)
	{
		printf("[Å¨ÎíÕÚµ²] [Occluded by fog]");
	}
	if ((stsalm & 0x80) == 0x80)
	{
		printf("[¼à²âÇøÓòÕÚµ²] [FM-area(s)-occluded]");
	}
	printf("\n\t------------------------------------------\n\n");
}
//Parsing LIM_CODE_RAINDUST_FLT_SWICTH LIM.
void LIM_CODE_RAINDUST_FLT_SWICTH_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_RAINDUST_FLT_SWICTH)
		return;

	printf("#%d LMD Frame: LIM_CODE_RAINDUST_FLT_SWICTH = %d--------------------\n", nFrameNum, lim->Data[0]);
	g_RAINDUST_FLT_SWICTH = lim->Data[0];
	printf("\n\t------------------------------------------\n\n");
}
//Parsing LIM_CODE_STATIC_APP_SWICTH LIM.
void LIM_CODE_STATIC_APP_SWICTH_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_STATIC_APP_SWICTH)
		return;

	printf("#%d LMD Frame: LIM_CODE_STATIC_APP_SWICTH = %d--------------------\n", nFrameNum, lim->Data[0]);
	g_STATIC_APP_SWICTH = lim->Data[0];
	printf("\n\t------------------------------------------\n\n");
}
//Parsing LIM_CODE_SPATIAL_FLT_SWICTH LIM.
void LIM_CODE_SPATIAL_FLT_SWICTH_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_SPATIAL_FLT_SWICTH)
		return;

	printf("#%d LMD Frame: LIM_CODE_SPATIAL_FLT_SWICTH = %d--------------------\n", nFrameNum, lim->Data[0]);
	g_SPATIAL_FLT_SWICTH = lim->Data[0];
	printf("\n\t------------------------------------------\n\n");
}
//Parsing LIM_CODE_FIELD_MNT_SWICTH LIM.
void LIM_CODE_FIELD_MNT_SWICTH_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_FIELD_MNT_SWICTH)
		return;

	printf("#%d LMD Frame: LIM_CODE_FIELD_MNT_SWICTH = %d--------------------\n", nFrameNum, lim->Data[0]);
	g_FIELD_MNT_SWICTH = lim->Data[0];
	printf("\n\t------------------------------------------\n\n");
}
//Parsing LIM_CODE_MEASURE_SWICTH LIM.
void LIM_CODE_MEASURE_SWICTH_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_MEASURE_SWICTH)
		return;

	printf("#%d LMD Frame: LIM_CODE_MEASURE_SWICTH = %d--------------------\n", nFrameNum, lim->Data[0]);
	g_MEASURE_SWICTH = lim->Data[0];
	printf("\n\t------------------------------------------\n\n");
}
//Parsing LIM_CODE_FOGCHK_SWICTH LIM.
void LIM_CODE_FOGCHK_SWICTH_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_FOGCHK_SWICTH)
		return;

	printf("#%d LMD Frame: LIM_CODE_FOGCHK_SWICTH = %d--------------------\n", nFrameNum, lim->Data[0]);
	g_FOGCHK_SWICTH = lim->Data[0];
	printf("\n\t------------------------------------------\n\n");
}
//Parsing LIM_CODE_LDBCONFIG LIM.
void LIM_CODE_LDBCONFIG_Decoding(LIM_HEAD* lim)
{
	if (lim->nCode != LIM_CODE_LDBCONFIG)
		return;
	printf("#%d LMD Frame: LIM_CODE_LDBCONFIG--------------------\n", nFrameNum);
	ULDINI_Type *ldtype = (ULDINI_Type*)LIM_ExData(lim);
	printf("----Product Info\n");
	printf("Type: %s\n", ldtype->szType);
	printf("Manufacturer: %s\n", ldtype->szManufacturer);
	printf("ReleaseDate: %s\n", ldtype->szReleaseDate);
	printf("SerialNo: %s\n", ldtype->szSerialNo);

	printf("----Network Configuration\n");
	printf("szMAC: %s\n", ldtype->szMAC);
	printf("szIP: %s\n", ldtype->szIP);
	printf("szMask: %s\n", ldtype->szMask);
	printf("szGate: %s\n", ldtype->szGate);
	printf("szDNS: %s\n", ldtype->szDNS);

	printf("----Measurement Parameters\n");
	printf("MR: %d cm\n", ldtype->nMR);
	printf("ESAR: %d¡ã(%d¡ã~%d¡ã) \n", ldtype->nESAR, ldtype->nESA[0]/1000, ldtype->nESA[1]/1000);
	printf("SAR: %d¡ã(%d¡ã~%d¡ã) \n", ldtype->nSAR, ldtype->nSA[0], ldtype->nSA[1]);
	printf("SAV: %d \n", ldtype->nSAV);
	printf("SAP: %3.3f¡ã \n", ldtype->nSAP / 1000.);
	printf("PF: %d \n", ldtype->nPF);

	printf("\t------------------------------------------\n\n");
}

//Parsing LIM_CODE_LMD LIM.
void LIM_CODE_NATBL_Decoding(LIM_HEAD*lim)
{
	if (lim->nCode != LIM_CODE_NATBL)
		return;
	NA_INFO* na_info = NA_Info(lim);  //Retrieve NA_INFO pointer.
	int* na = NA_D(lim);        //Retrieve NA Table pointer.

	printf("Range<%5.2fM> Angle<%5.3f¡ã- %5.3f¡ã> AngleStep<%3.3f¡ã>\nRPM<%drpm> MeasurementData_Num<%d>\n", \
		na_info->nRange / 100., na_info->nBAngle / 1000., \
		na_info->nEAngle / 1000., na_info->nAnglePrecision / 1000., \
		na_info->nRPM, na_info->nMDataNum);


	//Print some NA.
	printf("    NA[0] :%d (1/1000¡ã) ", na[0]);
	printf("    NA[1] :%d (1/1000¡ã) ", na[1]);
	printf("... ... ... ...");
	printf("    NA[%d] :%d (1/1000¡ã)", na_info->nMDataNum - 1, na[na_info->nMDataNum - 1]);
	printf("\t------------------------------------------\n\n");
}
//Parsing LIM_CODE_LMD LIM.
void LIM_CODE_LMD_Decoding(LIM_HEAD*lim)
{
	if (lim->nCode != LIM_CODE_LMD)
		return;
	LMD_INFO* lmd_info = LMD_Info(lim);  //Retrieve LMD_INFO pointer.
	LMD_D_Type* lmd = LMD_D(lim);        //Retrieve LMD distance measurements pointer.

	//Print LMD TS.
	printf("#%d LMD Frame: LIM_CODE_LMD(%d.%03d s)--------------------\n", nFrameNum, lim->Data[2],lim->Data[3]);

	printf("Range<%5.2fM> Angle<%5.3f¡ã- %5.3f¡ã> AngleStep<%3.3f¡ã>\nRPM<%drpm> MeasurementData_Num<%d>\n", \
		lmd_info->nRange / 100., lmd_info->nBAngle / 1000., \
		lmd_info->nEAngle / 1000., lmd_info->nAnglePrecision / 1000., \
		lmd_info->nRPM, lmd_info->nMDataNum);


	//Print polar-coordinates of some measurements.
	//infotmp.nBAngle + i*(infotmp.nEAngle - infotmp.nBAngle) / ((int)(mp_curDataLen - 1))
	printf("    LMD  %5.4f¡ã:%d cm", (float)(lmd_info->nBAngle + 0 * (float)(lmd_info->nEAngle - lmd_info->nBAngle)/(lmd_info->nMDataNum-1)) / 1000.0, lmd[0]);
	printf("    %5.4f¡ã:%d cm", (float)(lmd_info->nBAngle + 1*(float)(lmd_info->nEAngle - lmd_info->nBAngle) / (lmd_info->nMDataNum - 1)) / 1000.0, lmd[1]);
	printf("... ... ... ...");
	printf("%5.4f¡ã:%d cm\n",
		(float)(lmd_info->nBAngle + (lmd_info->nMDataNum - 1)* (float)(lmd_info->nEAngle - lmd_info->nBAngle) / (lmd_info->nMDataNum - 1)) / 1000.0, lmd[(lmd_info->nMDataNum - 1)]);
	printf("\t------------------------------------------\n\n");
}

//Parsing LIM_CODE_LMD_RSSI LIM.
void LIM_CODE_LMD_RSSI_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_LMD_RSSI)
		return;
	LMD_INFO* lmd_info = LMD_Info(lim);			//Retrieve LMD_INFO pointer.
	LMD_D_Type* lmd = LMD_D(lim);				//Retrieve LMD distance measurements pointer.
	LMD_D_RSSI_Type *lmdrssi = LMD_D_RSSI(lim);	//Retrieve LMD RSSI measurements pointer.

	printf("#%d LMD Frame: LIM_CODE_LMD_RSSI(%d.%03d s)--------------------\n", nFrameNum, lim->Data[2], lim->Data[3]);

	//Print LMD_INFO.
	printf("Range<%5.2fM> Angle<%5.3f¡ã- %5.3f¡ã> AngleStep<%3.3f¡ã>\nRPM<%drpm> MeasurementData_Num<%d>\n", \
		lmd_info->nRange / 100., lmd_info->nBAngle / 1000., \
		lmd_info->nEAngle / 1000., lmd_info->nAnglePrecision / 1000., \
		lmd_info->nRPM, lmd_info->nMDataNum);
	//Print polar-coordinats and RSSIs of some measurements.
	printf("    LMD  %5.4f¡ã:%d cm RSSI:%d", (float)(lmd_info->nBAngle + 0 * (float)(lmd_info->nEAngle - lmd_info->nBAngle) / (lmd_info->nMDataNum - 1)) / 1000.0, lmd[0], lmdrssi[0]);
	printf("    %5.4f¡ã:%d cm RSSI:%d", (float)(lmd_info->nBAngle + 1 * (float)(lmd_info->nEAngle - lmd_info->nBAngle) / (lmd_info->nMDataNum - 1)) / 1000.0, lmd[1], lmdrssi[1]);
	printf("... ... ... ...");
	printf("%5.4f¡ã:%d cm RSSI:%d\n",
		(float)(lmd_info->nBAngle + (lmd_info->nMDataNum - 1) * (float)(lmd_info->nEAngle - lmd_info->nBAngle) / (lmd_info->nMDataNum - 1)) / 1000.0, lmd[(lmd_info->nMDataNum - 1)], lmdrssi[(lmd_info->nMDataNum - 1)]);
	printf("\t------------------------------------------\n\n");
}

//Parsing LIM_CODE_FMSIG LIM.
void LIM_CODE_FMSIG_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_FMSIG)
		return;
	int fieldIdx;
	int alm;
	int attentionW, attentionA;
	int alertW, alertA;
	int alarmW, alarmA;

	fieldIdx = lim->Data[0];
	alm = lim->Data[1];

	alarmA = (alm & 0x01);
	alarmW = (alm & 0x02) >> 1;
	alertA = (alm & 0x04) >> 2;
	alertW = (alm & 0x08) >> 3;
	attentionA = (alm & 0x10) >> 4;
	attentionW = (alm & 0x20) >> 5;

	printf("#%d LMD Frame: LIM_CODE_FMSIG(%d.%03d s)--------------------\n", nFrameNum, lim->Data[2], lim->Data[3]);
	printf("field_%d FMSIG: %d  %d  %d\n", fieldIdx, alarmA, alertA, attentionA);
	printf("  whole FMSIG: %d  %d  %d\n", alarmW, alertW, attentionW);
	printf("\t------------------------------------------\n\n");
}

//Parsing LIM_CODE_IOSTATUS LIM.
void LIM_CODE_IOSTATUS_Decoding(LIM_HEAD *lim)
{
	if (lim->nCode != LIM_CODE_IOSTATUS)
		return;

	int iosts = lim->Data[0];

	for (int i = 0; i < IO_OUTNUM; i++)
		g_IO_OutSts[i] = ((iosts & (1 << i)) >> i);

	for (int i = IO_OUTNUM; i < IO_OUTNUM + IO_INNUM; i++)
		g_IO_InSts[i - IO_OUTNUM] = ((iosts & (1 << i)) >> i);

	printf("#%d LMD Frame: LIM_CODE_IOSTATUS(%d.%03d s)--------------------\n", nFrameNum, lim->Data[2], lim->Data[3]);
	for (int i = 0; i < IO_OUTNUM; i++)
		printf(" OUT_%d: %d", i + 1, g_IO_OutSts[i]);
	printf("\n");
	for (int i = 0; i < IO_INNUM; i++)
		printf(" IN_%d: %d", i + 1, g_IO_InSts[i]);
	printf("\n\t------------------------------------------\n\n");
}

//Parsing LIM_CODE_ALARM LIM.
void LIM_CODE_ALARM_Decoding(LIM_HEAD *lim)
{
	int type;
	if (lim->nCode != LIM_CODE_ALARM)
		return;
	type = lim->Data[0];
	printf("#%d LMD Frame: LIM_CODE_ALARM--------------------\n", nFrameNum);
	if (type == LIM_DATA_ALARMCODE_INTERNAL)
		printf("Equipment Alarm: Internal error\n");
	else if (type == LIM_DATA_ALARMCODE_Occluded)
		printf("Equipment Alarm: Occluded or too dirty\n");
	else if (type == LIM_DATA_ALARMCODE_High_Temperature)
		printf("Equipment Alarm: Temperature too high\n");
	else if (type == LIM_DATA_ALARMCODE_Low_Temperature)
		printf("Equipment Alarm: Temperature too low\n");
	else if (type == LIM_DATA_ALARMCODE_Measurement_Failure)
		printf("Equipment Alarm: Measurement Failure\n");
	else if (type == LIM_DATA_ALARMCODE_Measurement_Stopped)
		printf("Equipment Alarm: Measurement Stopped\n");
	else if (type == LIM_DATA_ALARMCODE_Fog_Occluding)
		printf("Equipment Alarm: Fog Occluding\n");
	else if (type == LIM_DATA_ALARMCODE_MNT_Field_Occluded)
		printf("Equipment Alarm: Monitoring Field Occluded\n");
	printf("\t------------------------------------------\n\n");
}

//Parsing LIM_CODE_DISALARM LIM.
void LIM_CODE_DISALARM_Decoding(LIM_HEAD *lim)
{
	int type;
	if (lim->nCode != LIM_CODE_DISALARM)
		return;
	type = lim->Data[0];
	printf("#%d LMD Frame: LIM_CODE_DISALARM--------------------\n", nFrameNum);
	if (type == LIM_DATA_ALARMCODE_INTERNAL)
		printf("Equipment LIM_DATA_ALARMCODE_INTERNAL DisAlarm\n");
	else if (type == LIM_DATA_ALARMCODE_Occluded)
		printf("Equipment LIM_DATA_ALARMCODE_Occluded DisAlarm\n");
	else if (type == LIM_DATA_ALARMCODE_High_Temperature)
		printf("Equipment LIM_DATA_ALARMCODE_High_Temperature DisAlarm\n");
	else if (type == LIM_DATA_ALARMCODE_Low_Temperature)
		printf("Equipment LIM_DATA_ALARMCODE_High_Temperature DisAlarm\n");
	else if (type == LIM_DATA_ALARMCODE_Measurement_Failure)
		printf("Equipment LIM_DATA_ALARMCODE_Measurement_Failure DisAlarm\n");
	else if (type == LIM_DATA_ALARMCODE_Measurement_Stopped)
		printf("Equipment LIM_DATA_ALARMCODE_Measurement_Stopped DisAlarm\n");
	else if (type == LIM_DATA_ALARMCODE_Fog_Occluding)
		printf("Equipment LIM_DATA_ALARMCODE_Fog_Occluding DisAlarm\n");
	else if (type == LIM_DATA_ALARMCODE_MNT_Field_Occluded)
		printf("Equipment LIM_DATA_ALARMCODE_MNT_Field_Occluded DisAlarm\n");

	printf("\t------------------------------------------\n\n");
}

//Test Some function switch.
void Test_RT_Func_Switch()
{
	LIM_HEAD* lim = NULL;
	unsigned int data[LIM_DATA_LEN];

	if (g_bTestRTFuncSwitchFlag == false)
	{
		return;
	}

	memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
	data[0] = 0;
	LIM_Pack(lim, USER_TASK_CID, LIM_CODE_MEASURE_SWICTH_STS_SET, data); //Packing LIM_CODE_MEASURE_SWICTH_STS_SET LIM.
	SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
	LIM_Release(lim);
	Sleep(5000);

	memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
	data[0] = 1;
	LIM_Pack(lim, USER_TASK_CID, LIM_CODE_MEASURE_SWICTH_STS_SET, data); //Packing LIM_CODE_MEASURE_SWICTH_STS_SET LIM.
	SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
	LIM_Release(lim);
	Sleep(5000);

	if (g_RAINDUST_FLT_SWICTH != 0xFFFFFFFF)
	{
		memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
		if (g_RAINDUST_FLT_SWICTH == 0)
			data[0] = 1;
		else
			data[0] = 0;
		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_RAINDUST_FLT_SWICTH_STS_SET, data); //Packing LIM_CODE_RAINDUST_FLT_SWICTH_STS_SET LIM.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);
		Sleep(5000);
	}

	if (g_STATIC_APP_SWICTH != 0xFFFFFFFF)
	{
		memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
		if (g_STATIC_APP_SWICTH == 0)
			data[0] = 1;
		else
			data[0] = 0;
		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_STATIC_APP_SWICTH_STS_SET, data); //Packing LIM_CODE_STATIC_APP_SWICTH_STS_SET LIM.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);
		Sleep(5000);
	}

	if (g_SPATIAL_FLT_SWICTH != 0xFFFFFFFF)
	{
		memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
		if (g_SPATIAL_FLT_SWICTH == 0)
			data[0] = 1;
		else
			data[0] = 0;
		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_SPATIAL_FLT_SWICTH_STS_SET, data); //Packing LIM_CODE_SPATIAL_FLT_SWICTH_STS_SET LIM.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);
		Sleep(5000);
	}

	if (g_FIELD_MNT_SWICTH != 0xFFFFFFFF)
	{
		memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
		if (g_FIELD_MNT_SWICTH == 0)
			data[0] = 1;
		else
			data[0] = 0;
		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_FIELD_MNT_SWICTH_STS_SET, data); //Packing LIM_CODE_FIELD_MNT_SWICTH_STS_SET LIM.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);
		Sleep(5000);
	}

	if (g_FOGCHK_SWICTH != 0xFFFFFFFF)
	{
		memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
		if (g_FOGCHK_SWICTH == 0)
			data[0] = 1;
		else
			data[0] = 0;
		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_FOGCHK_SWICTH_STS_SET, data); //Packing LIM_CODE_FOGCHK_SWICTH_STS_SET LIM.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);
		Sleep(5000);
	}
}

//LIM-processing call-back function of device-communication-DLL.
void CALLBACK EqCommDataCallBack(int _cid, unsigned int _lim_code, void* _lim, int _lim_len, int _paddr)
{
	printf("EqCommDataCallBack _cid=%d, _lim_code=%d, _lim_len=%d, _paddr=%d\n",
		_cid, _lim_code, _lim_len, _paddr);

	LIM_HEAD *lim = (LIM_HEAD*)_lim;
	unsigned int checksum = LIM_CheckSum(lim);
	if (checksum != lim->CheckSum)           //Checking the checksum.
	{
		printf("\tLIM checksum error!\n");
		return;
	}

	if (LIM_CODE_LDBCONFIG == lim->nCode)
		LIM_CODE_LDBCONFIG_Decoding(lim);

	if (LIM_CODE_LMD == lim->nCode)
	{
		nFrameNum++;
		LIM_CODE_LMD_Decoding(lim);
	}

	if (LIM_CODE_LMD_RSSI == lim->nCode)
	{
		nFrameNum++;
		LIM_CODE_LMD_RSSI_Decoding(lim);
	}

	if (LIM_CODE_FMSIG == lim->nCode)
		LIM_CODE_FMSIG_Decoding(lim);

	if (LIM_CODE_IOSTATUS == lim->nCode)
		LIM_CODE_IOSTATUS_Decoding(lim);

	if (LIM_CODE_ALARM == lim->nCode)
		LIM_CODE_ALARM_Decoding(lim);

	if (LIM_CODE_DISALARM == lim->nCode)
		LIM_CODE_DISALARM_Decoding(lim);

	if (LIM_CODE_FIRMWARE_VER == lim->nCode)
		LIM_CODE_FIRMWARE_VER_Decoding(lim);

	if (LIM_CODE_NATBL == lim->nCode)
		LIM_CODE_NATBL_Decoding(lim);

	if (LIM_CODE_DEVICE_STATUS == lim->nCode)
		LIM_CODE_DEVICE_STATUS_Decoding(lim);

	if (LIM_CODE_SYS_REBOOT_ACK == lim->nCode)
		LIM_CODE_SYS_REBOOT_ACK_Decoding(lim);

}

//Communication-status-processing call-back function of device-communication-DLL.
void CALLBACK EqCommStateCallBack(int _cid, unsigned int _state_code, char* _ip, int _port, int _paddr)
{
	printf("EqCommStateCallBack _cid=%d, _state_code=%d, _ip=%s, _port=%d _paddr=%d\n",
		_cid, _state_code, _ip, _port, _paddr);

	if (_state_code == EQCOMM_STATE_OK)   //Connection established.
	{
		printf("Connection success.code-----------------------:%d\n", _state_code);

		LIM_HEAD* lim = NULL;
		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_GET_LDBCONFIG, NULL); //Retrieve device information.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);        //Send LIM.
		LIM_Release(lim);                                 //Release LIM pack.

		connected = true;
	}
	else if ((_state_code == EQCOMM_STATE_ERR) || (_state_code == EQCOMM_STATE_LOST))
	{
		connected = false;

		printf("Connection failed.code---------------\n\n\n--------:%d\n",_state_code);
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	LIM_HEAD* lim = NULL;
	unsigned int data[LIM_DATA_LEN];

	printf("GetEquipmentCommVersion version =%d\n", GetEquipmentCommVersion()); //Get SDK version.

	EquipmentCommInit(USER_DATA, EqCommDataCallBack, EqCommStateCallBack);      //Initialize device-communication-DLL.

	OpenEquipmentComm(USER_TASK_CID, szIP, LIM_USER_PORT);                      //Connect lidar.

	while (MAX_FRAME_NUM > nFrameNum)
	{
		Sleep(5000); //Wait 5s for connection established.

		if (connected == true)
		{
			LIM_HEAD* lim = NULL;

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_FIRMWARE_VER_QUERY, NULL); //Get firmware version of the lidar.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_NATBL_QUERY, NULL); //Get NATBL of the lidar.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_DEVICE_STATUS_QUERY, NULL); //Retrieve the status of device-ready signal.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_RAINDUST_FLT_SWICTH_STS_QUERY, NULL);	//Retrieve the status of rain-dust-filtering switch.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_STATIC_APP_SWICTH_STS_QUERY, NULL);	//Retrieve the status of temporal-filtering switch.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_SPATIAL_FLT_SWICTH_STS_QUERY, NULL);	//Retrieve the status of spatial-filtering switch.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_FIELD_MNT_SWICTH_STS_QUERY, NULL);	//Retrieve the status of FM function switch.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_MEASURE_SWICTH_STS_QUERY, NULL);		//Retrieve the status of measuring.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_FOGCHK_SWICTH_STS_QUERY, NULL);		//Retrieve the status of fog-detecting switch.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(500);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = 0;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_FMSIG_QUERY, data); //Retrieve FM signals.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_HB, NULL); //Send heart-beat LIM.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = LIM_DATA_ALARMCODE_INTERNAL;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_ALARM_QUERY, data); //Retrieve device-alarming status of LIM_DATA_ALARMCODE_INTERNAL.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = LIM_DATA_ALARMCODE_Occluded;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_ALARM_QUERY, data); //Retrieve device-alarming status of LIM_DATA_ALARMCODE_Occluded.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = LIM_DATA_ALARMCODE_High_Temperature;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_ALARM_QUERY, data); //Retrieve device-alarming status of LIM_DATA_ALARMCODE_High_Temperature.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = LIM_DATA_ALARMCODE_Low_Temperature;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_ALARM_QUERY, data); //Retrieve device-alarming status of LIM_DATA_ALARMCODE_Low_Temperature.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = LIM_DATA_ALARMCODE_Measurement_Failure;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_ALARM_QUERY, data); //Retrieve device-alarming status of LIM_DATA_ALARMCODE_Measurement_Failure.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = LIM_DATA_ALARMCODE_Measurement_Stopped;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_ALARM_QUERY, data); //Retrieve device-alarming status of LIM_DATA_ALARMCODE_Measurement_Stopped.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = LIM_DATA_ALARMCODE_Fog_Occluding;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_ALARM_QUERY, data); //Retrieve device-alarming status of LIM_DATA_ALARMCODE_Fog_Occluding.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = LIM_DATA_ALARMCODE_MNT_Field_Occluded;
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_ALARM_QUERY, data); //Retrieve device-alarming status of LIM_DATA_ALARMCODE_MNT_Field_Occluded.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_IOREAD,NULL); //Read status of I/O contacts.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			memset(data, 0, LIM_DATA_LEN*sizeof(unsigned int));
			data[0] = 0; // value
			data[1] = 1; // index
			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_IOSET, data); //Set status of I/O output contacts.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_IOSET_RELEASE, NULL); //Release setting I/O output contacts status.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_START_LMD); //Start LMD LIM transferring.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);
			Sleep(1000);

			Test_RT_Func_Switch();

			Test_RT_Func_Switch();

			LIM_Pack(lim, USER_TASK_CID, LIM_CODE_STOP_LMD); //Stop LMD LIM transferring.
			SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
			LIM_Release(lim);

			break;
		}

	}

	if (connected)
	{
		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_IOSET_RELEASE, NULL); //Release setting I/O output contacts status.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);


		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_STOP_LMD, NULL); //Stop LMD LIM transferring.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);
		Sleep(1000);
	}

	if (g_bRebootFlag == true)
	{
		LIM_Pack(lim, USER_TASK_CID, LIM_CODE_SYS_REBOOT, NULL); //Reboot the lidar.
		SendLIM(USER_TASK_CID, lim, lim->nLIMLen);
		LIM_Release(lim);

		Sleep(2000);
	}

	CloseEquipmentComm(USER_TASK_CID); //Close connection.

	EquipmentCommDestory(); //Close device-communication-DLL.

	printf("press any key to exit\n");
	getchar();

	return 0;
}

