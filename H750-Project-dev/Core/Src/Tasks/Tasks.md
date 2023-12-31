## 通用启动任务

1. 开机

   1. 打开电源开关

   2. 香橙派和STM32上电自启动

   3. STM32开机运行等待程序

   4. 等待时全开灯

      ```
      4、无人车在开机启动时，首先打开所有车灯（常亮或闪烁超过3 秒钟），车辆初始化完成后，必须采用非接触的感应方式出发（比如“挥手”或“遮挡”等），禁止使用遥控方式。
      ```

   5. 派自启动,给STM32发送信息

   6. 启动完成

2. 等待开始

   1. 按下按钮决定模式
      1. led_4 常亮
      2. TASK_A 直线行驶 Led_1 闪烁
      3. TASK_B 超车会车 Led_2 闪烁
      4. TASK_C 绕锥行驶 Led_3 闪烁

   2. TOF被遮挡
   3. 灯全灭，等待TOF遮挡消失

   4. TOF遮挡消失

   5. 启动开始，进入任务

## 任务一 直线行驶测试

1. 进入TASK_A
   - 接收信息 
     1. 侧边TOF——直线闭环行驶 
     2. 前面TOF ——检测是否停车
     3. 编码器——电机闭环行驶
2. 停车
   1. 无限等待，灯光闪烁

## 任务二 超车会车

1. 直线行驶

   1. 检测前方TOF来决定是否左转
   2. 侧边TOF直线闭环行驶

2. 左转向行驶

   ```
   无人车的前大灯（白色）、前后的左右转向灯（黄色）、后刹车灯（红色）必须处于正确的位置，能正常发挥功能并且能够便于观测。例如在超车或变道时，同侧的前后转向灯应同频率闪亮，恢复直行时熄灭；会车时前大灯应闪亮 2~3 次；减速或停车时后刹车灯应亮起，直至减速结束或车辆完全停住时灭灯。检录时，必须向裁判展示其所在的位置及正常的（闪亮）功能。
   ```
    1. 修改侧边TOF距离目标值
    2. 左边前后转向灯同频闪烁
    3. 前方TOF数据不读取

3. 直线行驶

   1. 灯全灭
   2. 恢复直线行驶，侧边TOF距离保持不变
   3. 保持一段时间，时间待调整

   ``` 
   转向灯未能闪亮的扣 5 分，超过龟行车 2 米以上未能回线者扣 10 分，压实线或出现轻度刮擦（仅限侧面）但仍能够继续行进的扣 10 分，出现严重刮擦、追尾或停止不前的扣 20 分，比赛结束。
   ```

4. 右转回右车道
   1. 侧边TOF目标值重设
   2. 右边前后转向灯同频闪烁
   3. 达到目标值后直行
   4. 前方TOF数据不读取
5. 直线行驶并会车
   1. 前大灯闪烁三次
   2. 左侧TOF直线闭环
   3. 左侧TOF检测对向行驶车辆
   4. 车辆行驶过去后左转
6. 左转向行驶
    1. 修改侧边TOF距离目标值
    2. 左边前后转向灯同频闪烁
    3. 前方TOF数据不读取
7. 直线行驶
   1. 灯全灭
   2. 直线闭环
   3. 前方TOF决定停车
8. 停车摆烂
   1. 停车灯常亮
   2. 无限等待

## 任务三 绕锥行驶

1. 直线行驶

   1. 灯全灭
   2. 前方TOF数据不读取 
   3. 侧边TOF距离保持不变
   4. 串口接受Lisii信息
   5. 解析数据决定左转时机

2. 左转向行驶

    1. 修改侧边TOF距离目标值
    2. 左边前后转向灯同频闪烁
    3. 前方TOF数据不读取 
   
3. 直线行驶
   1. 灯全灭
   2. 前方TOF数据不读取 
   3. 侧边TOF距离保持不变
   4. 串口接受Lisii信息
   5. 解析数据决定右转时机

4. 右转向行驶

    1. 修改侧边TOF距离目标值
    2. 右边前后转向灯同频闪烁
    3. 前方TOF数据不读取 

5. 直线行驶
   1. 灯全灭
   2. 前方TOF数据不读取 
   3. 侧边TOF距离保持不变
   4. 串口接受Lisii信息
   5. 解析数据决定减速时机
   
6. 减速行驶
   1. 调整目标速度
   2. 刹车灯常亮
   3. 串口接受Lisii信息
   4. 解析数据决定停车时机

7. 停车行驶
   1. 调整目标速度为0
   2. 等待5s后继续行驶
   
8. 直线行驶
   1. 灯全灭
   2. 前方TOF决定停车时机
   3. 侧边TOF距离保持不变
   
9. 停车摆烂
   1. 停车灯常亮
   2. 无限等待