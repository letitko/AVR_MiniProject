#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "ledmatrix7219d88.h"

// * 필독 : AVR에 소스코드를 업로딩 할 때에 반드시 HC-SR04[초음파 거리 감지센서]의 트리거와 에코 단자를 빼서 통신을 OFF 시켜야 업로딩이 가능함.
// RC-CAR 소스: 코딩 순서[참고] (블루투스 모듈 : Name/Pass = RC/5683)
// 코딩 순서[참고] [정의코드/변수/함수선언 및 기타코드/메인 순]

/*
	01. 블루투스 송수신 설정	사용된 포트 UART0 포트 (E포트)
	02. DC모터 드라이버 설정	사용된 포트 PORTF,PORTA (DC모터 드라이버 전원 = 2개의 DC 모터 드라이버 공통으로 : 보조배터리의 VCC, AVR의 VCC 사용
																										  및 보조배터리로 공통접지)
	03. 서보모터 동작   설정	사용된 포트 PORTE
	04. 초음파 감지센서 설정	사용된 포트 PORTB
	05. LED도트매트릭스 설정	사용된 포트 PORTD
	06. 부저           설정	사용된 포트 PORTC
	07. 라이트		   설정 사용된 포트 PORTA
												*/

// ##### 정의코드

// 01. 블루투스 - 없음

// 02. DC모터 드라이버 - 없음

// 03. 서보모터

	#define ROTATION_DELAY	4000

// 04. 초음파 감지센서

	#define trig_1 PORTB|=1  // PB0, srf04 트리거 (송신)
	#define trig_0 PORTB&=~1 // PB0, srf04 트리거 (송신)
	#define echo   (PINB&2)  // PB1, srf04 에코 (수신)

// 05. LED도트매트릭스 - 파일 첨부

// 06. 부저 - 없음

// 07. 라이트 -없음


// ##### 변수

// 01. 블루투스

	volatile unsigned char flag;

// 02. DC모터 드라이버 - 없음

// 03. 서보모터

	int PULSE_MID = 3000; // 3000 = 0도 <기준> // 각도조절 좌현 90도 (1300)) , 우현조절 90도 4700)

// 04. 초음파 감지센서

	unsigned int cnt;

// 05. LED도트매트릭스 - 파일 첨부

// 06. 부저 - 없음

// 07. 라이트 - 없음


// ##### 함수선언 및 기타코드

// 01. 블루투스

	ISR(USART0_RX_vect)
	{
		flag=UDR0;
	}

	void init()
	{
		UCSR0A=0x00;
		UCSR0B=0x98;
		UCSR0C=0x06;
		UBRR0H=0;
		UBRR0L=103;
		SREG=0x80;
	}

// 02. DC모터 드라이버

	void DC_Forward(void)

	{
		PORTF=0b10101010; // 전진
	}

	void DC_Backward(void)

	{
		PORTF=0b01010101; // 후진
	}

	void DC_Stop(void)
	{
		PORTF=0x00;	  // 정지
	}

// 03. 서보모터
	
	void InitioalizeTimer(void)
	
	{
		// 모드 14, 고속 PWM 모드
		TCCR3A |= (1 << WGM11);
		TCCR3B |= (1 <<WGM12) | (1 << WGM13);
	
		// 비반전 모드
		// TOP:ICR1, 비교 일치값:OCR3A 레지스터
		TCCR3A |= (1 << COM1A1);
	
		TCCR3B |= (1 << CS11);	// 분주율 8, 2MHz
		
		ICR3 = 39999;	//  20ms 주기
	}

// 04. 초음파 감지센서

	void srf04(){

	trig_1; //트리거
	_delay_us(10);
	trig_0; //트리거

	while(!echo);		// 신호가 high가 될 때까지 대기
	TCNT1=0; TCCR1B=2;	// 카운터 시작, 8분주 0.5us
	while(echo);		// 신호가 low가 될 때까지 대기
	TCCR1B=0;		// 카운터 정지
	cnt=TCNT1/116;		// cm로 변경
	_delay_ms(50);		// 딜레이
}

// 05. LED도트매트릭스

void Back(void)

{
	ledmatrix7219d88_init();	// LED_도트매트릭스 초기 설정
	uint8_t ledmatrix = 0;		// LED_도트매트릭스 초기화

	//display test rows // 특정 키가 입력되어 있을시의 LED 출력
	uint8_t rows[8] = {
		0b00000000,
		0b00011000,
		0b00111100,
		0b01111110,
		0b00011000,
		0b00011000,
		0b00011000,
		0b00000000
	};
	ledmatrix = 0;
	ledmatrix7219d88_setmatrix(0, rows);

	//display test rows // 아무키도 입력되지 않을시 혹은 키가 입력될 시의 LED 출력 (모든 상태에서의 LED 출력)
	ledmatrix = 0;
	ledmatrix7219d88_setrow(ledmatrix, 0, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 1, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 2, 0b00111100);
	ledmatrix7219d88_setrow(ledmatrix, 3, 0b01111110);
	ledmatrix7219d88_setrow(ledmatrix, 4, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 5, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 6, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 7, 0b00000000);

}

void Go(void)

{
	ledmatrix7219d88_init();	// LED_도트매트릭스 초기 설정
	uint8_t ledmatrix = 0;		// LED_도트매트릭스 초기화

	//display test rows // 특정 키가 입력되어 있을시의 LED 출력
	uint8_t rows[8] = {
		0b00000000,
		0b00011000,
		0b00011000,
		0b00011000,
		0b01111110,
		0b00111100,
		0b00011000,
		0b00000000
	};
	ledmatrix = 0;
	ledmatrix7219d88_setmatrix(0, rows);

	//display test rows // 아무키도 입력되지 않을시 혹은 키가 입력될 시의 LED 출력 (모든 상태에서의 LED 출력)
	ledmatrix = 0;
	ledmatrix7219d88_setrow(ledmatrix, 0, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 1, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 2, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 3, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 4, 0b01111110);
	ledmatrix7219d88_setrow(ledmatrix, 5, 0b00111100);
	ledmatrix7219d88_setrow(ledmatrix, 6, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 7, 0b00000000);

}

void Left(void)

{
	ledmatrix7219d88_init();	// LED_도트매트릭스 초기 설정
	uint8_t ledmatrix = 0;		// LED_도트매트릭스 초기화

	//display test rows // 특정 키가 입력되어 있을시의 LED 출력
	uint8_t rows[8] = {
		0b00000000,
		0b00001000,
		0b00001100,
		0b01111110,
		0b01111110,
		0b00001100,
		0b00001000,
		0b00000000
	};
	ledmatrix = 0;
	ledmatrix7219d88_setmatrix(0, rows);

	//display test rows // 아무키도 입력되지 않을시 혹은 키가 입력될 시의 LED 출력 (모든 상태에서의 LED 출력)
	ledmatrix = 0;
	ledmatrix7219d88_setrow(ledmatrix, 0, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 1, 0b00001000);
	ledmatrix7219d88_setrow(ledmatrix, 2, 0b00001100);
	ledmatrix7219d88_setrow(ledmatrix, 3, 0b01111110);
	ledmatrix7219d88_setrow(ledmatrix, 4, 0b01111110);
	ledmatrix7219d88_setrow(ledmatrix, 5, 0b00001100);
	ledmatrix7219d88_setrow(ledmatrix, 6, 0b00001000);
	ledmatrix7219d88_setrow(ledmatrix, 7, 0b00000000);

}

void Right(void)

{
	ledmatrix7219d88_init();	// LED_도트매트릭스 초기 설정
	uint8_t ledmatrix = 0;		// LED_도트매트릭스 초기화

	//display test rows // 특정 키가 입력되어 있을시의 LED 출력
	uint8_t rows[8] = {
		0b00000000,
		0b00010000,
		0b00110000,
		0b01111110,
		0b01111110,
		0b00110000,
		0b00010000,
		0b00000000
	};
	ledmatrix = 0;
	ledmatrix7219d88_setmatrix(0, rows);

	//display test rows // 아무키도 입력되지 않을시 혹은 키가 입력될 시의 LED 출력 (모든 상태에서의 LED 출력)
	ledmatrix = 0;
	ledmatrix7219d88_setrow(ledmatrix, 0, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 1, 0b00010000);
	ledmatrix7219d88_setrow(ledmatrix, 2, 0b00110000);
	ledmatrix7219d88_setrow(ledmatrix, 3, 0b01111110);
	ledmatrix7219d88_setrow(ledmatrix, 4, 0b01111110);
	ledmatrix7219d88_setrow(ledmatrix, 5, 0b00110000);
	ledmatrix7219d88_setrow(ledmatrix, 6, 0b00010000);
	ledmatrix7219d88_setrow(ledmatrix, 7, 0b00000000);

}

void Straight(void)

{
	ledmatrix7219d88_init();	// LED_도트매트릭스 초기 설정
	uint8_t ledmatrix = 0;		// LED_도트매트릭스 초기화

	//display test rows // 특정 키가 입력되어 있을시의 LED 출력
	uint8_t rows[8] = {
		0b00000000,
		0b00100100,
		0b00100100,
		0b00000000,
		0b01000010,
		0b01011010,
		0b00100100,
		0b00000000
	};
	ledmatrix = 0;
	ledmatrix7219d88_setmatrix(0, rows);

	//display test rows // 아무키도 입력되지 않을시 혹은 키가 입력될 시의 LED 출력 (모든 상태에서의 LED 출력)
	ledmatrix = 0;
	ledmatrix7219d88_setrow(ledmatrix, 0, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 1, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 2, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 3, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 4, 0b01000010);
	ledmatrix7219d88_setrow(ledmatrix, 5, 0b01011010);
	ledmatrix7219d88_setrow(ledmatrix, 6, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 7, 0b00000000);

}

void Smile(void)

{
	ledmatrix7219d88_init();	// LED_도트매트릭스 초기 설정
	uint8_t ledmatrix = 0;		// LED_도트매트릭스 초기화

	//display test rows // 특정 키가 입력되어 있을시의 LED 출력
	uint8_t rows[8] = {
		0b00000000,
		0b00100100,
		0b00100100,
		0b00000000,
		0b01000010,
		0b01000010,
		0b00111100,
		0b00000000
	};
	ledmatrix = 0;
	ledmatrix7219d88_setmatrix(0, rows);

	//display test rows // 아무키도 입력되지 않을시 혹은 키가 입력될 시의 LED 출력 (모든 상태에서의 LED 출력)
	ledmatrix = 0;
	ledmatrix7219d88_setrow(ledmatrix, 0, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 1, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 2, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 3, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 4, 0b01000010);
	ledmatrix7219d88_setrow(ledmatrix, 5, 0b01000010);
	ledmatrix7219d88_setrow(ledmatrix, 6, 0b00111100);
	ledmatrix7219d88_setrow(ledmatrix, 7, 0b00000000);

}

void Angry (void)

{
	ledmatrix7219d88_init();	// LED_도트매트릭스 초기 설정
	uint8_t ledmatrix = 0;		// LED_도트매트릭스 초기화

	//display test rows // 특정 키가 입력되어 있을시의 LED 출력
	uint8_t rows[8] = {
		0b00000000,
		0b00100100,
		0b00100100,
		0b00000000,
		0b00111100,
		0b01000010,
		0b01000010,
		0b00000000
	};
	ledmatrix = 0;
	ledmatrix7219d88_setmatrix(0, rows);

	//display test rows // 아무키도 입력되지 않을시 혹은 키가 입력될 시의 LED 출력 (모든 상태에서의 LED 출력)
	ledmatrix = 0;
	ledmatrix7219d88_setrow(ledmatrix, 0, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 1, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 2, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 3, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 4, 0b00111100);
	ledmatrix7219d88_setrow(ledmatrix, 5, 0b01000010);
	ledmatrix7219d88_setrow(ledmatrix, 6, 0b01000010);
	ledmatrix7219d88_setrow(ledmatrix, 7, 0b00000000);

}

void watchout(void)

{
	ledmatrix7219d88_init();	// LED_도트매트릭스 초기 설정
	uint8_t ledmatrix = 0;		// LED_도트매트릭스 초기화

	//display test rows // 특정 키가 입력되어 있을시의 LED 출력
	uint8_t rows[8] = {
		0b00000000,
		0b00100100,
		0b00100100,
		0b00000000,
		0b00011000,
		0b00111100,
		0b00111100,
		0b00011000
	};
	ledmatrix = 0;
	ledmatrix7219d88_setmatrix(0, rows);

	//display test rows // 아무키도 입력되지 않을시 혹은 키가 입력될 시의 LED 출력 (모든 상태에서의 LED 출력)
	ledmatrix = 0;
	ledmatrix7219d88_setrow(ledmatrix, 0, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 1, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 2, 0b00100100);
	ledmatrix7219d88_setrow(ledmatrix, 3, 0b00000000);
	ledmatrix7219d88_setrow(ledmatrix, 4, 0b00011000);
	ledmatrix7219d88_setrow(ledmatrix, 5, 0b00111100);
	ledmatrix7219d88_setrow(ledmatrix, 6, 0b00111100);
	ledmatrix7219d88_setrow(ledmatrix, 7, 0b00011000);

}

// 06. 부저

void Caution_Rapidly1(void)

{
	PORTC=0b10000000;
	_delay_ms(500);
	PORTC=0x00;
	_delay_ms(500);
}

void Caution_Rapidly2(void)

{
	PORTC=0b10000000;
	_delay_ms(250);
	PORTC=0x00;
	_delay_ms(250);
}

void Caution_Rapidly3(void)

{
	PORTC=0b10000000;
	_delay_ms(100);
	PORTC=0x00;
	_delay_ms(100);
}

void Caution_Emergency(void)

{
	PORTC=0b10000000;
}

// 07. 라이트

void Turn_the_light_on(void)

{
	PORTA=0b00000011;
}

void Turn_the_light_off(void)

{
	PORTA=0x00;
}

// #### 메인 함수

int main(void)

{
	/* 하위 기본 스타팅 셋업 코드 */
	
	DDRC=0xff;					// 부저 포트를 출력 포트로 설정
	DDRB=1;						// 초음파 거리 감지 센서포트 설정
	DDRE |= (1 << PE3);			// 서보모터 드라이버 포트를 출력 포트로 설정
	DDRF=0xff;					// 포트F(DC모터) 출력 포트로 설정
	DDRA=0xff;					// 포트A(방향등, 라이트) 출력 포트로 설정
	init();						// 블루투스 초기 설정
	InitioalizeTimer();			// 서브모터 초기 설정
	OCR3A = PULSE_MID;			// 서브모터 초기 바퀴 각 설정(바퀴방향=0도)

	/* 하위 로직 반복 코드 */

	while(1) {

	if(flag=='G'){					// 블루투스 통신이 가즈아 상태라면

		Go();
		DC_Forward();
	}

	else if(flag=='B'){				// 블루투스 통신이 뒤로 가즈아 상태라면

		Back();
		DC_Backward();
	}

	else if(flag=='L'){ 			// 블루투스 통신이 좌로 가즈아 상태라면
	
		Left();
		PULSE_MID = PULSE_MID+100;
		OCR3A = PULSE_MID;
		_delay_ms(10);

		if(PULSE_MID>=4100)		// 우회전시 바퀴 파손 방지
		{
		OCR3A = PULSE_MID = 4100;	
		}
		
	}
	
	else if(flag=='R'){				// 블루투스 통신이 우로 가즈아 상태라면
	
		Right();
		PULSE_MID = PULSE_MID-100;
		OCR3A = PULSE_MID;
		_delay_ms(10);

		if(PULSE_MID<=1900)		// 좌회전시 바퀴 파손방지
		{
			OCR3A = PULSE_MID = 1900;
		}
		
	}

	else if(flag=='S'){				// 블루투스 통신이 직진으로 가즈아 상태라면
		
		Straight();
		PULSE_MID = 3000;
		OCR3A = PULSE_MID;		
		_delay_ms(10);
		
	}

	else if(flag=='I'){				// 블루투스 통신이 안보인다 상태라면
		
		Turn_the_light_on();
		
	}
	
	else if(flag=='P'){				// 블루투스 통신이 불좀꺼라 상태라면
		
		Turn_the_light_off();
		
	}

	else if(flag=='H'){				// 블루투스 통신이 떡상(환호) 상태라면

		Angry();
		PORTC = 0b10000000;

	}
	
	else {							// 블루투스 통신이 존버 상태라면 ['C' 신호 수신시]

		Smile();
		DC_Stop();
		srf04();
		
			if(cnt<4){
				
				watchout();
				Caution_Emergency();
		
			}
	
			else if(cnt<6){
		
				watchout();
				Caution_Rapidly3();
		
			}

			else if(cnt<8){

				watchout();		
				Caution_Rapidly2();
		
			}
	
			else if(cnt<10){

				watchout();		
				Caution_Rapidly1();
		
			}
		
			else	{
			
				PORTC=0x00;
			}

		
		}

	}
}
