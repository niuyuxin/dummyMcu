
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

typedef struct { // С�ˣ�����Ҫת���ֽ���
	uint16_t vol;
	uint16_t cur;
	uint16_t batinfo; // ��ߵ��嶯�����ص�ѹ�������
	uint8_t soc;
	uint16_t remaintime;
}bcs_t;
typedef struct {
	float cur;
	float vol;
}psm_output_t;

typedef struct { // С�˷��Ͳ���Ҫת���ֽ���
	struct {
		uint8_t soc:2; // 00=δ�ﵽ����SOCĿ��ֵ��01=�ﵽ����SOCĿ��ֵ��10=������״̬
		uint8_t wholevol:2; // 00=δ�ﵽ�ܵ�ѹ�趨ֵ��01=�ﵽ�ܵ�ѹ�趨ֵ��10=������״̬
		uint8_t singlevol:2; // 00=δ�ﵽ�����ѹ�趨ֵ��01=�ﵽ�����ѹ�趨ֵ��10=������״̬
		uint8_t state:2; // 00=���� 01=���׮��ֹ 10=������
	}stopreason;
	struct {
		uint8_t insulationfault:2; // ��Ե���ϣ�00=>������01=>���ϣ�10=>������״̬��
		uint8_t outputcontactovertempfault:2; // ������������¹��ϣ�00=>������01=>���ϣ�10=>������״̬��
		uint8_t bmscomponentfault:2; // BMSԪ����������������£�00=>������01=>���ϣ�10=>������״̬��
		uint8_t chargingcontactfault:2; // ������������ϣ�00=>������01=>���ϣ�10=>������״̬��
		uint8_t batovertempfault:2; // ������¶ȹ��߹��ϣ�00=>�¶�������01=>�¶ȹ��ߣ�10=>������״̬��
		uint8_t highvolrelayfault:2; // ��ѹ�̵������ϣ�00=>������01=>���ϣ�10=>������״̬��
		uint8_t point2volfault:2; // ����2��ѹ�����ϣ�00=>������01=>���ϣ�10=>������״̬��
		uint8_t otherfault:2; // �������ϣ�00=>������01=>���ϣ�10=>������״̬��
	} faultstopreason;	
	struct {
		uint8_t overcurrent:2; // �������ϣ�00=>������01=>���ϣ�10=>������״̬��
		uint8_t overvoltage:2; // ��ѹ�쳣��00=>������01=>��ѹ�쳣��10=>������״̬
		uint8_t reserved:4; //
	} errorstopreason;	
} bst_t;
typedef struct { // С�˷��ͣ�����Ҫת���ֽ���
	struct {
		uint8_t satisfiedcondition:2; // �ﵽ�����趨��������ֹ��00=>������01=>�ﵽ�����趨��������ֹ��10=>������״̬��
		uint8_t userstop:2; // �˹���ֹ��00=>������01=>�˹���ֹ��10=>������״̬��
		uint8_t faultstop:2; // ������ֹ��00=>������01=>������ֹ��10=>������״̬��
		uint8_t bmsstop:2; // BMS������ֹ��00=>������01=>BMS��ֹ(�յ�BST)��10=>������״̬��
	} stopreason;
	struct {
		uint8_t overtemp:2; // 00=>�����¶�������01=>�������£�10=>������״̬��
		uint8_t chargingcontact:2; // 00=>���������������01=>������������ϣ�10=>������״̬��
		uint8_t insideovertemp:2; // 00=>�����ڲ��¶�������01=>�����ڲ����£�10=>������״̬��
		uint8_t energytransmit:2; //  00=>���ܴ���������01=>���ܲ��ܴ��ͣ�10=>������״̬��
		uint8_t emergency:2; // ������ͣ���ϣ�00=>������01=>������ͣ��10=>������״̬��
		uint8_t other:2; // �������ϣ�00=>������01=>���ϣ�10=>������״̬��
		uint8_t reserved:4; // 
	} faultstopreason;	
	struct {
		uint8_t matchcur:2; // 00=>����ƥ�䣬01=>������ƥ�䣬10=>������״̬��
		uint8_t matchvol:2; // 00=>��ѹ������01=>��ѹ�쳣��10=>������״̬��
	} errorstopreason;	
} cst_t;
typedef struct {
	uint8_t stopsoc; // ��ֹ�ɵ�״̬SOC��1����1%��0~100%
	uint16_t singleminvol;	// 	�������ص�����͵�ѹ��2����0.01V��0~24V
	uint16_t singlemaxvol;	// 	�������ص�����ߵ�ѹ��2����0.01V��0~24V
	uint8_t mintemp; //	������������¶ȣ�1����1�棬-50��ƫ������-50��~200��
	uint8_t maxtemp; // ������������¶ȣ�1����1�棬-50��ƫ������-50��~200��
} bsd_t;
typedef struct {
	uint8_t msg0x0:2; // ���ձ�ʶ���Ϊ0x00�ĳ�����ʶ���ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t msg0xAA:2; // ���ձ�ʶ���Ϊ0xAA�ĳ�����ʶ���ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t reserved:4; // 
	uint8_t timesync_maxoutput:2; // ���ճ�����ʱ��ͬ���ͳ����������������ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t finishedcharging:2; // ���ճ�����ɳ��׼�����ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t reserved1:4; // 
	uint8_t chargingstate:2; // ���ճ������״̬���ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t stopcharging:2; // ���ճ������״̬���ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t reserved2:4; // 
	uint8_t statistic:2; // ���ճ������ͳ�Ʊ��ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t reserved3:6; // 
} bem_msg_timeout_t;
typedef struct {
	uint8_t bms_ev:2; // ����BMS�ͳ����ı�ʶ���ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t reserved:6; // 
	uint8_t batparam:2; // ���յ�س��������ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t bmsfinishedcharging:2; //  ����BMS��ɳ��׼�����ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t reserved1:4; // 
	uint8_t batchargingstate:2; // ���յ�س����״̬���ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t batchargingrequest:2; // ���յ�س��Ҫ���ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t batstopcharging:2; // ����BMS��ֹ��籨�ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t reserved2:2; // 
	uint8_t bmsstatistic:2; // ����BMS���ͳ�Ʊ��ĳ�ʱ��00=>������01=>��ʱ��10=>������״̬��
	uint8_t others:6; // ����
}cem_msg_t;
typedef struct { // ����Ҫת���ֽ���
	bst_t bst;
	cst_t cst;	
	bsd_t bsd;	
	bem_msg_timeout_t bem;	
	cem_msg_t cem;	
} bms_t;
typedef struct { // ����Ҫת���ֽ���
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

extern uint8_t evReadyToCharge; // ģ��綯����BMS׼���ó��
extern psm_param_t psmParam;	// �û���Դģ���������

int initDummyEv(void);

#endif
