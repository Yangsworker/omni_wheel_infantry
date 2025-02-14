#ifndef _MY_RC_
#define _MY_RC_

#include "board.h"
#include "KeyProcess.h"



#pragma pack(1)

class MyRC
{
public:
	MyRC();
	void myRC_Update(void);

	enum VisionColor_t{
		BLUE = 0,
		GG = 1,
		RED = 2
	};//�Ӿ���ɫ0��2��1��

	/*��̨���***************/
	float mySpdSetZ;		//�����С���̨�����ٶ��趨
	float mySpdSetP;		//��̨�����ٶ��趨
/**/uint8_t visionMode;		//�����С��Ӿ�ģʽ 0:δ����|1:����|2:����|3:����|4:С��|5:���
	bool lookBack;			//���ٻ�ͷ
	bool baseShootSW;		//�����빤�̻�����
	bool visionYawOnly;		//�Ӿ�����Y��ģʽ
	bool longFocusSwitch;	//�������ģʽ
	bool laserOn;			//���ü���
	
	/*�������***************/
	bool if_Ctrl;			    //�����С��Ƿ�Ctrl
	bool defollowFlag;		//�����С����̸���ȡ��
	int16_t mySpdSetY;		//�����С�����ǰ���ٶ��趨
	int16_t mySpdSetX;		//�����С����������ٶ��趨
	uint8_t AvoidMode;		//�����С�0-normal 1-����ģʽ 2-��¼ 3-��¼
	bool if_Shift;			  //�����С��Ƿ�Shift
	bool SpeedUp_level;   //�����С����ٵȼ�
  bool flySloping;		  //�����С�����ģʽ
  bool useCapacitor = 1;	//�����С�ʹ�õ���
  bool Rotation_HL;		  //�����С��ߵ�����
  bool ULTS;				    //�����С�ULTSģʽ
  bool CheckMode;			  //�����С���¼ģʽ

	/*Ħ�������*************/
	bool friRun;			//��Ħ����

	/*�������***************/
	bool fireTrig;			//������
	u8 Firefrequency;			//��Ƶ 0:����|1:10��Ƶ|2:20��Ƶ
	bool AutoFire;			// �Զ�����
	bool AutoFire_buff; //����Զ�����

	/*���ָ����*************/
	bool coverCal;			//���ָ�У׼
	bool coverState;		//���ָ�״̬ 0:δ�� | 1:�Ѵ�
	
	/*ϵͳ���***************/
/**/bool topCtrlRst;		//����������
/**/bool botCtrlRst;		//�����С�����������
/**/bool UIInit;			//�����С�UI��ʼ��
	uint8_t whatWheelCtrl:3;//�����С�����ʹ�ù��ֵ���ʲô 0:���̸߶� 1:roll�� 2:���P 3:���Y
	bool VTX;
	uint8_t VTXplan_ID;
		
	//�ɱ������[����flash��]
	union{
		uint8_t paramInFlash[8];//�����ڻ���
		struct{
			uint8_t visionColor;	//�Ӿ���ɫ
			int8_t	BuffPOffset:5;	//���P��ƫ�õ���		
			int8_t	BuffYOffset:5;	//���Y��ƫ�õ���
			int8_t	VisPOffset:5;	//���P��ƫ�õ���		
			int8_t	VisYOffset:5;	//���Y��ƫ�õ���
		};
	};	

	void FlashParamInit(void);
	void FlashParamSave(void);
private:
	//˫�������
	doubleKickJudge* visionKick;//����ģʽ���Ҽ���
	doubleKickJudge* buffKick;	//����ģʽ��Q��
	doubleKickJudge* avoidKick;	//���ģʽ��E��
	doubleKickJudge* powerKick;	//����ģʽ��G��
	doubleKickJudge* lFocusKick;//����ģʽ��X��
	//������ʱ���
	float lookBackTimeStamp;	//���ٻ�ͷʱ���
	float ULTSTimeStamp;		//ULTSʱ���
	//����˽�б���
	// int32_t wheelCodeOffset;	//���ֱ���ƫ��
	
	bool contiSwitch(uint8_t Key);
};

#pragma pack()

extern MyRC myRC;




#endif

