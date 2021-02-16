#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#include "APDS9960.h"

#define APDS9960_GVALID         0b00000001
#define FIFO_PAUSE_TIME         30

int gesture_ud_delta_;
int gesture_lr_delta_;
int gesture_ud_count_;
int gesture_lr_count_;
int gesture_near_count_;
int gesture_far_count_;
int gesture_state_;
int gesture_motion_;

enum {
  DIR_NONE,
  DIR_LEFT,
  DIR_RIGHT,
  DIR_UP,
  DIR_DOWN,
  DIR_NEAR,
  DIR_FAR,
  DIR_ALL
};

/* State definitions */
enum {
  NA_STATE,
  NEAR_STATE,
  FAR_STATE,
  ALL_STATE
};

int APDS9960_setGestureGain(uint8_t gain)
{
	uint8_t val;

	if(!APDS_9960_ReadData(APDS9960_GCONF2, &val, 1))
		return 0;

	gain &= 0b00000011;
	gain = gain << 5;
	val &= 0b10011111;
	val |= gain;

	if(!APDS_9960_WriteData(APDS9960_GCONF2, &val, 1))
		return 0;

	return 1;
}

int APDS9960_setGestureLEDDrive(uint8_t drive)
{
	uint8_t val;

	if(!APDS_9960_ReadData(APDS9960_GCONF2, &val, 1))
		return 0;

	drive &= 0b00000011;
	drive = drive << 3;
	val &= 0b11100111;
	val |= drive;

	if(!APDS_9960_WriteData(APDS9960_GCONF2, &val, 1))
		return 0;

	return 1;
}

int APDS9960_setGestureWaitTime(uint8_t time)
{
	uint8_t val;

	if(!APDS_9960_ReadData(APDS9960_GCONF2, &val, 1))
		return 0;

	time &= 0b00000011;
	val &= 0b11111100;
	val |= time;

	if(!APDS_9960_WriteData(APDS9960_GCONF2, &val, 1))
		return 0;

	return 1;
}

int APDS9960_isGestureAvailable()
{
	uint8_t val;

	if( !APDS_9960_ReadData(APDS9960_GSTATUS, &val, 1) ) {
		return APDS9960_ERROR;
	}

	val &= APDS9960_GVALID;

	if( val == 1) {
		return 1;
	} else {
		return 0;
	}
}

/* processGestureData
 * [in] : u_data, d_data, l_data, r_data - 상하좌우 방향 데이터가 담긴 배열들
 * 			total_gestre - 각 데이터 배열의 크기
 * description : 각 데이터 배열을 processGestureData 함수의 인자로 넘겨줘서, 상하 좌우 방향으로 움직인 변화량, 절대값 등.. 을 계산한다.
 */
int processGestureData(uint8_t *u_data, uint8_t *d_data, uint8_t *l_data, uint8_t *r_data, int total_gesture) {
	uint8_t u_first = 0;
	uint8_t d_first = 0;
	uint8_t l_first = 0;
	uint8_t r_first = 0;
	uint8_t u_last = 0;
	uint8_t d_last = 0;
	uint8_t l_last = 0;
	uint8_t r_last = 0;
	int ud_ratio_first;
	int lr_ratio_first;
	int ud_ratio_last;
	int lr_ratio_last;
	int ud_delta;
	int lr_delta;
	int i;
	//제스쳐 임계를 넘는 유효한 초기 값을 구한다.
	for(i = 0; i < total_gesture; i++) {
		if((u_data[i] > GESTURE_THRESHOLD_OUT) &&
				(d_data[i] > GESTURE_THRESHOLD_OUT) &&
				(l_data[i] > GESTURE_THRESHOLD_OUT) &&
				(r_data[i] > GESTURE_THRESHOLD_OUT) ) {

			u_first = u_data[i];
			d_first = d_data[i];
			l_first = l_data[i];
			r_first = r_data[i];
			break;
		}
	}
	if( (u_first == 0) || (d_first == 0) || (l_first == 0) || (r_first == 0) ) {
		return 0;
	}
	//제스쳐 임계를 넘는 유효한 마지막 값을 구한다.
	for(i = total_gesture - 1; i >= 0; i--) {
		if( (u_data[i] > GESTURE_THRESHOLD_OUT) &&
				(d_data[i] > GESTURE_THRESHOLD_OUT) &&
				(l_data[i] > GESTURE_THRESHOLD_OUT) &&
				(r_data[i] > GESTURE_THRESHOLD_OUT) ) {

			u_last = u_data[i];
			d_last = d_data[i];
			l_last = l_data[i];
			r_last = r_data[i];
			break;
		}
	}
	//비율 재기
	ud_ratio_first = ((u_first - d_first) * 100) / (u_first + d_first);
	lr_ratio_first = ((l_first - r_first) * 100) / (l_first + r_first);
	ud_ratio_last = ((u_last - d_last) * 100) / (u_last + d_last);
	lr_ratio_last = ((l_last - r_last) * 100) / (l_last + r_last);

	//처음과 마지막의 변화량
	ud_delta = ud_ratio_last - ud_ratio_first;
	lr_delta = lr_ratio_last - lr_ratio_first;

	gesture_ud_delta_ += ud_delta;
	gesture_lr_delta_ += lr_delta;

	//변화량과 제스쳐 민감도 비교 
	if( gesture_ud_delta_ >= GESTURE_SENSITIVITY_1 ) {
		gesture_ud_count_ = 1;
	} else if( gesture_ud_delta_ <= -GESTURE_SENSITIVITY_1 ) {
		gesture_ud_count_ = -1;
	} else {
		gesture_ud_count_ = 0;
	}

	if( gesture_lr_delta_ >= GESTURE_SENSITIVITY_1 ) {
		gesture_lr_count_ = 1;
	} else if( gesture_lr_delta_ <= -GESTURE_SENSITIVITY_1 ) {
		gesture_lr_count_ = -1;
	} else {
		gesture_lr_count_ = 0;
	}

	//절대값이랑 민감도 비교
	if( (gesture_ud_count_ == 0) && (gesture_lr_count_ == 0) ) {
		if( (abs(ud_delta) < GESTURE_SENSITIVITY_2) && \
				(abs(lr_delta) < GESTURE_SENSITIVITY_2) ) {

			if( (ud_delta == 0) && (lr_delta == 0) ) {
				gesture_near_count_++;
			} else if( (ud_delta != 0) || (lr_delta != 0) ) {
				gesture_far_count_++;
			}
			//변화량에 따라 먼거리와 가까운 거리 재기
			if( (gesture_near_count_ >= 10) && (gesture_far_count_ >= 2) ) {
				if( (ud_delta == 0) && (lr_delta == 0) ) {
					gesture_state_ = NEAR_STATE;
				} else if( (ud_delta != 0) && (lr_delta != 0) ) {
					gesture_state_ = FAR_STATE;
				}
				return 1;
			}
		}
	} else {
		if( (abs(ud_delta) < GESTURE_SENSITIVITY_2) && \
				(abs(lr_delta) < GESTURE_SENSITIVITY_2) ) {

			if( (ud_delta == 0) && (lr_delta == 0) ) {
				gesture_near_count_++;
			}

			if( gesture_near_count_ >= 10 ) {
				gesture_ud_count_ = 0;
				gesture_lr_count_ = 0;
				gesture_ud_delta_ = 0;
				gesture_lr_delta_ = 0;
			}
		}
	}

	return 0;
}

/* decodeGesture 
 * [out] : decodeGesture 함수를 잘 실행했다는 의미로 1을 return 해줌.
 * description : 계산된 상화, 좌우 방향 변화량과 절댓값을 이용해 알맞은 제스처를 찾아서 gesture_motion_에 저장한다.
 */
int decodeGesture()
{
	if( gesture_state_ == NEAR_STATE ) {
		gesture_motion_ = DIR_NEAR;
		return 1;
	} else if ( gesture_state_ == FAR_STATE ) {
		gesture_motion_ = DIR_FAR;
		return 1;
	}

	//방향 재기
	if( (gesture_ud_count_ == -1) && (gesture_lr_count_ == 0) ) {
		gesture_motion_ = DIR_UP;
	} else if( (gesture_ud_count_ == 1) && (gesture_lr_count_ == 0) ) {
		gesture_motion_ = DIR_DOWN;
	} else if( (gesture_ud_count_ == 0) && (gesture_lr_count_ == 1) ) {
		gesture_motion_ = DIR_RIGHT;
	} else if( (gesture_ud_count_ == 0) && (gesture_lr_count_ == -1) ) {
		gesture_motion_ = DIR_LEFT;
	} else if( (gesture_ud_count_ == -1) && (gesture_lr_count_ == 1) ) {
		if( abs(gesture_ud_delta_) > abs(gesture_lr_delta_) ) {
			gesture_motion_ = DIR_UP;
		} else {
			gesture_motion_ = DIR_RIGHT;
		}
	} else if( (gesture_ud_count_ == 1) && (gesture_lr_count_ == -1) ) {
		if( abs(gesture_ud_delta_) > abs(gesture_lr_delta_) ) {
			gesture_motion_ = DIR_DOWN;
		} else {
			gesture_motion_ = DIR_LEFT;
		}
	} else if( (gesture_ud_count_ == -1) && (gesture_lr_count_ == -1) ) {
		if( abs(gesture_ud_delta_) > abs(gesture_lr_delta_) ) {
			gesture_motion_ = DIR_UP;
		} else {
			gesture_motion_ = DIR_LEFT;
		}
	} else if( (gesture_ud_count_ == 1) && (gesture_lr_count_ == 1) ) {
		if( abs(gesture_ud_delta_) > abs(gesture_lr_delta_) ) {
			gesture_motion_ = DIR_DOWN;
		} else {
			gesture_motion_ = DIR_RIGHT;
		}
	} else {
		return 0;
	}

	return 1;
}

/* resetGestureParameters
 * description : 제스처 인식에 사용되는 parameter들을 초기화 해줌.
 */
void resetGestureParameters()
{
	gesture_ud_delta_ = 0;
	gesture_lr_delta_ = 0;

	gesture_ud_count_ = 0;
	gesture_lr_count_ = 0;

	gesture_near_count_ = 0;
	gesture_far_count_ = 0;

	gesture_state_ = 0;
	gesture_motion_ = DIR_NONE;
}

int APDS9960_enableGesture(void)
{
	resetGestureParameters();

	return APDS9960_setMode(GESTURE, 1);
}

/* APDS9960_readGesture
 * [out] : 정수 motion 값을 return. 6가지 getsture를 1~6 의 번호로 값을 전달.
 * description : GVALID 값이 0이 아닌 동안에, 계속 제스처 센서에서 입력 값을 가져옴.
 */
int APDS9960_readGesture() {
	uint8_t fifo_level = 0;
	uint8_t byte_read = 0;
	uint8_t fifo_data[128];
	uint8_t gstatus;
	uint8_t u_data[32];
	uint8_t d_data[32];
	uint8_t l_data[32];
	uint8_t r_data[32];
	int motion;

	if( !APDS9960_isGestureAvailable()) 
		return 0;

	while(1) {
		usleep(FIFO_PAUSE_TIME * 1000);

		if(!APDS_9960_ReadData(APDS9960_GSTATUS, &gstatus, 1))
			return APDS9960_ERROR;
		//FIFO 레벨이 FIFO 임계값에 도달하면 GVALID 발생 GMODE와 GFLVL이 0이되면 리셋
		if((gstatus & APDS9960_GVALID) == APDS9960_GVALID) {
			//GFIFO 레벨 읽기
			if(!APDS_9960_ReadData(APDS9960_GFLVL, &fifo_level, 1))
				return APDS9960_ERROR;
			//GFIFO level은 현재 읽을 수 있는 데이터 세트 수를 표시
			if(fifo_level > 0) {
				APDS_9960_ReadData(APDS9960_GFIFO_U, fifo_data, (fifo_level * 4));
				// DEBUG 매크로(#define DEBUG)가 정의되어 있다면 #ifdef, #endif 사이의 코드를 컴파일
#ifdef DEBUG
				printf("Start\n");
				for(int i = 0; i < (fifo_level * 4); i++) {
					printf("%d\n",fifo_data[i]); 
				}
#endif
				int i, j;
				//fifo_level 값 만큼, up, down, left right 방향  data 값을 각각의 데이터 배열에 읽어온다.
				for(i = 0, j = 0; i < fifo_level; i++,j += 4) {
					u_data[i] = fifo_data[j + 0];
					d_data[i] = fifo_data[j + 1];
					l_data[i] = fifo_data[j + 2];
					r_data[i] = fifo_data[j + 3];
				}

				// 각 데이터 배열을 processGestureData 함수의 인자로 넘겨줘서, 상하 좌우 방향으로 움직인 변화량, 절대값 등.. 을 계산한다.
				if(processGestureData(u_data, d_data, l_data, r_data, fifo_level)) {
					
					// processGestureData 를 통해 계산 값들을 통해 인식된 gesture를 해석하고 gesture_motion_ 변수에 계속 update해줌.
					if(decodeGesture()) {

					}
				}
			}

		} else {
			usleep(FIFO_PAUSE_TIME * 1000);
			decodeGesture();
			motion = gesture_motion_;    
			// 제스처를 인식하기 위해 사용하는 parameter들을 모두 초기화 한다.
			resetGestureParameters();
			return motion;
		}
	}
}

/* APDS9960_printGesture
 * description : data 매개변수에 담긴 주소로 부터 size 만큼 크기의 값을 읽어와 센서의 register 주소에 찾아가 저장한다.
 */
void APDS9960_printGesture(void)
{
	
	// Gesture 인식이 가능한지 체크 해줌
	if(APDS9960_isGestureAvailable()) 
	{
		// APDS9960_isGestureAvailable() 함수의 리턴 값이 양수라면, Gesture 인식이 가능
		printf("Gesture : ");
		
		// 숫자 값으로 제스처를 리턴 받음.
		// 리턴 받은 숫자 값으로 그에 맞는 제스처를 출력해줌.
		switch(APDS9960_readGesture())
		{
			case DIR_UP:
				printf("UP\n");
				break;
			case DIR_DOWN:
				printf("DOWN\n");
				break;
			case DIR_LEFT:
				printf("LEFT\n");
				break;
			case DIR_RIGHT:
				printf("RIGHT\n");
				break;
			case DIR_NEAR:
				printf("NEAR\n");
				break;
			case DIR_FAR:
				printf("FAR\n");
				break;
			default:
				printf("FAILED\n");
				break;
		}

		sleep(1);
	}
}
