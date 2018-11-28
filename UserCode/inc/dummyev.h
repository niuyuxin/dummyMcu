
#ifndef DUMMYEV_H
#define DUMMYEV_H
#include <stdint.h>

#define PSM_SIZE 5
#pragma pack(1)
typedef struct {
	union {
		uint32_t all;
		struct {
			uint8_t acmeterfault:1;
			uint8_t acinputovervol:1;
			uint8_t acinputundervol:1;
			uint8_t acinputphaseerr:1;
			uint8_t gndcheck:1;
			uint8_t fancheck:1;
			uint8_t lowmcuerr:1;
			uint8_t psmcomerr:1;
			uint8_t auxivol12:1;
			uint8_t auxivol24:1;
			uint8_t reserved1:2;
			uint8_t reserved2:4;
			uint8_t reserved3:4;
			uint8_t dccontactdriver1:1;
			uint8_t dccontactdriver2:1;
			uint8_t dccontactdriver3:1;
			uint8_t dccontactdriver4:1;
			uint8_t dccontactattachonopen1:1;
			uint8_t dccontactattachonopen2:1;
			uint8_t dccontactattachonopen3:1;
			uint8_t dccontactattachonopen4:1;
			uint8_t dccontactattachonclose1:1;
			uint8_t dccontactattachonclose2:1;
			uint8_t dccontactattachonclose3:1;
			uint8_t dccontactattachonclose4:1;
		}bit;
	}sta;
	uint32_t dcenergy;
	uint16_t accurc;
	uint16_t accurb;
	uint16_t accura;
	uint16_t acvolc;
	uint16_t acvolb;
	uint16_t acvola;
	uint32_t acenergy;	
} meter_value_status_t;
typedef struct {
	int8_t temperature;
	union {
		uint16_t all;
		struct {
			uint8_t emergency:1;
			uint8_t insulationabnormal:1;
			uint8_t insulationfault:1;
			uint8_t lockattach:1;
			uint8_t lockdriver:1;
			uint8_t tempabnormal:1;
			uint8_t dcmetercommfault:1;
			uint8_t relais:1;
			uint8_t auxivoltage1224switch:1;
			uint8_t auxivoltageoutput:1;
			uint8_t ischeckinginsulation:1;
			uint8_t overload:1;
			uint8_t auxivoltage1224:1;
			uint8_t reserved:3;
		} bit;
	} status;	
	uint8_t bmsstatus;	
	uint8_t plugstatus;	
} plug_bms_status_temp_t;

typedef struct {
	union {
		uint8_t all;
		struct {
			uint8_t powerlimitstate:1;
			uint8_t idduplicated:1;
			uint8_t balancedcurrenterror:1;
			uint8_t inputphasewarning:1;
			uint8_t inputbalancedwarning:1;
			uint8_t inputundervolwarning:1;
			uint8_t inputovervolwarning:1;
			uint8_t pfcshutdown:1;
		} bit;
	} t3;	
	union {
		uint8_t all;
		struct {
			uint8_t dcshutdown:1;
			uint8_t faultwarning:1;
			uint8_t protectedwaring:1;
			uint8_t fanwarning:1;
			uint8_t tempwarning:1;
			uint8_t walkinenable:1;
			uint8_t commbreak:1;
			uint8_t reserved:1;
		} bit;
	} t2;		
	union {
		uint8_t all;
		struct {
			uint8_t outputshort:1;
			uint8_t reserved:3;
			uint8_t sleep:1;
			uint8_t reserved1:3;
		} bit;
	}t1;	
}psm_t;

typedef struct {
	uint8_t id;
	int8_t temp;
	psm_t psm;	
}psm_status_t;

typedef struct { // 小端，不需要转换字节序
	uint16_t vol;
	uint16_t cur;
	uint16_t batinfo; // 最高单体动力蓄电池电压及其组号
	uint8_t soc;
	uint16_t remaintime;
}bcs_t;
typedef struct {
	float cur;
	float vol;
}psm_output_t;

typedef struct { // 小端发送不需要转换字节序
	struct {
		uint8_t soc:2; // 00=未达到所需SOC目标值，01=达到所需SOC目标值，10=不可信状态
		uint8_t wholevol:2; // 00=未达到总电压设定值，01=达到总电压设定值，10=不可信状态
		uint8_t singlevol:2; // 00=未达到单体电压设定值，01=达到单体电压设定值，10=不可信状态
		uint8_t state:2; // 00=正常 01=充电桩终止 10=不可信
	}stopreason;
	struct {
		uint8_t insulationfault:2; // 绝缘故障：00=>正常，01=>故障，10=>不可信状态；
		uint8_t outputcontactovertempfault:2; // 输出连接器过温故障：00=>正常，01=>故障，10=>不可信状态；
		uint8_t bmscomponentfault:2; // BMS元件、输出连接器过温：00=>正常，01=>故障，10=>不可信状态；
		uint8_t chargingcontactfault:2; // 充电连接器故障：00=>正常，01=>故障，10=>不可信状态；
		uint8_t batovertempfault:2; // 电池组温度过高故障：00=>温度正常，01=>温度过高，10=>不可信状态；
		uint8_t highvolrelayfault:2; // 高压继电器故障：00=>正常，01=>故障，10=>不可信状态；
		uint8_t point2volfault:2; // 检测点2电压检测故障：00=>正常，01=>故障，10=>不可信状态；
		uint8_t otherfault:2; // 其他故障：00=>正常，01=>故障，10=>不可信状态；
	} faultstopreason;	
	struct {
		uint8_t overcurrent:2; // 其他故障：00=>正常，01=>故障，10=>不可信状态；
		uint8_t overvoltage:2; // 电压异常：00=>正常，01=>电压异常，10=>不可信状态
		uint8_t reserved:4; //
	} errorstopreason;	
} bst_t;
typedef struct { // 小端发送，不需要转换字节序
	struct {
		uint8_t satisfiedcondition:2; // 达到充电机设定的条件终止：00=>正常，01=>达到充电机设定的条件终止，10=>不可信状态；
		uint8_t userstop:2; // 人工终止：00=>正常，01=>人工终止，10=>不可信状态；
		uint8_t faultstop:2; // 故障终止：00=>正常，01=>故障终止，10=>不可信状态；
		uint8_t bmsstop:2; // BMS主动终止：00=>正常，01=>BMS终止(收到BST)，10=>不可信状态；
	} stopreason;
	struct {
		uint8_t overtemp:2; // 00=>充电机温度正常，01=>充电机过温，10=>不可信状态；
		uint8_t chargingcontact:2; // 00=>充电连接器正常，01=>充电连接器故障，10=>不可信状态；
		uint8_t insideovertemp:2; // 00=>充电机内部温度正常，01=>充电机内部过温，10=>不可信状态；
		uint8_t energytransmit:2; //  00=>电能传送正常，01=>电能不能传送，10=>不可信状态；
		uint8_t emergency:2; // 充电机急停故障：00=>正常，01=>充电机急停，10=>不可信状态；
		uint8_t other:2; // 其他故障：00=>正常，01=>故障，10=>不可信状态；
		uint8_t reserved:4; // 
	} faultstopreason;	
	struct {
		uint8_t matchcur:2; // 00=>电流匹配，01=>电流不匹配，10=>不可信状态；
		uint8_t matchvol:2; // 00=>电压正常，01=>电压异常，10=>不可信状态；
	} errorstopreason;	
} cst_t;
typedef struct {
	uint8_t stopsoc; // 终止荷电状态SOC（1）：1%，0~100%
	uint16_t singleminvol;	// 	动力蓄电池单体最低电压（2）：0.01V，0~24V
	uint16_t singlemaxvol;	// 	动力蓄电池单体最高电压（2）：0.01V，0~24V
	uint8_t mintemp; //	动力蓄电池最低温度（1）：1℃，-50℃偏移量，-50℃~200℃
	uint8_t maxtemp; // 动力蓄电池最高温度（1）：1℃，-50℃偏移量，-50℃~200℃
} bsd_t;
typedef struct {
	uint8_t msg0x0:2; // 接收辨识结果为0x00的充电机辨识报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t msg0xAA:2; // 接收辨识结果为0xAA的充电机辨识报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t reserved:4; // 
	uint8_t timesync_maxoutput:2; // 接收充电机的时间同步和充电机最大输出能力报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t finishedcharging:2; // 接收充电机完成充电准备报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t reserved1:4; // 
	uint8_t chargingstate:2; // 接收充电机充电状态报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t stopcharging:2; // 接收充电机充电状态报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t reserved2:4; // 
	uint8_t statistic:2; // 接收充电机充电统计报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t reserved3:6; // 
} bem_msg_timeout_t;
typedef struct {
	uint8_t bms_ev:2; // 接收BMS和车辆的辨识报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t reserved:6; // 
	uint8_t batparam:2; // 接收电池充电参数报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t bmsfinishedcharging:2; //  接收BMS完成充电准备报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t reserved1:4; // 
	uint8_t batchargingstate:2; // 接收电池充电总状态报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t batchargingrequest:2; // 接收电池充电要求报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t batstopcharging:2; // 接收BMS终止充电报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t reserved2:2; // 
	uint8_t bmsstatistic:2; // 接收BMS充电统计报文超时：00=>正常，01=>超时，10=>不可信状态；
	uint8_t others:6; // 其他
}cem_msg_t;
typedef struct { // 不需要转换字节序
	bst_t bst;
	cst_t cst;	
	bsd_t bsd;	
	bem_msg_timeout_t bem;	
	cem_msg_t cem;	
} bms_t;
typedef struct { // 不需要转换字节序
	uint16_t batcapacity;
	uint8_t vincode[17];
} brm_t;
typedef struct {
	uint16_t batMaxChargingVol;
	uint16_t maxCur;
	uint16_t nominalEnergy;
	uint16_t maxChargingTotalVol;
	uint8_t maxTemp;
	uint16_t batHopowerState;
	uint16_t batTotalVol;
} bcp_t;
typedef struct {
	uint8_t cml[8];
	uint8_t maxtemp;	
	bcp_t bcp;	
} cml_temp_bcp_t;
typedef struct {
	uint8_t bsm[5];
}bsm_t;

typedef struct {
	uint16_t minCur;
	uint16_t minVol;
	uint16_t maxCur;
	uint16_t maxVol;
}psm_param_t;
	
#pragma pack()

extern meter_value_status_t meterValueStatus;
extern plug_bms_status_temp_t plugBmsStatusTemp;
extern psm_status_t psmStatus[PSM_SIZE];
extern uint32_t insulationValue;
extern bcs_t bcs;
extern psm_output_t psmOutput;
extern bms_t bmsMsg;
extern brm_t brmMsg;
extern cml_temp_bcp_t cmlTempBcp;
extern bsm_t bsm;

extern uint8_t evReadyToCharge; // 模拟电动汽车BMS准备好充电
extern psm_param_t psmParam;	// 用户电源模块参数设置

int initDummyEv(void);

#endif
