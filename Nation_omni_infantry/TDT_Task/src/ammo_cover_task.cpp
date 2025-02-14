#include "ammo_cover_task.h"
#include "dbus.h"
#include "pid.h"
#include "my_math.h"


PidParam BulletInner, BulletOuter;
AmmoCover ammoCover;
const float position_rota = -2500;  //�г̣���ʾ���λ��

void Ammo_Task()
{
	if(deforceFlag || (RC.Key.B && RC.Key.CTRL)) {ammoCover.ifstartoffset = 1;} //�������/��Ctrl+E���������㿪ʼУ׼
	if(!deforceFlag) 
	{
		if(ammoCover.ifstartoffset )  
		{
			ammoCover.Ammo_offset(20, 5000, 8000); //0.4s
		}
		if(!ammoCover.offset_start)
		{
		ammoCover.Ammo_open(); 
		ammoCover.auto_close();
		}
	}
}
//���������pid������ֵ 
AmmoCover::AmmoCover()
{
	Bullet  = new Motor (M2006, CAN2, 0x204);
	
	BulletInner.kp = 13;   //12 30 0 10 9000
	BulletInner.ki = 30;
	BulletInner.kd = 0;
	BulletInner.integralErrorMax = 0;
	BulletInner.resultMax = 9000;
	
	BulletOuter.kp = 11;
	BulletOuter.ki = 15;
	BulletOuter.kd = 0;
	BulletOuter.integralErrorMax = 1;
	BulletOuter.resultMax = 3000;
	
  Bullet ->pidInner.setPlanNum(1);
	Bullet ->pidOuter.setPlanNum(1);
	
	Bullet-> pidInner.paramPtr = &BulletInner;
	Bullet-> pidOuter.paramPtr = &BulletOuter;
	
	Bullet ->pidInner.fbValuePtr[0] = &Bullet ->canInfo.speed;
	Bullet ->pidOuter.fbValuePtr[0] = &Bullet ->canInfo.totalAngle_f;
}


//���ո�У׼����--У׼���λ�� 
bool AmmoCover::Ammo_offset(float reSetSpeed, float maxErr, float outLimit)
{
   switch(ammoCover.count)
		{
			case 0:
				//��¼ԭʼ����޷�ֵ
				ammoCover.outLimitTemp = this -> Bullet -> pidInner.paramPtr ->resultMax;
			  //��ȡ����yaw��̨����totalangleֵ
			  ammoCover.offsetPos = *(this -> Bullet -> pidOuter.fbValuePtr[0]);
				ammoCover.count++;
				break;
			case 1: 
				//����pid��������޷�Ϊ�趨ֵ
			  this -> Bullet -> pidInner.paramPtr -> resultMax = outLimit;
			  //����У׼�ٶȣ���ʼУ׼
			  ammoCover.offset_start = 1;
			  ammoCover.offsetPos += reSetSpeed;
			  this -> Bullet -> ctrlPosition(ammoCover.offsetPos);
			  //��ת���
			  if(ABS(Bullet -> pidOuter.error) > maxErr)   //�����ڴ˼���Ƚϴ���
				{ ammoCover.count++; }
				break;
			case 2: 
				//��ȡ�����еλ��,���λ�ø�0
				int16_t offsetEncoderTemp = Bullet -> canInfo.encoder;
			  memset(&(Bullet -> canInfo), 0, sizeof(Bullet -> canInfo));
			  Bullet -> canInfo.offsetEncoder = offsetEncoderTemp;
			  //��������ۼ����ʵ�pid����
			  Bullet -> pidOuter.Clear();
			  Bullet -> ctrlPosition(0);
				ammoCover.count = 0;
			  //�ָ�pid����޷�Ϊ����ģʽ
			  Bullet -> pidInner.paramPtr -> resultMax = ammoCover.outLimitTemp;
			  //���У��ɹ�����ֵ
			 	ammoCover.ifstartoffset = 0; //У׼��ɹر�У׼�ӿ�
			  ammoCover.offset_start = 0;
			  open_close = 0; //У׼��˲�䲻�������ո�
				return 1; 
		}	
  return 0;

}




//���տ���(0.6s CD)
int open_CD = 0;
float bullet_current_now = 0, bullet_position_now = 0, bullet_position_error = 0;
float fuck_time = 0, fuck_speed = 1500, fuck_times = 0;
bool mode_change = 0, if_fuck = 0;
void AmmoCover::Ammo_open(void)     
{
	bullet_current_now = ABS(Bullet->canInfo.trueCurrent);
	bullet_position_now = Bullet->canInfo.totalAngle_f;
	bullet_position_error = ABS(bullet_position_now - position_set);
	mode_change = 0;
//	if(!offset_start && !if_fuck) //У׼���תʱ��������
	if(!if_fuck)
	{
	  if(RC.Key.B && !open_CD){ 
	    open_close = !open_close; 
			mode_change = 1;  //����ʱ��ֵ����־λ
			open_CD = 1;}
		if(open_CD) {
	    open_CD++;
		  if(open_CD > 300) {open_CD = 0;}
		}
		
		if(mode_change)
		{position_set = bullet_position_now;}
	  if(!open_close) //�ص���
	  {
			if(position_set <= -150) 
		  {
		  	position_set += 20;
		  }
			else
			{
				position_set = -150;
			}
	  }
	  if(open_close)  //������
	  {
		  if(position_set >= position_rota)
		  {
			  position_set -= 20;
		  }
			else
			{
				position_set = position_rota;
			}
	  }
	  Bullet->ctrlPosition(position_set);
 }
//ʱ�̼���Ƿ��ת
 if(bullet_position_error > 800)
 {
	 fuck_times++;
	 if(fuck_times >= 1000)
	 {
		 fuck_times = 0;
	 if_fuck = 1;
	 }
 }
 else if(bullet_position_error < 800)
 {fuck_times = 0;}
 if(if_fuck)
 {
	 fuck_time++;
	 if(open_close) //������
	 {
		 Bullet->ctrlSpeed(fuck_speed);
	 }
	 else           //�ص���
	 {
		 Bullet->ctrlSpeed(-fuck_speed);
	 }
	 if(fuck_time >= 100)
	 {
		 if_fuck = 0;
		 position_set = bullet_position_now;
		 fuck_time = 0;
	 }
 }
}

//�жϵ��ո��Ƿ��(λ�ü����ж�)
void AmmoCover::Coveropen_Judge()
{
	bullet_position_now = Bullet -> canInfo.totalAngle_f;
	if(bullet_position_now < -2300) {coverOpen = 1;}
	if(bullet_position_now > -500) {coverOpen = 0;}
}
void AmmoCover::auto_close(void)
{
	Coveropen_Judge();
	//���ոǿ��������ҵ����ƶ�--�رյ���
	if(chassis.judgeIfMoving() && coverOpen && !offset_start && !chassis.rotateFlag && !chassis.flexRotate) 
	{
		ifchassismove = 1;
		open_close = 0;
	}
}


	