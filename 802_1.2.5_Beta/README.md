# BN802TY
## 版本号与尺寸对应关系

- T.X.X.X ：18.5寸屏参1366x768-刀头设备USER_APP_MASK
- S.X.X.X ：18.5寸屏参1366x768-407SS设备USER_APP_CARGO
- M.X.X.X ：15.4寸屏参1280x800-刀头设备USER_APP_MASK
- P.X.X.X ：19.0寸屏参1440x900-刀头设备USER_APP_MASK
- R.X.X.X ：19.0寸屏参1440x900-407SS设备USER_APP_CARGO
- Q.X.X.X ：21.5寸屏参1920x1080-刀头设备USER_APP_MASK

ssh-keygen
ssh-keygen -y -f id_rsa > id_rsa.pub
ssh-copy-id  proxy@1.15.103.154
ssh-keygen -R 1.15.103.154
ssh -CqTfnN -R 0.0.0.0:7233:127.0.0.1:22 proxy@1.15.103.154
ssh -CqTfnN -R 0.0.0.0:12345:127.0.0.1:22 proxy@1.15.103.154


autossh -M 12345 -CqTfnN -R :7233:127.0.0.1:22 proxy@1.15.103.154
netstat -anp | grep 7233

autossh -M 5678 -CNR 7233:0.0.0.0:22 proxy@1.15.103.154

2022.3.2自1.1.7版本以来修改说明：
一、主要增加的内容有：
1、鼎基刀头由控制1个增加为最多控制4个
2、新增适配切刀刀头部分。
3、新增1280*1024屏幕适配，增加内容均使用了条件编译，main_app.h line58-59
#define FANG_PING_1280x1024 0//方屏未适配拉伸，所以方屏需要按原分辨率进行使用
#define FANG_PING_1280x1024_SPECIAL_DEMAND 0//方屏特殊要求，横放状态下以16：9播放视频，最下面播放图片
使用了uint8_t Get1280x1024SpecialDemand(void)判断方屏有无特殊需求（方屏分屏播放），main_app.c line87。

二、修复bug：
1、系列黑屏，卡视频、卡图片的问题。
修复方法：降低刷新二维码、版本号、信号强度控件的频率。
修改内容有：
1、playControl.c
void *handleStartPlayerThread(void *arg)
void *handlePlayPicThread(void *arg)
2、net_listener.c
void *keep_net_thread(void *arg)
void CheckPlayTask(uint8_t split_screen)

125版本202120125

修复竖屏版本中，视频播放时未进行缩放的bug

124版本20211215

新增支持切刀刀头

123版本20211210

1、优化仅播放图片黑屏问题
2、优化5min断网重启问题
3、优化黑屏后无法恢复问题
4、优化播放维护任务，未进行播放任务判断盲目维护的问题（仅播放图片时也会维护视频播放）
5、老总特地要求，不需要每晚00：00重启


### 117版本 2021.08.09
1、解决R S 116版本中无图片任务时 重启时只有声音无图像
2、连续上架任务时，视频任务闪现
3、播放视频时开启视图，结束播放关闭视图
4、开机后直接播放本地已经下载的广告任务
5、上下架任务时 客户无感


### R.1.1.6  S.1.1.6 BUG修复2021.08.04 (本次更新只涉及R116和S116版本)
- 修复升级MCU中涉及到HTTPS下载接口前未去S导致升级失败的问题
- 修复上报MCU版本号不全的问题
- 将缺省图片播放方式也改为轮播，以便尽快响应新任务
- 修复下架所有任务后设备端无法上架新任务的问题


### 116版本 2021.07.26

- 如何解决监控播放器是否处于播放状态？切入点：进度条
- 如何测试：
    1.无广告任务 情况下 播放30s缺省图片 无重启
    2.有广告任务 杀死播放器 看能否重启
    3.只有图片任务 情况下能否喂狗
- 测试出bug：如果下架所有任务 再上架任务 设备不会播放本地任务 除非重启解决
- 解决方法：

#### 参数组/广告任务同步

1. 上架任务超过10个：是否溢出导致应用挂掉
2. 设备在线时下架任务：是否有删除日志且本地已经删除
3. 上架内存超过50M：设备是否拒绝下载且上报失败
4. 本地存在非法文件是否会自动删除
5. 过期视频是否还会在HTTP的API中给出：不会给出 但是会通过MQTT通知
6. 播放过程中发现过期视频是否会自动删除且上报日志
7. 参数组是否实时更新并响应到设备
8. 已经存在的任务和视频在重启后是否重新拉取(浪费流量)
    8.1 如果有上下架操作就会
    8.2 解决JSON文件受损 会导致 每次启动都重新下载 解决办法：发现解析出错时直接删掉该文件 根本原因是：sync的问题

#### 播放任务策略

1. 插上U盘后是否优先播放U盘视频：从非U盘模式切换到U盘模式
2. U盘中的文件是否过滤：没有合法文件，是否能自动切换到其他模式
3. RAM模式下，图片和视频混播是否正常
    3.1 注意：由于图片播放采用线程sleep方式来设定时长，因此会出现无法及时响应上下架操作
    3.2 在本地已经存在的任务 需要修改RAM和JSON文件中的播放时间和图片轮播时间(由于平台可以随时更改这些参数 而其他参数只能删除重建)
    3.3 发现问题：如果上下架操作的时候 正在播放图片 会导致新的请求与旧请求产生混乱：图音不符
        3.3.1 需要更改该策略：视频轮播策略是根据其回调完成的，而图片播放完全靠图片播放线程里的sleep完成
        3.3.2 将图片播放和到视频播放调用的同一个线程中
        3.3.3 把图片播放从线程改为阻塞操作
        3.3.4 停止图片播放就直接让UI不显示图片即可

4. 播放过程中是否精准到秒级别播放
5. 任务中的可修改参数是否实时生效
6. 缺省视频播放
7. 缺省图片播放

#### 用户APP测试

1. 刀头设备功能测试
2. 货道设备功能测试

#### 其他系统优化

1. /config/fbdev.ini 中增大FB_BUFFER_LEN为10240用于图片背景
2. mma需要继续增大(sz=0x3800000)暂时未做
3. 更改首次请求为网络连接后 
4. 增加在已存在任务的部分参数实时更改 
5. 将图片播放线程改为阻塞函数放到播放策略线程中与视频播放统一管理

### 1.1.5 2021.07.13

- 重点：U盘升级功能从业务应用剥离，使用纯shell脚本进行版本校验和OTA升级。



### R1.1.4 2021.07.07

- 凡是JSON相关的依赖请使用蜜连组件库中的CJSON 不可使用SDK自带库
- 新增支持BN407SS功能板

### 1.1.4 2021.07.06

- 修复表单为空时解析出错导致应用挂掉的bug
- MQTT重启设备前断电模块15s
- 通过main_app.h中的宏配置具体使用哪个串口设备应用

### 1.1.3 2021.07.03

- 将RS485串口的控制引脚由普通IO口更改为集成到串口驱动中，以解决总线释放不及时导致的乱码问题

### 1.1.2 2021.07.02

- mqtt上线后立即同步广告
- 替换播放器进程的动态库，修复某些视频只播放前几帧的问题
- 在播放视频前先关掉当前的视频

### 1.1.1 2021.06.29

- 新增二维码生成规则(默认蜜连推广二维码、自定义二维码、自定义+SN二维码)
- 将视频加载超时后视频播放状态强制更改为未播放以期待解决长时间黑屏问题
- 对AT指令获取的结果进行校验（因模块回复非顺序执行），避免iccid与csq信号值结果倒置。
- 在4G无网络情况下，修改45s时间为75s断电重启模块。

### 1.1.1 2021.06.24

- 新增二维码生成规则(默认蜜连推广二维码、自定义二维码、自定义+SN二维码)

### 1.1.0 2021.06.18

- 修改系统mma大小，用于高分辨率播放视频时内存申请
- 制作P110母片并测试
- 修复下架广告任务时在某些情况下无法同步本地广告的问题
- 修复拿到服务器视频下载链接中空格引起的下载链接无效的问题

### 1.0.9 2021.06.16

- 将电脑新增分区W用于解决flythingsIDE生成UI文件时的读写权限问题
- 在无网络情况下，将模块断电后重启系统，以使得模块重新寻网，增强网络稳定性

### 1.0.8 2021.06.08

- 播放器长时间挂测发现内存溢出导致播放器一直处于错误状态而不被看门狗发现
- 新增23:59分重启设备

### 1.0.7

- 1.新增在系统启动前插入U盘(NTFS/FAT32)也能挂载到指定路径并播放U盘视频
- 2.新增自动编译和自动打包OTA镜像的脚本
- 3.新增自动编译屏参相关文件的脚本
- 4.优化了U盘升级逻辑
- 5.优化UI更新逻辑 除切换视频更新UI外 新增网络检测中操作二维码显示和隐藏

### T.1.0.5

- 1.新增U盘按指定格式进行OTA升级(SStarOta.bin.gz_T.X.X.X),支持版本校验,自动删除升级包
- 2.新增U盘视频和网络视频切换播放(U盘视频优先原则)的功能
- 3.新增网络视频下发后同步本地视频的功能(删除和增加)

### T.1.0.4

- 1.支持ibeelink-http远程升级 升级成功后删除本地升级包
- 2.新增口罩机SQ800刀头业务APP，调整RS485控制引脚延时参数(后续可以优化到驱动中去)


修改部分：
#define UI_MAX_WIDTH			PANEL_MAX_WIDTH
#define UI_MAX_HEIGHT			PANEL_MAX_HEIGHT
static int g_s32VolValue = 50;
static bool g_bPlayCompleted = true;
stSetAttr.u32ChnCnt = 2;
s32SetVolumeDb


增加部分：
static int curDisplayImageTime = 0;
static pthread_t g_playPicThread = 1;
static bool isSplitDisplay = false;

减少部分：
StartPlayAudio---------system("echo 1 > /sys/class/gpio/gpio12/value");
StopPlayAudio----------system("echo 0 > /sys/class/gpio/gpio12/value");
MI_S32 CreatePlayerDev()---------------   system("echo 12 > /sys/class/gpio/export");
                                        system("echo out > /sys/class/gpio/gpio12/direction");
                                        //system("echo 1 > /sys/class/gpio/gpio12/value");
Pin60  gpio23
/*
StartPlayStreamFile
StopPlayStreamFile
PlayFileProc
myplayer:
    在makefile中通过宏SUPPORT_PLAYER_PROCESS使能跨进程模式, 需要单独编译生成bin文件.
    编译方法:
    1. cd myplayer/stdc++
    2. make clean;make
    3. 将生成的MyPlayer文件拷贝到sdk/verify/application/zk_full_sercurity/bin下

    播放器进程与主进程通讯说明.
    1. 播放器进程间通讯源码在UuidSSDPlayer/myplayer/app/main.cpp中, 主要接收主进程的消息, 并反馈播放状态信息.
    2. 主进程(即zkgui)通讯相关的源码在UuidSSDPlayer/stdc++/zk_full/jni/logic/playerLogic.cc中, 主要函数StartPlayStreamFile, StopPlayStreamFile, PlayFileProc发送消息到播放进程并接收反馈.
    3. 主进程中会通过UI的一些操作调到进程间通讯的接口. 主进程中会通过popen创建一个播放器进程, 退出时销毁播放器进程.
    
    使用注意事项：
    播放器切换为跨进程模式, SDK版本需要在V008版本上更新“20201224_DISP支持多进程”的RedFlag.
    跨进程模式下使用了心跳包机制, 如果进程间超过5秒没有消息握手, 播放进程会自动退出.
    播放器进程与主进程的版本需要保持一致, 否则可能导致进程间通讯失败.
*/

```
ffprobe -v quiet -show_frames -select_streams v no_b.mp4 | grep "pict_type=B"
```

```
ffmpeg -i have_b.mp4 -c:v libx264 -x264-params "bframes=0:ref=1" -strict -2 no_b.mp4
```

```
sudo rpm -Uvh http://li.nux.ro/download/nux/dextop/el7/x86_64/nux-dextop-release-0-1.el7.nux.noarch.rpm
```

```
sudo yum install -y automake autoconf libtool gcc gcc-c++
```

```
sudo yum install ffmpeg ffmpeg-devel
```

 ```
 sudo ffmpeg -version
 ```

