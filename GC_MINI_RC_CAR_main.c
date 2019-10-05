#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "ledmatrix7219d88.h"

// * �ʵ� : AVR�� �ҽ��ڵ带 ���ε� �� ���� �ݵ�� HC-SR04[������ �Ÿ� ��������]�� Ʈ���ſ� ���� ���ڸ� ���� ����� OFF ���Ѿ� ���ε��� ������.
// RC-CAR �ҽ�: �ڵ� ����[����] (������� ��� : Name/Pass = RC/5683)
// �ڵ� ����[����] [�����ڵ�/����/�Լ����� �� ��Ÿ�ڵ�/���� ��]

/*
	01. ������� �ۼ��� ����	���� ��Ʈ UART0 ��Ʈ (E��Ʈ)
	02. DC���� ����̹� ����	���� ��Ʈ PORTF,PORTA (DC���� ����̹� ���� = 2���� DC ���� ����̹� �������� : �������͸��� VCC, AVR�� VCC ���
																										  �� �������͸��� ��������)
	03. �������� ����   ����	���� ��Ʈ PORTE
	04. ������ �������� ����	���� ��Ʈ PORTB
	05. LED��Ʈ��Ʈ���� ����	���� ��Ʈ PORTD
	06. ����           ����	���� ��Ʈ PORTC
	07. ����Ʈ		   ���� ���� ��Ʈ PORTA
												*/

// ##### �����ڵ�

// 01. ������� - ����

// 02. DC���� ����̹� - ����

// 03. ��������

	#define ROTATION_DELAY	4000

// 04. ������ ��������

	#define trig_1 PORTB|=1  // PB0, srf04 Ʈ���� (�۽�)
	#define trig_0 PORTB&=~1 // PB0, srf04 Ʈ���� (�۽�)
	#define echo   (PINB&2)  // PB1, srf04 ���� (����)

// 05. LED��Ʈ��Ʈ���� - ���� ÷��

// 06. ���� - ����

// 07. ����Ʈ -����


// ##### ����

// 01. �������

	volatile unsigned char flag;

// 02. DC���� ����̹� - ����

// 03. ��������

	int PULSE_MID = 3000; // 3000 = 0�� <����> // �������� ���� 90�� (1300)) , �������� 90�� 4700)

// 04. ������ ��������

	unsigned int cnt;

// 05. LED��Ʈ��Ʈ���� - ���� ÷��

// 06. ���� - ����

// 07. ����Ʈ - ����


// ##### �Լ����� �� ��Ÿ�ڵ�

// 01. �������

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

// 02. DC���� ����̹�

	void DC_Forward(void)

	{
		PORTF=0b10101010; // ����
	}

	void DC_Backward(void)

	{
		PORTF=0b01010101; // ����
	}

	void DC_Stop(void)
	{
		PORTF=0x00;	  // ����
	}

// 03. ��������
	
	void InitioalizeTimer(void)
	
	{
		// ��� 14, ��� PWM ���
		TCCR3A |= (1 << WGM11);
		TCCR3B |= (1 <<WGM12) | (1 << WGM13);
	
		// ����� ���
		// TOP:ICR1, �� ��ġ��:OCR3A ��������
		TCCR3A |= (1 << COM1A1);
	
		TCCR3B |= (1 << CS11);	// ������ 8, 2MHz
		
		ICR3 = 39999;	//  20ms �ֱ�
	}

// 04. ������ ��������

	void srf04(){

	trig_1; //Ʈ����
	_delay_us(10);
	trig_0; //Ʈ����

	while(!echo);		// ��ȣ�� high�� �� ������ ���
	TCNT1=0; TCCR1B=2;	// ī���� ����, 8���� 0.5us
	while(echo);		// ��ȣ�� low�� �� ������ ���
	TCCR1B=0;		// ī���� ����
	cnt=TCNT1/116;		// cm�� ����
	_delay_ms(50);		// ������
}

// 05. LED��Ʈ��Ʈ����

void Back(void)

{
	ledmatrix7219d88_init();	// LED_��Ʈ��Ʈ���� �ʱ� ����
	uint8_t ledmatrix = 0;		// LED_��Ʈ��Ʈ���� �ʱ�ȭ

	//display test rows // Ư�� Ű�� �ԷµǾ� �������� LED ���
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

	//display test rows // �ƹ�Ű�� �Էµ��� ������ Ȥ�� Ű�� �Էµ� ���� LED ��� (��� ���¿����� LED ���)
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
	ledmatrix7219d88_init();	// LED_��Ʈ��Ʈ���� �ʱ� ����
	uint8_t ledmatrix = 0;		// LED_��Ʈ��Ʈ���� �ʱ�ȭ

	//display test rows // Ư�� Ű�� �ԷµǾ� �������� LED ���
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

	//display test rows // �ƹ�Ű�� �Էµ��� ������ Ȥ�� Ű�� �Էµ� ���� LED ��� (��� ���¿����� LED ���)
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
	ledmatrix7219d88_init();	// LED_��Ʈ��Ʈ���� �ʱ� ����
	uint8_t ledmatrix = 0;		// LED_��Ʈ��Ʈ���� �ʱ�ȭ

	//display test rows // Ư�� Ű�� �ԷµǾ� �������� LED ���
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

	//display test rows // �ƹ�Ű�� �Էµ��� ������ Ȥ�� Ű�� �Էµ� ���� LED ��� (��� ���¿����� LED ���)
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
	ledmatrix7219d88_init();	// LED_��Ʈ��Ʈ���� �ʱ� ����
	uint8_t ledmatrix = 0;		// LED_��Ʈ��Ʈ���� �ʱ�ȭ

	//display test rows // Ư�� Ű�� �ԷµǾ� �������� LED ���
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

	//display test rows // �ƹ�Ű�� �Էµ��� ������ Ȥ�� Ű�� �Էµ� ���� LED ��� (��� ���¿����� LED ���)
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
	ledmatrix7219d88_init();	// LED_��Ʈ��Ʈ���� �ʱ� ����
	uint8_t ledmatrix = 0;		// LED_��Ʈ��Ʈ���� �ʱ�ȭ

	//display test rows // Ư�� Ű�� �ԷµǾ� �������� LED ���
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

	//display test rows // �ƹ�Ű�� �Էµ��� ������ Ȥ�� Ű�� �Էµ� ���� LED ��� (��� ���¿����� LED ���)
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
	ledmatrix7219d88_init();	// LED_��Ʈ��Ʈ���� �ʱ� ����
	uint8_t ledmatrix = 0;		// LED_��Ʈ��Ʈ���� �ʱ�ȭ

	//display test rows // Ư�� Ű�� �ԷµǾ� �������� LED ���
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

	//display test rows // �ƹ�Ű�� �Էµ��� ������ Ȥ�� Ű�� �Էµ� ���� LED ��� (��� ���¿����� LED ���)
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
	ledmatrix7219d88_init();	// LED_��Ʈ��Ʈ���� �ʱ� ����
	uint8_t ledmatrix = 0;		// LED_��Ʈ��Ʈ���� �ʱ�ȭ

	//display test rows // Ư�� Ű�� �ԷµǾ� �������� LED ���
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

	//display test rows // �ƹ�Ű�� �Էµ��� ������ Ȥ�� Ű�� �Էµ� ���� LED ��� (��� ���¿����� LED ���)
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
	ledmatrix7219d88_init();	// LED_��Ʈ��Ʈ���� �ʱ� ����
	uint8_t ledmatrix = 0;		// LED_��Ʈ��Ʈ���� �ʱ�ȭ

	//display test rows // Ư�� Ű�� �ԷµǾ� �������� LED ���
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

	//display test rows // �ƹ�Ű�� �Էµ��� ������ Ȥ�� Ű�� �Էµ� ���� LED ��� (��� ���¿����� LED ���)
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

// 06. ����

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

// 07. ����Ʈ

void Turn_the_light_on(void)

{
	PORTA=0b00000011;
}

void Turn_the_light_off(void)

{
	PORTA=0x00;
}

// #### ���� �Լ�

int main(void)

{
	/* ���� �⺻ ��Ÿ�� �¾� �ڵ� */
	
	DDRC=0xff;					// ���� ��Ʈ�� ��� ��Ʈ�� ����
	DDRB=1;						// ������ �Ÿ� ���� ������Ʈ ����
	DDRE |= (1 << PE3);			// �������� ����̹� ��Ʈ�� ��� ��Ʈ�� ����
	DDRF=0xff;					// ��ƮF(DC����) ��� ��Ʈ�� ����
	DDRA=0xff;					// ��ƮA(�����, ����Ʈ) ��� ��Ʈ�� ����
	init();						// ������� �ʱ� ����
	InitioalizeTimer();			// ������� �ʱ� ����
	OCR3A = PULSE_MID;			// ������� �ʱ� ���� �� ����(��������=0��)

	/* ���� ���� �ݺ� �ڵ� */

	while(1) {

	if(flag=='G'){					// ������� ����� ����� ���¶��

		Go();
		DC_Forward();
	}

	else if(flag=='B'){				// ������� ����� �ڷ� ����� ���¶��

		Back();
		DC_Backward();
	}

	else if(flag=='L'){ 			// ������� ����� �·� ����� ���¶��
	
		Left();
		PULSE_MID = PULSE_MID+100;
		OCR3A = PULSE_MID;
		_delay_ms(10);

		if(PULSE_MID>=4100)		// ��ȸ���� ���� �ļ� ����
		{
		OCR3A = PULSE_MID = 4100;	
		}
		
	}
	
	else if(flag=='R'){				// ������� ����� ��� ����� ���¶��
	
		Right();
		PULSE_MID = PULSE_MID-100;
		OCR3A = PULSE_MID;
		_delay_ms(10);

		if(PULSE_MID<=1900)		// ��ȸ���� ���� �ļչ���
		{
			OCR3A = PULSE_MID = 1900;
		}
		
	}

	else if(flag=='S'){				// ������� ����� �������� ����� ���¶��
		
		Straight();
		PULSE_MID = 3000;
		OCR3A = PULSE_MID;		
		_delay_ms(10);
		
	}

	else if(flag=='I'){				// ������� ����� �Ⱥ��δ� ���¶��
		
		Turn_the_light_on();
		
	}
	
	else if(flag=='P'){				// ������� ����� �������� ���¶��
		
		Turn_the_light_off();
		
	}

	else if(flag=='H'){				// ������� ����� ����(ȯȣ) ���¶��

		Angry();
		PORTC = 0b10000000;

	}
	
	else {							// ������� ����� ���� ���¶�� ['C' ��ȣ ���Ž�]

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
