# MeArm 机械臂
> 小小机械臂，功能简单，学习必备。

## 视频
* B站 硬件篇：https://www.bilibili.com/video/BV1Eq4y1G7f1
* B站 软件篇：https://www.bilibili.com/video/BV1zr4y1y7CZ

## 材料清单
* MeArm 亚克力
* Arduino uno 官方版 x 1
* 面包板 x 1 
* 面包板电源 x 1 
* 伺服电机（舵机）SG90 x4
* 色环电阻 10K x1 , 20k x 1
* 蓝牙 AT-09 4.0BLE 模块
* 红外接收管 1838
* 其他材料：公对公 杜邦线若干，电工胶，5V 电源
* 可能用到的工具：螺丝刀，剪刀


## 电路图
Fritzing 软件打开 arm.fzz 文件， 也可以直接看 schematic.png

## 代码简单解释

1.  work(char cmd, int to) 
    指定某个电机 转动 到某个角度,为了保护电机需要限制最大转动角度，并且需要控制转动速度。

2.  workAction(Act act)
    enum 必须定义在所有函数前（Arduino 自己的 bug)
    实现方式: 读出当前电机角度 +- 2 为目标角度，调用 work 函数转动到目标角度

3.  workGroup(int action[][2], int len)
    动作组合序列， action 动作数组，len 数组长度，

3.  reportStatus 
    显示当前各电机所在的角度，为 workGroup 组合序列功能提供数值参考。

4.  iptvIRCmd
    长按红外遥控器时，第一次收到 value 指令，后面收到 FFFFFFFF,
    遇到 FFFFFFFF 时，执行上一次 value 指令，实现长按功能。

5.  serialCmd
    蓝牙通过 SoftwareSerial 传递数据，功能和串口监视器完全一致，可以认为是一个蓝牙键盘，所以可以合并串口监视器和蓝牙功能代码。

6.  happyGame
    在一个固定的地方抓取物品，设定 3 个 base 电机位置，然后随机旋转到这 3 个位置，再随机决定是否放下物品并结速游戏。

7.  小细节
* servo.attach，servo.write 之后需要 delay 一小段时间
* 尽量用 print(F"xx")) 代替 print("xx") 节约动态内存收益最大。

## 参考
太极创客
