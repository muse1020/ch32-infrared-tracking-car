#include "zf_common_headfile.h"
#include "infrared_tracking.h"

// ======================== 红外传感器引脚 ========================
// 传感器排列: CH1(最左) CH2(左) CH3(中) CH4(右) CH5(最右)
#define IR_CH1_PIN              C6
#define IR_CH2_PIN              C8
#define IR_CH3_PIN              A8
#define IR_CH4_PIN              A10
#define IR_CH5_PIN              A12

#define IR_LINE_DETECT          0           // 检测到黑线 (低电平)
#define IR_NO_LINE              1           // 未检测到 (高电平)

// ======================== 电机参数 ========================
#define MOTOR_PWM_FREQ          5000        // PWM 频率 5kHz

#define BASE_SPEED              3000        // 直行基础速度 (满量程 10000)
#define TURN_KP                 30          // 转弯比例系数

// 右电机补偿: 两个电机特性有差异，实测右电机需要 +160 才能和左电机对齐
#define RIGHT_MOTOR_COMP        160

// ======================== 引脚定义 ========================
// 左电机: PWM=PA1(TIM5_CH2), 方向=PA5(IN1)/PA7(IN2)
#define MOTOR_L_PWM             TIM5_PWM_MAP0_CH2_A1
#define MOTOR_L_IN1             A5
#define MOTOR_L_IN2             A7

// 右电机: PWM=PA3(TIM5_CH4), 方向=PC5(IN3)/PB1(IN4)
#define MOTOR_R_PWM             TIM5_PWM_MAP0_CH4_A3
#define MOTOR_R_IN3             C5
#define MOTOR_R_IN4             B1

// ======================== 传感器误差映射表 ========================
// CH1(最左)=-4  CH2(左)=-2  CH3(中)=0  CH4(右)=+2  CH5(最右)=+4
// 误差越大 → 转弯越急，体现偏离程度的梯度
static const int8 ERROR_MAP[5] = { -4, -2, 0, +2, +4 };

// ======================== 引脚表 ========================
static const gpio_pin_enum IR_PINS[5] = {
    IR_CH1_PIN, IR_CH2_PIN, IR_CH3_PIN, IR_CH4_PIN, IR_CH5_PIN
};

// ======================== 初始化函数 ========================
static void motor_init(void)
{
    pwm_init(MOTOR_L_PWM, MOTOR_PWM_FREQ, 0);
    pwm_init(MOTOR_R_PWM, MOTOR_PWM_FREQ, 0);

    gpio_init(MOTOR_L_IN1, GPO, 0, GPO_PUSH_PULL);
    gpio_init(MOTOR_L_IN2, GPO, 0, GPO_PUSH_PULL);
    gpio_init(MOTOR_R_IN3, GPO, 0, GPO_PUSH_PULL);
    gpio_init(MOTOR_R_IN4, GPO, 0, GPO_PUSH_PULL);

    // 左电机正转: IN1=低, IN2=高
    gpio_set_level(MOTOR_L_IN1, 0);
    gpio_set_level(MOTOR_L_IN2, 1);

    // 右电机正转: IN3=低, IN4=高
    gpio_set_level(MOTOR_R_IN3, 0);
    gpio_set_level(MOTOR_R_IN4, 1);
}

static void ir_sensor_init(void)
{
    for (int i = 0; i < 5; i++)
        gpio_init(IR_PINS[i], GPI, 0, GPI_PULL_UP);
}

// ======================== 寻迹核心 ========================
// 读取 5 路传感器，返回误差值
// 优先级: 中间 → 左/右 → 远左/远右 → 丢线保持
static int32 ir_get_error(int32 last_error)
{
    uint8 ir[5];
    for (int i = 0; i < 5; i++)
        ir[i] = gpio_get_level(IR_PINS[i]);

    // 按优先级检测: 中心 → 近侧 → 远侧
    if (ir[2] == IR_LINE_DETECT) return ERROR_MAP[2];   // 正中
    if (ir[1] == IR_LINE_DETECT) return ERROR_MAP[1];   // 偏左
    if (ir[3] == IR_LINE_DETECT) return ERROR_MAP[3];   // 偏右
    if (ir[0] == IR_LINE_DETECT) return ERROR_MAP[0];   // 远左
    if (ir[4] == IR_LINE_DETECT) return ERROR_MAP[4];   // 远右

    return last_error;  // 全白(丢线) 或 全黑(十字)，保持上次方向
}

// ======================== 主函数 ========================
int main(void)
{
    clock_init(SYSTEM_CLOCK_120M);
    debug_init();

    motor_init();
    ir_sensor_init();

    int32 error = 0;
    int32 last_error = 0;

    while (1)
    {
        error = ir_get_error(last_error);
        last_error = error;

        int32 turn_speed = error * TURN_KP;
        int32 left_speed  = BASE_SPEED + turn_speed;
        int32 right_speed = BASE_SPEED - turn_speed + RIGHT_MOTOR_COMP;

        pwm_set_duty(MOTOR_L_PWM, left_speed);
        pwm_set_duty(MOTOR_R_PWM, right_speed);
    }
}
