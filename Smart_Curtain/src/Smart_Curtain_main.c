#include <mega128.h>
#include "lcd.h"
#include "Term.h"
#include <stdio.h>
#define Buzz  195
#define warning 300
#define Up_max 1
#define Down_max 2
#define Switch 3
#define Bright_check 4
#define Mode_Sel 5
unsigned int average[10] = {0,};   //10칸을 0으로 초기화 
unsigned int Temp_sort = 0; 
int index_bri = 0;
int i,j;
unsigned char Erase[] = "                ";
int Mode_flag = 0;
int bright_count = 0;
short Bright_val;
int Count = 0;
int B_on = 0;
int Mo_count = 0;
int Mo_exit = 0;
char Time[] = {0,0,0,0,0,0};
int Mo_time_count = 0;
int reservation_mode = 0;
int wrong_flag = 0;
int index = 0;
int Treshold = 170;
int Treshold_count = 0;
int no_interrupt_flag = 0;

short Get_ADC(int Number)//ADC1 사용 
{
    ADMUX = Number;
    ADMUX |= (1<<REFS0)|(1<<REFS1);
    ADCSRA |= (1<<ADEN)|(1<<ADSC)|(1<<ADFR)|(1<<ADPS2)|(1<<ADPS1);
    delay_ms(5);
    while(!(ADCSRA&(1<<ADIF)));
    return ADCW;
}

interrupt [TIM0_COMP] void timer0_out__comp(void)   //타이머 CTC 비교일치시 수행되는 함수 정의 
{
    bright_count++;                                                                                
    Count++;
    if(bright_count > 499)
    {
        if(B_on == 1)
        {
            average[index_bri]  = Get_ADC(Bright_check); //조도는 시도때도없이 확인할 필요가 없기때문에 일정시간동안만 하는것을 구현 
            index_bri++;
            if(index_bri >= 9)
            {
                index_bri = 0;
                for(i = 0; i < 10 ; i++)//10개의 입력받은 데이터 값을 내림차순으로 
                {                       //정렬하는 알고리즘 입니다. 
                    for(j = i+1; j < 10 ; j++)
                    {
                        if(average[i]>average[j])
                        {
                            Temp_sort = average[i];
                            average[i] = average[j];
                            average[j] = Temp_sort;
                            j = i+1;    
                        }
                    }
                }
                Bright_val =  average[4];
                
            }
        }
        bright_count = 0;
    }    
}


interrupt [EXT_INT4] void ext_int4_isr(void)
{
    delay_ms(100);
    if(no_interrupt_flag == 1)
    {
    
    }
    else if(reservation_mode == 1)
    {
        reservation_mode = 0;
        delay_ms(200);
    }
    else if(Mode_flag !=0)
    {
        while(1)
        {
            if(PINE.4 == 1)
            {
                Mo_count = Mode_flag;
                Mode_flag = 0;           
                delay_ms(100);
                break;
            }
        }
    }
    else
    {
        Mo_exit = 1;    
    }       
}
void Auto_mode(void)
{
    LCD_Pos(1,0);
    LCD_Str(Erase);
    B_on = 1;
    Bright_val = Get_ADC(Bright_check);// 초기 한번 측정 후 반응  
    while(1)
    { //Auto_mode
        if(Bright_val < Treshold)
        {
            if(Treshold_count == 0)
            {
                Treshold_count = 1 ;
                Treshold = Treshold + 100;
            }
            if(Get_ADC(Down_max) < 600){
                motor_down();
                LCD_Pos(1,0);
                LCD_Str(Erase);
                LCD_Pos(1,0);
                LCD_Str("Curtain Down");
                Count = 0;
                while(Count<125)
                {SSound(Buzz);}
                delay_ms(20);
                motor_stop();
            }
            else
            {
                LCD_Pos(1,0);
                LCD_Str(Erase);
                LCD_Pos(1,5);
                LCD_Str("Down_Max");
                //Count = 0;
                //while(Count<125)
                //{SSound(warning);}
                delay_ms(200);
            }
        }
        else
        {   
            if(Treshold_count == 1)
            {
                Treshold_count = 0 ;
                if(Treshold > 100)
                {Treshold = Treshold - 100;}
            }
            if(Get_ADC(Up_max) > 600){
                motor_up();
                LCD_Pos(1,0);
                LCD_Str(Erase);
                LCD_Pos(1,0);
                LCD_Str("Curtain up ");
                Count = 0;                                            
                while(Count<125)
                {SSound(Buzz);}
                delay_ms(20);
                motor_stop();
            }       
            else
            {
                LCD_Pos(1,0);
                LCD_Str(Erase);
                LCD_Pos(1,5);
                LCD_Str("Up_Max");
                //Count = 0;
                //while(Count<125)
                //{SSound(warning);}
                delay_ms(200);
            } 
        
        } 
        if(Mode_flag != 1) //Auto Mode 가 아닐시 함수탈출 
        {
            LCD_Clear();
            Treshold = 170; 
            break;
        }
     }   
}

void Manual_mode(void)
{
     int Switch_val = 0;
     LCD_Pos(1,0);
     LCD_Str(Erase);
     B_on = 0; //조도 꺼주는 옵션 
     while(1)
     {
        Switch_val =  Get_ADC(Switch);
        //Manual_mode
        if(Mode_flag != 2)
        {
            LCD_Clear();
            break;
        }
        if(Switch_val <300)
        {
            if(Get_ADC(Up_max) > 600){
                motor_up();
                LCD_Pos(1,0);
                LCD_Str(Erase);
                LCD_Pos(1,0);
                LCD_Str("Curtain up ");
                Count = 0;                                            
                while(Count<125)
                {SSound(Buzz);}
                delay_ms(20);
                motor_stop();
            }       
            else
            {
                LCD_Pos(1,0);
                LCD_Str(Erase);
                LCD_Pos(1,5);
                LCD_Str("Up_Max");
                Count = 0;
                while(Count<125)
                {SSound(warning);}
                delay_ms(200);
            }
        }
        else if(Switch_val > 1000)
        {
            if(Get_ADC(Down_max) < 600){
                motor_down();
                LCD_Pos(1,0);
                LCD_Str(Erase);
                LCD_Pos(1,0);
                LCD_Str("Curtain Down");
                Count = 0;
                while(Count<125)
                {SSound(Buzz);}
                delay_ms(20);
                motor_stop();
            }
            else
            {
                LCD_Pos(1,0);
                LCD_Str(Erase);
                LCD_Pos(1,5);
                LCD_Str("Down_Max");
                Count = 0;
                while(Count<125)
                {SSound(warning);}
                delay_ms(200);
            }
        }
        else
        {
            LCD_Pos(1,0);
            LCD_Str(Erase);
            LCD_Pos(1,0);
            LCD_Str("Curtain NoMov");
            delay_ms(100);
        }   
     }
}

void Timer_mode(void)
{
    int hr,min,sec;
    int Switch_val = 0;
    char Message[20] = {0,};
    wrong_flag=0;
    LCD_Pos(1,0);
    LCD_Str(Erase);
    index = 0;
    B_on = 0;
    puts_USART1("\renter hour : ");
    while(1)
    {
        if(Mode_flag != 3)
        {
            LCD_Clear();
            wrong_flag = 1;
            break;
        }
        if(index > 5)
        {
            break;
        }
    }    
    puts_USART1("\n\r Time Setting Ok!\n\r");
    if(wrong_flag == 0)
    {
        hr = (Time[0]-48)*10 + (Time[1]-48);
        min = (Time[2]-48)*10 + (Time[3]-48);
        sec = (Time[4]-48)*10 + (Time[5]-48);
        LCD_Pos(0,1);
        LCD_Str(Erase);
        LCD_Pos(0,1);
        sprintf(Message,"%2dh %2dm %2ds",hr,min,sec);
        LCD_Str(Message);
            
        LCD_Pos(1,0);
        LCD_Str("select updown");
        no_interrupt_flag = 1;
        reservation_mode = 1;
        Mo_time_count = 3; //맵핑되지 않은 정수 대입 
        while(1)
        {
            Switch_val =  Get_ADC(Mode_Sel);
            if(Switch_val > 1000)
            {
                if(Mo_time_count == 0)
                {
                    Mo_time_count = 1;
                    LCD_Pos(1,0);
                    LCD_Str(Erase);
                    LCD_Pos(1,0);
                    LCD_Str("Up reservation");
                    no_interrupt_flag = 0;
                }
                else
                {
                    Mo_time_count = 0;
                    LCD_Pos(1,0);
                    LCD_Str(Erase);
                    LCD_Pos(1,0);
                    LCD_Str("Down reservation");
                    no_interrupt_flag = 0;
                }
                Count = 0;
                while(Count<125)
                {SSound(Buzz);}
                delay_ms(100);    
            }
            if(reservation_mode == 0)
            {
                delay_ms(500);
                break;
            }    
        }
        Count = 0;
        while(1)
        {
            if(Count >999)
            {
                Count = 0;
                if(sec == 0)
                {
                    if(min == 0)
                    {
                        if(hr == 0)
                        {
                            hr = 0;
                            min = 0;
                            sec = 0;
                        }
                        else
                        {
                            hr--;
                            min = 59;
                            sec = 59;
                        }
                    }
                    else
                    {
                        min--;
                        sec = 59;
                    }
                }
                else
                {
                    sec--;
                }
                LCD_Pos(0,1);
                sprintf(Message,"%2dh %2dm %2ds",hr,min,sec);
                LCD_Str(Message);
            }
            if(Mode_flag != 3)
            {
                LCD_Clear();
                break;
            }
            if(hr == 0 && min == 00 && sec == 0)
            {
                Mode_flag = 0;
                if(Mo_time_count == 1)
                {//up reservation
                    Count = 0;
                    while(Count<250)
                    {SSound(478);}
                    Count = 0;
                    while(Count<250)
                    {SSound(451);}
                    Count = 0;
                    while(Count<250)
                    {SSound(426);}
                    while(1)
                    {
                        if(Get_ADC(Up_max) > 600)
                        {
                            motor_up();
                            LCD_Pos(1,0);
                            LCD_Str(Erase);
                            LCD_Pos(1,0);
                            LCD_Str("Curtain up ");
                            Count = 0;
                            while(Count<125)
                            {SSound(Buzz);}
                            delay_ms(20);
                            motor_stop();
                        }
                        else
                        {
                            LCD_Pos(1,0);
                            LCD_Str(Erase);
                            LCD_Pos(1,5);
                            LCD_Str("Up_Max");
                            Count = 0;
                            while(Count<125)
                            {SSound(warning);}
                            delay_ms(200);
                            break;
                        }
                    }
                    if(Mode_flag != 3)
                    {
                        LCD_Clear();
                        break;
                    }
                }
                else
                {//down reservation
                    Count = 0;
                    while(Count<250)
                    {SSound(97.9988);}
                    Count = 0;
                    while(Count<250)
                    {SSound(82.407);}
                    Count = 0;
                    while(Count<250)
                    {SSound(65.4064);}
                    while(1)
                    {
                        if(Get_ADC(Down_max) < 600)
                        {
                            motor_down();
                            LCD_Pos(1,0);
                            LCD_Str(Erase);
                            LCD_Pos(1,0);
                            LCD_Str("Curtain Down");
                            Count = 0;
                            while(Count<125)
                            {SSound(Buzz);}
                            delay_ms(20);
                            motor_stop();
                        }
                        else
                        {
                            LCD_Pos(1,0);
                            LCD_Str(Erase);
                            LCD_Pos(1,5);
                            LCD_Str("Down_Max");
                            Count = 0;
                            while(Count<125)
                            {SSound(warning);}
                            delay_ms(200);
                            break;
                        }
                    }
                    if(Mode_flag != 3)
                    {
                        LCD_Clear();
                        break;
                    }
                }
            }
          
        }
    }    
    else
    {
        Mode_flag = 0;
    }
        
   
}

void Mode_select()
{
    int Switch_val = 0;
    LCD_Pos(1,0);
    LCD_Str(Erase);
    while(1)
    {
        Switch_val =  Get_ADC(Mode_Sel);
        if(Switch_val > 1000)
        {
            if(Mo_count >= 3)
            {
                Mo_count = 1;
            }
            else
            {
                Mo_count++;
            }
            Count = 0;
            while(Count<125)
            {SSound(Buzz);}
            delay_ms(100);      
        }
        else if(Switch_val < 100)
        {
            if(Mo_count <= 1)
            {
                Mo_count = 3;
            }
            else
            {
                Mo_count--;
            }
            Count = 0;
            while(Count<125)
            {SSound(Buzz);}
            delay_ms(100);
        }
        if(Mo_count == 0)
        {
            LCD_Pos(1,0);
            LCD_Str("Start Mode ");
        }
        else if(Mo_count == 1)
        {
            LCD_Pos(1,0);
            LCD_Str("Auto Mode  ");
        }
        else if(Mo_count == 2)
        {
            LCD_Pos(1,0);
            LCD_Str("Manual Mode");  
        }
        else if(Mo_count == 3)
        {
            LCD_Pos(1,0);
            LCD_Str("Timer Mode ");  
        }
        if(Mo_exit == 1)
        {
            Mode_flag = Mo_count;
            Mo_exit = 0;
            break;
        } 
    }    
}

interrupt [USART1_RXC] void usart1_receive(void)
{
    unsigned char str;
    str = UDR1; 
    if(str >= 0x30 && str <= 0x39)
    {                               
        if(index >5)
        {
            index = 6;   
        }
        else
        {
            if(index == 0 || index == 2 ||index == 4)
            {
                if(str >= 0x30 && str <= 0x35)
                {
                    Time[index] = str;
                    index++;
                }
                else
                {
                    puts_USART1("\r\nerror retry\n\r");
                    index = 0;
                    
                }    
            }
            else
            {
                Time[index] = str;
                index++;
            }
            if(index == 2 || index == 4)
            {
                putch_USART1(':');
            }
        }    
    }
}

interrupt [EXT_INT0] void ext_int0_isr(void)
{
    delay_ms(100);
    Treshold = Get_ADC(Bright_check);
    delay_ms(20);       
}

void main(void)
{   
    DDRB = 0xff;
    PORTB.0 = 0;
    PORTB.1 = 0;
    PORTB.7 = 0;
    DDRG|=(1<<4);
    initAD();
    LCD_Init();
    PinE_4_init();
    PinD_0_init();
    Timer2_Init();
    Timer0_Init();
    TCCR2 |= CS21;
    Init_USART1();
    LCD_High_line(Mode_flag);
    while(1)
    {
        if(Mode_flag == 1)
        {
            LCD_High_line(Mode_flag);
            Auto_mode();
        }
        else if(Mode_flag == 2)
        {
            LCD_High_line(Mode_flag);
            Manual_mode();    
        }
        else if(Mode_flag == 3)
        {
            LCD_High_line(Mode_flag);
            Timer_mode();    
        }
        else if(Mode_flag ==0)
        {
            LCD_High_line(Mode_flag);
            Mode_select();
        }
    }
}
                