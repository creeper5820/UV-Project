#include "Messager.hh"

#define STRAIGHT 0
#define LEFT 1
#define RIGHT 2

// Message caches
static Data_OpenCV data_opencv_temp;
static Data_Motion data_motion_temp;
static Data_Tof data_tof_temp;
static Data_Light data_light_temp;

extern Serial_Transceiver lisii;
extern Motion_Controller motion;

// Queue to use
auto Queue_Motion = xQueueCreate(1, sizeof(Data_Motion));
auto Queue_System = xQueueCreate(1, sizeof(Data_System));
auto Queue_Opencv = xQueueCreate(1, sizeof(Data_OpenCV));
auto Queue_Margin = xQueueCreate(1, sizeof(Data_Margin));
auto Queue_Light = xQueueCreate(1, sizeof(Data_Light));
auto Queue_Tof = xQueueCreate(1, sizeof(Data_Tof));

/******************************************
 * @brief Loop function of the task
 */
void Messager_Loop()
{

    for (;;) {

        if (flag_begin)
            Begin();

        switch (model) {

        case MODEL_DEBUG:
            Model_Debug();
            break;

        case TASK_A:
            Task_A();
            break;

        case TASK_B:
            Task_B();
            break;

        case TASK_C:
            Task_C();
            break;
        }
    }
}

void Begin()
{
    Light_Mode(model);

    // 按键消抖
    osDelay(200);

    // 使用按键调节模式
    while (Button_Scan(Button_1_GPIO_Port, Button_1_Pin)) {

        if (!Button_Scan(Button_5_GPIO_Port, Button_5_Pin) && model > 0) {
            osDelay(20);
            if (!Button_Scan(Button_5_GPIO_Port, Button_5_Pin)) {
                model--;
                Light_Mode(model);
            }
        }

        if (!Button_Scan(Button_3_GPIO_Port, Button_3_Pin) && model < 3) {
            osDelay(20);
            if (!Button_Scan(Button_3_GPIO_Port, Button_3_Pin)) {
                model++;
                Light_Mode(model);
            }
        }

        osDelay(100);
    }

    Light_Mode(FLASH_ALL);

    if (flag_debug)
        lisii.Send((char*)"Wait OrangePi to send message\n", sizeof("Wait OrangePi to send message"));

    // 等待香橙派发送信息
    if (flag_wait_pi)
        while (xQueueReceive(Queue_Opencv, &data_opencv_temp, 100) != pdTRUE) {
            Light_Mode(FLASH_ALL);
            // 避免阻塞优先级低的线程，有必要加一个延时
            osDelay(50);
        }

    if (flag_debug)
        lisii.Send((char*)"Done!\n", sizeof("Done!"));

    Light_Mode(LIGHT_ALL);

    if (flag_debug)
        lisii.Send((char*)"Wait tof to be covered by hand\n", sizeof("Wait tof to be covered by hand"));

    // 清空消息队列
    xQueueReset(Queue_Tof);
    data_tof_temp.distance = 8888;

    // TOF有遮挡时下一步
    Scan_Block_Next(10);

    if (flag_debug)
        lisii.Send((char*)"Done!\n", sizeof("Done!"));

    // 灯光全部亮起
    data_light_temp.status = FLASH_ALL;
    xQueueOverwrite(Queue_Light, &data_light_temp);

    if (flag_debug)
        lisii.Send((char*)"Wait hand removed\n", sizeof("Wait hand removed"));

    // 清空消息队列
    xQueueReset(Queue_Tof);

    // TOF没有遮挡时下一步
    Scan_Block_Stop(20);

    // 灯光全部暗下
    Light_Mode(DARK_ALL);

    if (flag_debug)
        lisii.Send((char*)"Done!\n", sizeof("Done!"));

    data_motion_temp = { 0, distance_right };
    xQueueOverwrite(Queue_Motion, &data_motion_temp);

    osDelay(1000);
}

/******************************************
 * @brief 直线行驶的任务, 直线行驶至墙墙则停止
 */
void Task_A()
{
    float speed_task_a = 0.25;

    Set_Speed(0, distance_right);

    Light_Mode(TASK_A);

    Set_Speed(speed_task_a, distance_right);

    // 无限阻塞以调试
    if (0)
        while (1)
            osDelay(50);

    // 防止抖动
    osDelay(1000);

    // 检测到前方的墙后进行下一步
    Scan_Block_Next(90);

    if (flag_debug)
        lisii.Send((char*)"stop\n", sizeof("stop"));

    // 刹车
    Light_Mode(LIGHT_TAIL);

    Set_Speed(-0.1, distance_right);

    osDelay(500);

    Set_Speed(0, distance_right);

    Light_Mode(DARK_ALL);
    Light_Mode(PREPARED);

    while (Button_Scan(Button_1_GPIO_Port, Button_1_Pin))
        ;
}

/******************************************
 * @brief 绕行锥桶的任务,右车道遇到锥桶转向左车道,再次遇到锥桶后转向右车道,最后停止
 */
void Task_B()
{
    data_light_temp.status = TASK_C;
    xQueueOverwrite(Queue_Light, &data_light_temp);

    // 直线行驶
    motion.flagTurn = STRAIGHT;

    Set_Speed(0.25, distance_right);

    Scan_Block_Next(150);

    // 转向左车道
    motion.flagTurn = LEFT;

    osDelay(400);

    motion.flagTurn = RIGHT;

    osDelay(400);

    // 回复直线行驶
    motion.flagTurn = STRAIGHT;

    Set_Speed(0.20, distance_left);

    Scan_Block_Next(150);

    // 转向右车道
    motion.flagTurn = RIGHT;

    osDelay(400);

    motion.flagTurn = LEFT;

    osDelay(400);

    // 回复直线行驶
    motion.flagTurn = STRAIGHT;

    Set_Speed(0.20, distance_left);

    Scan_Block_Next(100);

    // 刹车
    Set_Speed(-0.4, distance_left);

    osDelay(400);

    Set_Speed(0, distance_left);

    while (Button_Scan(Button_1_GPIO_Port, Button_1_Pin))
        ;
}

/******************************************
 * @brief 超车会车的任务, 超过第一辆车,回道后和第二辆车回车,最后转向左车道
 */
void Task_C()
{
    // 直线行驶
    motion.flagTurn = STRAIGHT;
    Set_Speed(0.25, distance_right);

    // 遇到前方红色车
    do {
        xQueueReceive(Queue_Opencv, &data_opencv_temp, portMAX_DELAY);
        osDelay(20);
    } while (data_opencv_temp.flag_turn != 1);

    // 向左转向
    Turn_Left(400);

    // 直线行驶
    motion.flagTurn = STRAIGHT;
    Set_Speed(0.25, distance_left);

    // 等超过红色车辆
    osDelay(1000);

    // 转向右车道
    Turn_Right(400);

    // 直线行驶
    motion.flagTurn = STRAIGHT;
    Set_Speed(0.25, distance_right);

    // 测距距离突然变小,检测到左方车辆
    while (motion.bias_balance_ > -20)
        ;

    // 测距距离突然变大,检测到回车结束
    while (motion.bias_balance_ < 20)
        ;

    // 向左转向
    Turn_Left(400);

    // 直线行驶
    motion.flagTurn = STRAIGHT;
    Set_Speed(0.25, distance_left);

    Scan_Block_Next(100);

    // 刹车
    Set_Speed(-0.4, distance_left);

    osDelay(400);

    Set_Speed(0, distance_left);

    while (Button_Scan(Button_1_GPIO_Port, Button_1_Pin))
        ;
}

/******************************************
 * @brief Debug用的任务,随意添加函数即可
 */
void Model_Debug()
{
    data_light_temp.status = MODEL_DEBUG;
    xQueueOverwrite(Queue_Light, &data_light_temp);

    for (;;) {

        if (0)
            if (xQueueReceive(Queue_Opencv, &data_opencv_temp, 0) == pdTRUE) {
                Show_Data_OpenCV(data_opencv_temp, lisii);
            }

        if (0)
            if (xQueueReceive(Queue_Tof, &data_tof_temp, 0) == pdTRUE) {
                Show_Data_Tof(data_tof_temp, lisii);
            }

        if (0) {

            /**
             * 面向车头 
             * 右边 1
             * 后面 3
             * 前面 5
             */

            char send_button[10];
            int size_button = sprintf(send_button, "B- %d\n",
                Button_Scan(Button_1_GPIO_Port, Button_1_Pin));

            lisii.Send(send_button, size_button);
        }

        Set_Speed(0, 20);

        if (0) {
            HAL_GPIO_WritePin(Light_1_GPIO_Port, Light_1_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Light_2_GPIO_Port, Light_2_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Light_3_GPIO_Port, Light_3_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(Light_4_GPIO_Port, Light_4_Pin, GPIO_PIN_RESET);
        }

        if (1) {
            xQueueReceive(Queue_Opencv, &data_opencv_temp, portMAX_DELAY);

            if (data_opencv_temp.flag_turn == 1)
                motion.flagTurn = RIGHT;
            else
                motion.flagTurn = LEFT;
        }

        osDelay(50);
    }
}

void Set_Speed(float speed, float distance)
{
    data_motion_temp = { speed, distance };
    xQueueOverwrite(Queue_Motion, &data_motion_temp);
}

void Light_Mode(int mode)
{
    data_light_temp.status = mode;
    xQueueOverwrite(Queue_Light, &data_light_temp);
}

// 检测到前方的障碍物 有障碍物时进行下一步
void Scan_Block_Next(int distance)
{
    int flag_block = 0;
    xQueueReset(Queue_Tof);

    while (flag_block < 1) {

        xQueueReceive(Queue_Tof, &data_tof_temp, 10);

        if (data_tof_temp.distance < distance)
            flag_block++;

        osDelay(50);
    };
}

// 检测到前方的障碍物 没有障碍物时才能进行下一步
void Scan_Block_Stop(int distance)
{
    int flag_block = 0;
    xQueueReset(Queue_Tof);

    while (flag_block < 1) {

        xQueueReceive(Queue_Tof, &data_tof_temp, 10);

        if (data_tof_temp.distance > distance)
            flag_block++;

        osDelay(50);
    };
}

void Turn_Right(int time_ms)
{
    // 转向右车道
    motion.flagTurn = RIGHT;

    osDelay(time_ms);

    motion.flagTurn = LEFT;

    osDelay(time_ms);

    // 回复直线行驶
    motion.flagTurn = STRAIGHT;
}

void Turn_Left(int time_ms)
{
    // 转向右车道
    motion.flagTurn = LEFT;

    osDelay(time_ms);

    motion.flagTurn = RIGHT;

    osDelay(time_ms);

    // 回复直线行驶
    motion.flagTurn = STRAIGHT;
}