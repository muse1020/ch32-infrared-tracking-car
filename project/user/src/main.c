

#include "zf_common_headfile.h"
#include "infrared_tracking.h"


#define IR_SAMPLE_PERIOD_MS         5           // 采样周期 
#define IR_DETAIL_PRINT_INTERVAL    200          // 每50次采样打印一次详细报告

//-------------------------------------------------------------------------------------------------------------------
// 电机 PWM 参数宏定义
//-------------------------------------------------------------------------------------------------------------------
#define MOTOR_PWM_FREQ              5000   // PWM 频率 1kHz
#define MOTOR_PWM_DUTY_MAX          PWM_DUTY_MAX // 占空比最大值 10000 
#define MOTOR_SPEED_STOP            0           // 停止
#define IR_CH1_PIN                  C6
#define IR_CH2_PIN                  C8
#define IR_CH3_PIN                  A8
#define IR_CH4_PIN                  A10
#define IR_CH5_PIN                  A12
#define BASE_SPEED    (3000)   
#define TURN_KP       (60)    // 转弯修正系数


int main (void)
{

    clock_init(SYSTEM_CLOCK_120M);              // 初始化芯片时钟, 配置系统频率为 120MHz
    debug_init();                               // 初始化调试串口 (UART3, 115200, B10/B11)

   pwm_init(TIM5_PWM_MAP0_CH2_A1, MOTOR_PWM_FREQ, MOTOR_SPEED_STOP);
    pwm_init(TIM5_PWM_MAP0_CH4_A3, MOTOR_PWM_FREQ, MOTOR_SPEED_STOP);

    // 左电机方向引脚: IN1=A6, IN2=A7
    gpio_init(A5, GPO, 0, GPO_PUSH_PULL);       // IN1 初始低电平
    gpio_init(A7, GPO, 0, GPO_PUSH_PULL);       // IN2 初始低电平

    // 右电机方向引脚: IN3=B0, IN4=B1
    gpio_init(C5, GPO, 0, GPO_PUSH_PULL);       // IN3 初始低电平
    gpio_init(B1, GPO, 0, GPO_PUSH_PULL);       // IN4 初始低电平

    gpio_set_level(A5, 0);                      // IN1=1  左电机正转
    gpio_set_level(A7, 1);                      // IN2=0
    gpio_set_level(C5, 0);                      // IN3=1  右电机正转
    gpio_set_level(B1, 1);                      // IN4=0


    gpio_init(IR_CH1_PIN, GPI, 0, GPI_PULL_UP);
    gpio_init(IR_CH2_PIN, GPI, 0, GPI_PULL_UP);
    gpio_init(IR_CH3_PIN, GPI, 0, GPI_PULL_UP);
    gpio_init(IR_CH4_PIN, GPI, 0, GPI_PULL_UP);
    gpio_init(IR_CH5_PIN, GPI, 0, GPI_PULL_UP);

    uint8 ir_state[5]; 
    int32 error = 0;
    int32 last_error = 0;   

    while(1)
    {
     ir_state[0] = gpio_get_level(IR_CH1_PIN);
     ir_state[1] = gpio_get_level(IR_CH2_PIN);
     ir_state[2] = gpio_get_level(IR_CH3_PIN);
     ir_state[3] = gpio_get_level(IR_CH4_PIN);
     ir_state[4] = gpio_get_level(IR_CH5_PIN);
    if (!ir_state[2])
        error = 0;          // 正中  00100
    else if (!ir_state[1])
        error = -4;         // 偏左  01000
    else if (!ir_state[3])
        error = 4;          // 偏右  00010
    else if (!ir_state[0])
        error = -4;         // 大偏左 10000
    else if (!ir_state[4])
        error = 4;          // 大偏右 00001
    else
        error = last_error; // 全白丢线 / 全黑十字路口，保持上次方向

    last_error = error;
    // 计算左右电机速度
    int32 turn_speed = error * TURN_KP;
    int32 left_speed  = BASE_SPEED + turn_speed;
    int32 right_speed = BASE_SPEED - turn_speed;
   

            pwm_set_duty(TIM5_PWM_MAP0_CH2_A1, left_speed);//左
            pwm_set_duty(TIM5_PWM_MAP0_CH4_A3, right_speed+160);
        

 
    }
}
