/**
* @filename:dummyev.c
* @brief: 模拟充电过程中串口数据的上传
*/
#include <string.h>
#include <lpc177x_8x_gpio.h>
#include "dummyev.h"
#include "framework.h"
#include "uart2_nCCS.h"

meter_value_status_t meterValueStatus;
plug_bms_status_temp_t plugBmsStatusTemp;
psm_status_t psmStatus[PSM_SIZE];
uint32_t insulationValue;
bcs_t bcs;
psm_output_t psmOutput;
ev_req_t ev_req;
bms_t bmsMsg;
brm_t brmMsg;
cml_temp_bcp_t cmlTempBcp;
bsm_t bsm;
uint32_t insulation;

uint8_t evReadyToCharge;	// 模拟电动汽车BMS准备好充电
psm_param_t psmParam;		// 用户电源模块参数设置


nccs_frame_t nccs_frame;
uint8_t frame_buf[50];


int dummyMeterValueStatus(int addr, meter_value_status_t* mvs, int size)
{
	static uint32_t mValue[2];
	uint32_t tempMeterValue;
	uint8_t bcd[sizeof(uint32_t)];
	uint8_t tempBcd;
	int i, len;
	memset(mvs, 0, size);
	mValue[addr-1]++;
	tempMeterValue = mValue[addr-1];
	for (i = 0; i < sizeof(uint32_t); i++) {
		tempBcd = tempMeterValue%100;
		bcd[i] = ((tempBcd/10)<< 4)+((tempBcd%10)&0xf);
		tempMeterValue/=100;
	}
	mvs->acenergy = *(uint32_t*)bcd;
	mvs->dcenergy = *(uint32_t*)bcd;
	mvs->accura = 101;
	mvs->accurb = 102;
	mvs->accurc = 103;
	mvs->acvola = 3801;
	mvs->acvolb = 3802;
	mvs->acvolc = 3803;
	mvs->sta.all = 0;
	mvs->sta.bit.auxivol12 = 0;
	mvs->sta.bit.auxivol24 = 0;
	mvs->sta.bit.acinputovervol = 0;
	mvs->sta.bit.gndcheck = 1;
	mvs->sta.bit.fancheck = 1;
	mvs->sta.bit.dccontactdriver1 = 1;
	
	nccs_frame.addr = addr;
	nccs_frame.order = 1;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)mvs;
	packageData(frame_buf, &nccs_frame, 1);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);	
	return 0;
}
int dummyPlugBmsStatusTemp(int addr, plug_bms_status_temp_t* pbst, int size)
{
	uint8_t pState, len, i = 0;
	uint8_t bmsStateBuf[] = {0, 16, 1, 2, 3, 4, 0x81, 0x82};
	static uint8_t bmsStatus[2];
	// memset(pbst, 0, size);
	if (addr == 1) {
		if (GPIO_ReadValue(0) & (1<<14)) { // p0.14检测是否连接电动汽车
			pState = 0;
		} else {
			pState = 2;
		}
	} else if (addr == 2) {
		if (GPIO_ReadValue(1) & (1<<22)) { // p1_22检测是否连接电动汽车
			pState = 0;
		} else {
			pState = 2;
		}
	}
	if (evReadyToCharge & (1<<addr)) {
		if (bmsStatus[addr-1] == 0) {
			bmsStatus[addr-1] = 0x10;
		} else if (bmsStatus[addr-1] == 0x10) {
			bmsStatus[addr-1] = 1;
		} else if (bmsStatus[addr-1] == 1) {
			bmsStatus[addr-1] = 2;
		} else if (bmsStatus[addr-1] == 4) {
			bmsStatus[addr-1] = 0;
		} else {
			for (i = 0; i < sizeof(bmsStateBuf)/sizeof(bmsStateBuf[0]); i++) {
				if (bmsStateBuf[i] == bmsStatus[addr-1]) {
					break;
				}
			}
			if (i >= sizeof(bmsStateBuf)/sizeof(bmsStateBuf[0])) {
				bmsStatus[addr-1] = 0;
			}
		}
	} else {
		if (pState == 2)
			bmsStatus[addr-1] = 4;
		else 
			bmsStatus[addr-1] = 0;
	}
	pbst->plugstatus = pState;
	pbst->bmsstatus = bmsStatus[addr-1];
	// pbst->status.all = 0;
//	pbst->status.bit.insulationfault = 0;
//	pbst->status.bit.lockdriver = 0;
//	pbst->status.bit.auxivoltage1224switch = 0;
//	pbst->status.bit.relais = 0;
	pbst->temperature = 66u;
//	pbst->status.bit.emergency = 0;
//	pbst->status.bit.dcmetercommfault = 0;
	
	nccs_frame.addr = addr;
	nccs_frame.order = 2;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)pbst;
	packageData(frame_buf, &nccs_frame, 1);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);	
	return 0;
}
int dummyPsmStatus(int addr, psm_status_t* psms, uint8_t size)
{
	int i, len;
	memset(psms, 0, size);
	for (i = 0; i < size/sizeof(psm_status_t); i++) {
		psms[i].id = 1;
		psms[i].temp = 60+i;
		psms[i].psm.t1.all = 0;
		psms[i].psm.t2.all = 0;
		psms[i].psm.t3.all = 0;
		if (i == 1) {
			psms[i].psm.t2.bit.fanwarning = 1;
		}
		if (i == 3) {
			psms[i].psm.t1.bit.outputshort = 1;
		}
	}
	
	nccs_frame.addr = addr;
	nccs_frame.order = 3;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)psms;
	packageData(frame_buf, &nccs_frame, 0);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);	
	return 0;
}
#define SOC_PREC	(15U)
#define REMAINTIME_PREC (10U)
int dummyBcs(int addr, bcs_t* bcs, int size) // Fixme: 
{
	int len;
	static uint16_t curBcs[2];
	static uint16_t volBcs[2];
	static uint16_t soc[2];
	static uint32_t remainTime[2];

	
	if (evReadyToCharge & (1<<addr)) {
		if (curBcs[addr-1] < psmParam.maxCur*10) curBcs[addr-1]++;
		if (volBcs[addr-1] < psmParam.maxVol*10) volBcs[addr-1]++;
		if (soc[addr-1] < 99*SOC_PREC) soc[addr-1]++;
		if (remainTime[addr-1] > 0) remainTime[addr-1]--;
	} else {
		curBcs[addr-1] = 0;
		volBcs[addr-1] = 0;
		soc[addr-1] = 0;
		remainTime[addr-1] = 65500*REMAINTIME_PREC;
		return 0;
	}
	
	bcs->batinfo = 90;
	bcs->cur = curBcs[addr-1]+4000;
	bcs->remaintime = remainTime[addr-1]/REMAINTIME_PREC;
	bcs->soc = soc[addr-1]/SOC_PREC;
	bcs->vol = volBcs[addr-1];
	
	nccs_frame.addr = addr;
	nccs_frame.order = 0x62;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)bcs;
	packageData(frame_buf, &nccs_frame, 0);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);	
	return 0;
}
int dummyBmsMsg(int addr, bms_t* bms, int size)
{
	int len;
	memset(bms, 0, size);
	
	bms->bem.msg0xAA = 1;
	
	bms->bsd.maxtemp = 100;
	bms->bsd.mintemp = 20;
	bms->bsd.singlemaxvol = 101;
	nccs_frame.addr = addr;
	nccs_frame.order = 0x5;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)bms;
	packageData(frame_buf, &nccs_frame, 0);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);
	
	return 0;
}
int dummyBrmMsg(int addr, brm_t* brm, int size)
{
	int len;
	memset(brm, 0, size);
	brm->batcapacity = 12345;
	nccs_frame.addr = addr;
	nccs_frame.order = 0x63;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)brm;
	packageData(frame_buf, &nccs_frame, 0);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);
	return 0;
}
int dummyCmlTempBcp(int addr, cml_temp_bcp_t* ctb, int size)
{
	int len;
	memset(ctb, 0, size);
	ctb->bcp.maxTemp = 10+50;
	ctb->bcp.nominalEnergy = 100;
	ctb->bcp.maxCur = 9+4000;
	ctb->bcp.maxChargingTotalVol = 700;
	ctb->bcp.batMaxChargingVol = 700;
	
	nccs_frame.addr = addr;
	nccs_frame.order = 0x64;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)ctb;
	packageData(frame_buf, &nccs_frame, 0);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);
	return 0;
}
int dummyBsm(bsm_t* bsm)  // Fixme: 
{
	bsm->bsm[0] = 10;
	return 0;
}
extern uint8_t checkInsulation;
int dummyInsulation(int addr, uint32_t* insulation, int size)
{
	int len;
	if (checkInsulation) {
		*insulation = 1234;
		
		nccs_frame.addr = addr;
		nccs_frame.order = 0x04;
		nccs_frame.size = size;
		nccs_frame.data = (uint8_t*)insulation;
		packageData(frame_buf, &nccs_frame, 1);
		len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
		UART2Send(LPC_UART2, frame_buf, len);
	}
	return 0;
}
int dummyPsmOutput(int addr, psm_output_t* psmOutput, int size)
{
	int len;
	static float cur[2];
	static float vol[2];

	if (evReadyToCharge & (1<<addr)) {
		if (cur[addr-1] < psmParam.maxCur) cur[addr-1] += 0.1;
		if (vol[addr-1] < psmParam.maxVol) vol[addr-1] += 0.1;
	} else {
		cur[addr-1] = 0;
		vol[addr-1] = 0;
		return 0;
	}
	psmOutput->cur = cur[addr-1];
	psmOutput->vol = vol[addr-1];
	
	nccs_frame.addr = addr;
	nccs_frame.order = 0x60;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)psmOutput;
	packageData(frame_buf, &nccs_frame, 1);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);
	return 0;
}
int dummyEvReq(int addr, ev_req_t* ev_req, int size)
{
	int len;
	static uint16_t req_cur[2];
	static uint16_t req_vol[2];

	if (evReadyToCharge & (1<<addr)) {
		if (req_cur[addr-1] < psmParam.maxCur) req_cur[addr-1]++;
		if (req_vol[addr-1] < psmParam.maxVol) req_vol[addr-1]++;
	} else {
		req_cur[addr-1] = 0;
		req_vol[addr-1] = 0;
		return 0;
	}
	ev_req->cur = req_cur[addr-1];
	ev_req->vol = req_vol[addr-1];
	
	nccs_frame.addr = addr;
	nccs_frame.order = 0x66;
	nccs_frame.size = size;
	nccs_frame.data = (uint8_t*)ev_req;
	packageData(frame_buf, &nccs_frame, 0);
	len = sizeof(nccs_frame)-sizeof(uint8_t*)+size;
	UART2Send(LPC_UART2, frame_buf, len);
	return 0;
}
int sendOnCharging()
{
	return 0;
}
int sendOnStopped()
{
	return 0;
}
int sendOnPeriod()
{
	int i;
	for (i = 1; i < 3; i++) {
		dummyMeterValueStatus(i, &meterValueStatus, sizeof(meterValueStatus));
		dummyPlugBmsStatusTemp(i, &plugBmsStatusTemp, sizeof(plugBmsStatusTemp));
		dummyPsmStatus(i, psmStatus, sizeof(psmStatus));
		dummyBrmMsg(i, &brmMsg, sizeof(brmMsg));
		dummyCmlTempBcp(i, &cmlTempBcp, sizeof(cmlTempBcp));
		dummyBmsMsg(i, &bmsMsg, sizeof(bmsMsg));
		dummyBcs(i, &bcs, sizeof(bcs));
		dummyInsulation(i, &insulation, sizeof(insulation));
		dummyPsmOutput(i, &psmOutput, sizeof(psmOutput));
		dummyEvReq(i, &ev_req, sizeof(ev_req));
	}
	timeTickStart(TIMER_TICK_THREAD_DUMMYEV_PERIOD, 100);
	return 0;
}

int initDummyEv(void)
{
	creatTimerTickFun(sendOnCharging, TIMER_TICK_THREAD_DUMMYEV_CHARGING);
	creatTimerTickFun(sendOnStopped, TIMER_TICK_THREAD_DUMMYEV_STOPPED);
	creatTimerTickFun(sendOnPeriod, TIMER_TICK_THREAD_DUMMYEV_PERIOD);
	timeTickStart(TIMER_TICK_THREAD_DUMMYEV_PERIOD, 100);
	return 0;
}

