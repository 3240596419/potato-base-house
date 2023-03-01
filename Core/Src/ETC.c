//
// Created by potaoman on 2023/2/19.
//
#include "main.h"
#include "usart.h"
#include "ETC.h"
#include "tim.h"

uint8_t Receive_data[33]={0};
uint16_t PWM[4]={2500,2500,2500,2500};
float pitch,roll,yaw; 		    //欧拉角
short aacx,aacy,aacz;			//加速度传感器原始数据
short gyrox,gyroy,gyroz;		//陀螺仪原始数据
float temp;
//
//tips:Transmit u8 for float.
void Transmit_Data(float m)
{
    uint16_t pp,t=1,l=1;
    int p;
    uint8_t x=' ',y='-',xx='.',yy='^',a='C',aa='0',i,ii;
    p=(int)(m*100);
    if(p<0){
        pp=(uint16_t)(-p);
        HAL_UART_Transmit(&huart1,&y,1,0xffff);//print'-'
    } else{
        pp=(uint16_t)(p);
    }
    if(pp<100){
        HAL_UART_Transmit(&huart1,&aa,1,0xffff);
        HAL_UART_Transmit(&huart1,&xx,1,0xffff);
    }
    while(t){
        if((pp/l>=l||pp>=10*l)&&pp>10){
            l=l*10;
            continue;
        } else{
            i=pp/l;
            ii=i+48;
            HAL_UART_Transmit(&huart1,&ii,1,0xffff);
            if(l==100){
                HAL_UART_Transmit(&huart1,&xx,1,0xffff);
            }
            pp=pp-l*i;
            l=l/10;
            if(l==0)t=pp;
        }
    }
    HAL_UART_Transmit(&huart1,&yy,1,0xffff);
    HAL_UART_Transmit(&huart1,&a,1,0xffff);
    HAL_UART_Transmit(&huart1,&x,1,0xffff);
    HAL_UART_Transmit(&huart1,&x,1,0xffff);
}
void Show_uloar(float pitch,float roll,float yaw){
    HAL_UART_Transmit(&huart1,(uint8_t *)"\tpitch: ",7,0xffff);
    Transmit_Data(pitch);
    HAL_UART_Transmit(&huart1,(uint8_t *)"roll: ",6,0xffff);
    Transmit_Data(roll);
    HAL_UART_Transmit(&huart1,(uint8_t *)"yaw: ",5,0xffff);
    Transmit_Data(yaw);
    HAL_UART_Transmit(&huart1,(uint8_t *)"\n",1,0xffff);
}
void R_T_text(){
    Receive();
    LED();
    M_text(Receive_data);
}
void LED(){
    if(Receive_data[0]==1){
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
    }else if(Receive_data[0]==2){
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
    } else if(Receive_data[0]==3){
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
    } else{
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
    }
}
void Receive(){
    while (NRF24L01_RxPacket(Receive_data)){}
    Receive_data[32]=0;
}
void M_text(uint8_t * a){
    switch (a[0]) {
        case 1:
            PWM[0]-=10;
            PWM[1]-=10;
            break;
        case 2:
            PWM[2]+=10;
            break;
        case 3:
            PWM[3]-=10;
            break;
        case 4:
            PWM[0]=2500;
            PWM[1]=2500;
            PWM[2]=2500;
            PWM[3]=2500;
            break;
        default:
            //error
            break;
    }
    __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_1,PWM[0]);
    __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_2,PWM[1]);
    __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_3,PWM[2]);
    __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_4,PWM[3]);
//1:-,2:-,3:+,4:
}
void System_start(){
    MPU_Init();			//MPU6050初始化
    mpu_dmp_init();		//dmp初始化
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_4);
    while(mpu_dmp_init()){}
    while(NRF24L01_Check()){}
    NRF24L01_RX_Mode();
}
//      if(PWM<5000){
//          __HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_1,PWM);
//          PWM+=100;
//      }else{
//          PWM=2500;
//      }
//      while(mpu_dmp_get_data(&pitch, &roll, &yaw));	//必须要用while等待，才能读取成功
//      Show_uloar(pitch,roll,yaw);
//      HAL_Delay(10);