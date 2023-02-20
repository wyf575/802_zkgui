/*
 * MediaPlayer.cpp
 *
 *  Created on: 2021年2月1日
 *      Author: Administrator
 */
#include "MediaPlayer.h"
#include "utils/Log.h"
#define printf LOGD

#include <pthread.h>
#include "storage/StoragePreferences.h"
#include "baselib/UrlManager.h"
#include "baselib/utils.h"
#include "baselib/DeviceManager.h"
#include "model/JsonParse.h"
#include "model/ModelManager.h"
#include "model/MediaManager.h"
#include "../curl/curl.h"
#include "restclient-cpp/restclient.h"
#include "utils/log.h"
//honestar karl.hong add 20210826
#include "appconfig.h"
#include "main_app/main_app.h"
//honestar karl.hong add 20210826

static ZKTextView* mTextViewPicPtr;
void *userCallback = NULL;

#ifdef SUPPORT_PLAYER_PROCESS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/prctl.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct shared_use_st
{
    int  written;    //作为一个标志，非0：表示可读，0表示可写
    bool flag;
};

struct shared_use_st *shm_addr = NULL;
int shm_id = 0;

#define CLT_IPC         "/appconfigs/client_input"
#define SVC_IPC         "/appconfigs/server_input"
#define MYPLAYER_PATH   "/customer/MyPlayer &"

typedef enum
{
  IPC_COMMAND_OPEN,
  IPC_COMMAND_CLOSE,
  IPC_COMMAND_PAUSE,
  IPC_COMMAND_RESUME,
  IPC_COMMAND_SEEK,
  IPC_COMMAND_SEEK2TIME,
  IPC_COMMAND_GET_POSITION,//6
  IPC_COMMAND_GET_DURATION,//7
  IPC_COMMAND_MAX,
  IPC_COMMAND_ACK,//9
  IPC_COMMAND_SET_VOLUMN,
  IPC_COMMAND_SET_MUTE,
  IPC_COMMAND_ERROR,
  IPC_COMMAND_COMPLETE,
  IPC_COMMAND_CREATE,
  IPC_COMMAND_DESTORY,//15
  IPC_COMMAND_EXIT,
  IPC_COMMAND_PANT//17
} IPC_COMMAND_TYPE;

typedef struct{
    int x;
    int y;
    int width;
    int height;
    double misc;
    int aodev, volumn;
    int status;
    int rotate;
    bool mute;
    bool audio_only, video_only;
    int  play_mode;
    char filePath[512];
}stPlayerData;

typedef struct {
    unsigned int EventType;
    stPlayerData stPlData;
} IPCEvent;

class IPCOutput {
public:
    IPCOutput(const std::string& file):m_fd(-1), m_file(file) {
    }

    virtual ~IPCOutput() {
        Term();
    }

    bool Init() {
        if (m_fd < 0) {
            m_fd = open(m_file.c_str(), O_WRONLY | O_NONBLOCK, S_IWUSR | S_IWOTH);
            printf("IPCOutput m_fd = %d\n", m_fd);
        }
        return m_fd >= 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
        printf("%s term!\n", m_file.c_str());
    }

    int Send(const IPCEvent& evt) {
        if (m_fd >= 0) {
            return write(m_fd, &evt, sizeof(IPCEvent));
        }
        printf("write %s failed!\n", m_file.c_str());
        return -1;
    }

private:
    int m_fd, ret;
    std::string m_file;
};

class IPCNameFifo {
public:
    IPCNameFifo(const char* file): m_file(file) {
        unlink(m_file.c_str());
        m_valid = !mkfifo(m_file.c_str(), 0777);
    }

    ~IPCNameFifo() {
    unlink(m_file.c_str());
}

inline const std::string& Path() { return m_file; }
inline bool IsValid() { return m_valid; }

private:
    bool m_valid;
    std::string m_file;
};

class IPCInput {
public:
    IPCInput(const std::string& file):m_fd(-1),m_file(file),m_fifo(file.c_str()){}

    virtual ~IPCInput() {
        Term();
    }

    bool Init() {
        if (!m_fifo.IsValid()){
            printf("%s non-existent!!!! \n",m_fifo.Path().c_str());
            return false;
        }
        if (m_fd < 0) {
            m_fd = open(m_file.c_str(), O_RDWR | O_CREAT | O_NONBLOCK, S_IRWXU | S_IWOTH);
            printf("IPCInput m_fd = %d\n", m_fd);
        }
        return m_fd >= 0;
    }

    int Read(IPCEvent& evt) {
        if (m_fd >= 0) {
            return read(m_fd, &evt, sizeof(IPCEvent));
        }
        printf("read %s failed!\n", m_file.c_str());
        return 0;
    }

    void Term() {
        if (m_fd >= 0) {
            close(m_fd);
        }
        m_fd = -1;
        printf("%s term!\n", m_file.c_str());
    }

private:
    int m_fd, ret;
    std::string m_file;
    IPCNameFifo m_fifo;
};

IPCEvent recvevt;
IPCEvent sendevt;
IPCInput  i_server(SVC_IPC);
IPCOutput o_client(CLT_IPC);

#endif
#define SUPPORT_PLAYER_MODULE
#ifdef SUPPORT_PLAYER_MODULE
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "mi_sys.h"
#include "mi_divp.h"
#include "mi_disp.h"
#include "mi_vdec_extra.h"
//#include "mi_panel.h"
#include "mi_gfx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "frame.h"
#include "demux.h"
#include "videostream.h"
#include "audiostream.h"
#include "player.h"
#include "blitutil.h"

#include "panelconfig.h"
#include "hotplugdetect.h"
#include "imageplayer.h"

#define UI_MAX_WIDTH			PANEL_MAX_WIDTH
#define UI_MAX_HEIGHT			PANEL_MAX_HEIGHT

#define ALIGN_DOWN(x, n)        (((x) / (n)) * (n))
#define MAKE_YUYV_VALUE(y,u,v)  ((y) << 24) | ((u) << 16) | ((y) << 8) | (v)
#define YUYV_BLACK              MAKE_YUYV_VALUE(0,128,128)
#define YUYV_WHITE              MAKE_YUYV_VALUE(255,128,128)
#define YUYV_RED                MAKE_YUYV_VALUE(76,84,255)
#define YUYV_GREEN              MAKE_YUYV_VALUE(149,43,21)
#define YUYV_BLUE               MAKE_YUYV_VALUE(29,225,107)

#define DIVP_CHN        		0
#define DISP_DEV        		0
#define DISP_LAYER      		0
#define DISP_INPUTPORT  		0
#define AUDIO_DEV       		0
#define AUDIO_CHN       		0
#define AUDIO_SAMPLE_PER_FRAME  1024
#define AUDIO_MAX_DATA_SIZE     25000
#define MIN_AO_VOLUME           -60
#define MAX_AO_VOLUME           30
#define MIN_ADJUST_AO_VOLUME    -10
#define MAX_ADJUST_AO_VOLUME    20

#define VOL_ADJUST_FACTOR		2
#define PROGRESS_UPDATE_TIME_INTERVAL	500000		// 0.5s

#define DISPLAY_PIC_DURATION	5000000				// 5s

typedef enum
{
    E_PLAY_FORWARD,
    E_PLAY_BACKWARD
}PlayDirection_e;

typedef enum
{
    E_PLAY_NORMAL_MODE,
    E_PLAY_FAST_MODE,
    E_PLAY_SLOW_MODE
}PlayMode_e;

typedef enum
{
    E_NORMAL_SPEED = 0,
    E_2X_SPEED,
    E_4X_SPEED,
    E_8X_SPEED,
    E_16X_SPEED,
    E_32X_SPEED
}PlaySpeedMode_e;

typedef enum
{
	FILE_REPEAT_MODE,
	LIST_REPEAT_MODE
}RepeatMode_e;

typedef enum
{
	NO_SKIP,
	SKIP_NEXT,
	SKIP_PREV
}SkipMode_e;

// playing page
static bool g_bShowPlayToolBar = FALSE;         // select file list page or playing page
static bool g_bPlaying = FALSE;					// 正在播放状态
static bool g_bPause = FALSE;					// 播放暂停状态
static bool g_bMute = FALSE;
static int g_s32VolValue = 50;
static bool g_ePlayDirection = E_PLAY_FORWARD;
static PlayMode_e g_ePlayMode = E_PLAY_NORMAL_MODE;
static PlaySpeedMode_e g_eSpeedMode = E_NORMAL_SPEED;
static unsigned int g_u32SpeedNumerator = 1;
static unsigned int g_u32SpeedDenomonator = 1;

// playViewer size
static int g_playViewWidth = PANEL_MAX_WIDTH;
static int g_playViewHeight = PANEL_MAX_HEIGHT;

// streamplayer & imagePlayer
static std::string g_fileName;
static player_stat_t *g_pstPlayStat = NULL;
static ImagePlayer_t *g_pstImagePlayer = NULL;
static int curDisplayImageTime = 0;

// play pos
static long long g_firstPlayPos = PLAY_INIT_POS;
static long long g_duration = 0;
static long long int g_lastpos = 0;

// play video/audio or picture
static int g_playStream = 0;
static pthread_t g_playFileThread = 0;
static pthread_t g_playPicThread = 1;
static bool g_bPlayFileThreadExit = false;
static bool g_bPlayCompleted = true;
static bool g_bPlayError = false;
static pthread_mutex_t g_playFileMutex;
static bool g_bPantStatus = false;

static bool isSplitDisplay = false;

MI_S32 StartPlayAudio()
{
    MI_AUDIO_Attr_t stSetAttr;
    MI_AUDIO_Attr_t stGetAttr;
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;

    MI_S32 s32SetVolumeDb;
    MI_S32 s32GetVolumeDb;

    //set Ao Attr struct
    memset(&stSetAttr, 0, sizeof(MI_AUDIO_Attr_t));
    stSetAttr.eBitwidth = E_MI_AUDIO_BIT_WIDTH_16;
    stSetAttr.eWorkmode = E_MI_AUDIO_MODE_I2S_MASTER;
    stSetAttr.u32FrmNum = 6;
    stSetAttr.u32PtNumPerFrm = AUDIO_SAMPLE_PER_FRAME;
    stSetAttr.u32ChnCnt = 2;

    if(stSetAttr.u32ChnCnt == 2)
    {
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_STEREO;
    }
    else if(stSetAttr.u32ChnCnt == 1)
    {
        stSetAttr.eSoundmode = E_MI_AUDIO_SOUND_MODE_MONO;
    }

    stSetAttr.eSamplerate = E_MI_AUDIO_SAMPLE_RATE_48000;

    /* set ao public attr*/
    MI_AO_SetPubAttr(AoDevId, &stSetAttr);

    /* get ao device*/
    MI_AO_GetPubAttr(AoDevId, &stGetAttr);

    /* enable ao device */
    MI_AO_Enable(AoDevId);

    /* enable ao channel of device*/
    MI_AO_EnableChn(AoDevId, AoChn);

    printf("g_s32VolValue---%d\n", g_s32VolValue);
    /* if test AO Volume */
    if (g_s32VolValue)
//    	s32SetVolumeDb = g_s32VolValue * (MAX_ADJUST_AO_VOLUME-MIN_ADJUST_AO_VOLUME) / 100 + MIN_ADJUST_AO_VOLUME;
    	s32SetVolumeDb = 0.14 * g_s32VolValue * (MAX_ADJUST_AO_VOLUME-MIN_ADJUST_AO_VOLUME) / 100 + MIN_ADJUST_AO_VOLUME;
    else
    	s32SetVolumeDb = MIN_AO_VOLUME;


    MI_AO_SetVolume(AoDevId, s32SetVolumeDb);
    MI_AO_SetMute(AoDevId, g_bMute);

    /* get AO volume */
    MI_AO_GetVolume(AoDevId, &s32GetVolumeDb);

    return 0;
}
//add by wyf 
void SSTAR_setVolume(int vol)
{
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_SetVolume(AoDevId, vol - 60);
}

void StopPlayAudio()
{
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;

    /* disable ao channel of */
    MI_AO_DisableChn(AoDevId, AoChn);

    /* disable ao device */
    MI_AO_Disable(AoDevId);
}

MI_S32 StartPlayVideo()
{
    MI_DISP_ShowInputPort(DISP_LAYER, DISP_INPUTPORT);
	return 0;
}

void StopPlayVideo()
{
	MI_DISP_ClearInputPortBuffer(DISP_LAYER, DISP_INPUTPORT, TRUE);
    MI_DISP_HideInputPort(DISP_LAYER, DISP_INPUTPORT);
}

MI_S32 CreatePlayerDev()
{
    /*MI_SYS_ChnPort_t stDivpChnPort;
    MI_DIVP_ChnAttr_t stDivpChnAttr;
    MI_DIVP_OutputPortAttr_t stOutputPortAttr;

    MI_U32 u32InputPort = DISP_INPUTPORT;
    MI_DISP_DEV dispDev = DISP_DEV;
    MI_DISP_LAYER dispLayer = DISP_LAYER;
    MI_SYS_ChnPort_t stDispChnPort;
    MI_DISP_RotateConfig_t stRotateConfig;*/

    MI_DISP_InputPortAttr_t stInputPortAttr;

    // 1.初始化DISP模块
    memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
    MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
    stInputPortAttr.u16SrcWidth         = ALIGN_DOWN(g_playViewWidth , 32);
    stInputPortAttr.u16SrcHeight        = ALIGN_DOWN(g_playViewHeight, 32);
    stInputPortAttr.stDispWin.u16X      = 0;
    stInputPortAttr.stDispWin.u16Y      = 0;
    stInputPortAttr.stDispWin.u16Width  = g_playViewWidth;
    stInputPortAttr.stDispWin.u16Height = g_playViewHeight;

    printf("disp input: w=%d, h=%d\n", stInputPortAttr.u16SrcWidth, stInputPortAttr.u16SrcHeight);

    // 2.初始化DIVP模块
    /*memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
    stDivpChnAttr.bHorMirror            = FALSE;
    stDivpChnAttr.bVerMirror            = FALSE;
    stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
    stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
    stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
    stDivpChnAttr.stCropRect.u16X       = 0;
    stDivpChnAttr.stCropRect.u16Y       = 0;
    stDivpChnAttr.stCropRect.u16Width   = 0;
    stDivpChnAttr.stCropRect.u16Height  = 0;
    stDivpChnAttr.u32MaxWidth           = 1920;
    stDivpChnAttr.u32MaxHeight          = 1080;

    MI_DIVP_CreateChn(DIVP_CHN, &stDivpChnAttr);
    MI_DIVP_StartChn(DIVP_CHN);
    
    memset(&stOutputPortAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
    stOutputPortAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
    stOutputPortAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
    stOutputPortAttr.u32Width           = ALIGN_DOWN(g_playViewWidth , 32);
    stOutputPortAttr.u32Height          = ALIGN_DOWN(g_playViewHeight, 32);

    // 3.配置旋转属性
    stRotateConfig.eRotateMode          = E_MI_DISP_ROTATE_NONE;
    MI_DISP_SetVideoLayerRotateMode(dispLayer, &stRotateConfig);

    if (stRotateConfig.eRotateMode == E_MI_DISP_ROTATE_NONE || stRotateConfig.eRotateMode == E_MI_DISP_ROTATE_180)
    {
        stInputPortAttr.u16SrcWidth     = ALIGN_DOWN(g_playViewWidth , 32);
        stInputPortAttr.u16SrcHeight    = ALIGN_DOWN(g_playViewHeight, 32);
        stOutputPortAttr.u32Width       = ALIGN_DOWN(g_playViewWidth , 32);
        stOutputPortAttr.u32Height      = ALIGN_DOWN(g_playViewHeight, 32);
    }
    else
    {
        stInputPortAttr.u16SrcWidth     = ALIGN_DOWN(g_playViewHeight, 32);
        stInputPortAttr.u16SrcHeight    = ALIGN_DOWN(g_playViewWidth , 32);
        stOutputPortAttr.u32Width       = ALIGN_DOWN(g_playViewHeight, 32);
        stOutputPortAttr.u32Height      = ALIGN_DOWN(g_playViewWidth , 32);
    }*/

    MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
    MI_DISP_SetInputPortAttr(DISP_LAYER, DISP_INPUTPORT, &stInputPortAttr);
    MI_DISP_EnableInputPort(DISP_LAYER, DISP_INPUTPORT);
    MI_DISP_SetInputPortSyncMode(DISP_LAYER, DISP_INPUTPORT, E_MI_DISP_SYNC_MODE_FREE_RUN);

    //MI_DIVP_SetOutputPortAttr(0, &stOutputPortAttr);

    //4.绑定DIVP与DISP
    /*memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId              = DISP_DEV;
    stDispChnPort.u32ChnId              = 0;
    stDispChnPort.u32PortId             = DISP_INPUTPORT;
    
    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId              = 0;
    stDivpChnPort.u32ChnId              = DIVP_CHN;
    stDivpChnPort.u32PortId             = 0;

    MI_SYS_SetChnOutputPortDepth(&stDivpChnPort, 0, 3);
    MI_SYS_BindChnPort(&stDivpChnPort, &stDispChnPort, 30, 30);*/

    return 0;
}

void DestroyPlayerDev()
{
    /*MI_DISP_LAYER dispLayer = DISP_LAYER;
    MI_U32 u32InputPort = DISP_INPUTPORT;
    MI_SYS_ChnPort_t stDivpChnPort;
    MI_SYS_ChnPort_t stDispChnPort;
    
    memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
    stDispChnPort.u32DevId              = DISP_DEV;
    stDispChnPort.u32ChnId              = 0;
    stDispChnPort.u32PortId             = DISP_INPUTPORT;
    
    memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
    stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
    stDivpChnPort.u32DevId              = 0;
    stDivpChnPort.u32ChnId              = DIVP_CHN;
    stDivpChnPort.u32PortId             = 0;
    
    MI_SYS_UnBindChnPort(&stDivpChnPort, &stDispChnPort);

    MI_DIVP_StopChn(0);
    MI_DIVP_DestroyChn(0);*/

    MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
}

#ifndef SUPPORT_PLAYER_PROCESS
static MI_S32 SetVideoDisplay(void)
{
    MI_DISP_RotateConfig_t stRotateConfig;

    if (g_pstPlayStat->decode_type == SOFT_DECODING)
    {
        MI_DIVP_ChnAttr_t stDivpChnAttr;
        MI_DIVP_OutputPortAttr_t stOutputPortAttr;
        MI_DISP_InputPortAttr_t stInputPortAttr;
        MI_SYS_ChnPort_t stDispChnPort;
        MI_SYS_ChnPort_t stDivpChnPort;

        MI_GFX_Open();

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
        stInputPortAttr.u16SrcWidth         = ALIGN_DOWN(g_pstPlayStat->src_width , 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_DOWN(g_pstPlayStat->src_height, 32);
        stInputPortAttr.stDispWin.u16X      = g_pstPlayStat->pos_x;
        stInputPortAttr.stDispWin.u16Y      = g_pstPlayStat->pos_y;
        stInputPortAttr.stDispWin.u16Width  = g_pstPlayStat->out_width;
        stInputPortAttr.stDispWin.u16Height = g_pstPlayStat->out_height;

        MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_SetInputPortAttr(DISP_LAYER, DISP_INPUTPORT, &stInputPortAttr);
        MI_DISP_EnableInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_SetInputPortSyncMode(DISP_LAYER, DISP_INPUTPORT, E_MI_DISP_SYNC_MODE_FREE_RUN);

        memset(&stDivpChnAttr, 0, sizeof(MI_DIVP_ChnAttr_t));
        stDivpChnAttr.bHorMirror            = FALSE;
        stDivpChnAttr.bVerMirror            = FALSE;
        stDivpChnAttr.eDiType               = E_MI_DIVP_DI_TYPE_OFF;
        stDivpChnAttr.eRotateType           = E_MI_SYS_ROTATE_NONE;
        stDivpChnAttr.eTnrLevel             = E_MI_DIVP_TNR_LEVEL_OFF;
        stDivpChnAttr.stCropRect.u16X       = 0;
        stDivpChnAttr.stCropRect.u16Y       = 0;
        stDivpChnAttr.stCropRect.u16Width   = 0;
        stDivpChnAttr.stCropRect.u16Height  = 0;
        stDivpChnAttr.u32MaxWidth           = 1920;
        stDivpChnAttr.u32MaxHeight          = 1080;

        MI_DIVP_CreateChn(DIVP_CHN, &stDivpChnAttr);
        MI_DIVP_SetChnAttr(0, &stDivpChnAttr);

        memset(&stOutputPortAttr, 0, sizeof(MI_DIVP_OutputPortAttr_t));
        stOutputPortAttr.eCompMode          = E_MI_SYS_COMPRESS_MODE_NONE;
        stOutputPortAttr.ePixelFormat       = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stOutputPortAttr.u32Width           = ALIGN_DOWN(g_pstPlayStat->src_width , 32);
        stOutputPortAttr.u32Height          = ALIGN_DOWN(g_pstPlayStat->src_height, 32);
        MI_DIVP_SetOutputPortAttr(0, &stOutputPortAttr);
		MI_DIVP_StartChn(0);

        memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
        stDispChnPort.u32DevId              = DISP_DEV;
        stDispChnPort.u32ChnId              = 0;
        stDispChnPort.u32PortId             = DISP_INPUTPORT;

        memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
        stDivpChnPort.u32DevId              = 0;
        stDivpChnPort.u32ChnId              = DIVP_CHN;
        stDivpChnPort.u32PortId             = 0;

        MI_SYS_SetChnOutputPortDepth(&stDivpChnPort, 0, 3);
        MI_SYS_BindChnPort(&stDivpChnPort, &stDispChnPort, 30, 30);

        // 软解采用GFX旋转, 无须设置DISP
        stRotateConfig.eRotateMode = E_MI_DISP_ROTATE_NONE;
    }
    else
    {
        MI_DISP_InputPortAttr_t stInputPortAttr;

        memset(&stInputPortAttr, 0, sizeof(MI_DISP_InputPortAttr_t));
        MI_DISP_GetInputPortAttr(0, 0, &stInputPortAttr);
        stInputPortAttr.u16SrcWidth         = ALIGN_DOWN(g_pstPlayStat->src_width , 32);
        stInputPortAttr.u16SrcHeight        = ALIGN_DOWN(g_pstPlayStat->src_height, 32);
        stInputPortAttr.stDispWin.u16X      = g_pstPlayStat->pos_x;
        stInputPortAttr.stDispWin.u16Y      = g_pstPlayStat->pos_y;
        stInputPortAttr.stDispWin.u16Width  = g_pstPlayStat->out_width;
        stInputPortAttr.stDispWin.u16Height = g_pstPlayStat->out_height;

        MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_SetInputPortAttr(DISP_LAYER, DISP_INPUTPORT, &stInputPortAttr);
        MI_DISP_EnableInputPort(DISP_LAYER, DISP_INPUTPORT);
        MI_DISP_SetInputPortSyncMode(DISP_LAYER, DISP_INPUTPORT, E_MI_DISP_SYNC_MODE_FREE_RUN);
        MI_DISP_ShowInputPort(DISP_LAYER, DISP_INPUTPORT);
        // 硬解时使用DISP旋转
        stRotateConfig.eRotateMode = (MI_DISP_RotateMode_e)g_pstPlayStat->display_mode;
    }

    MI_DISP_SetVideoLayerRotateMode(DISP_LAYER, &stRotateConfig);

    return MI_SUCCESS;
}

static MI_S32 ResetVideoDisplay(void)
{
    if (g_pstPlayStat->decode_type == SOFT_DECODING)
    {
        MI_DISP_LAYER dispLayer = DISP_LAYER;
        MI_U32 u32InputPort = DISP_INPUTPORT;
        MI_SYS_ChnPort_t stDivpChnPort;
        MI_SYS_ChnPort_t stDispChnPort;

        memset(&stDispChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDispChnPort.eModId                = E_MI_MODULE_ID_DISP;
        stDispChnPort.u32DevId              = DISP_DEV;
        stDispChnPort.u32ChnId              = 0;
        stDispChnPort.u32PortId             = DISP_INPUTPORT;

        memset(&stDivpChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stDivpChnPort.eModId                = E_MI_MODULE_ID_DIVP;
        stDivpChnPort.u32DevId              = 0;
        stDivpChnPort.u32ChnId              = DIVP_CHN;
        stDivpChnPort.u32PortId             = 0;

        MI_SYS_UnBindChnPort(&stDivpChnPort, &stDispChnPort);

        MI_DIVP_StopChn(0);
        MI_DIVP_DestroyChn(0);
        MI_GFX_Close();
    }

    MI_DISP_DisableInputPort(DISP_LAYER, DISP_INPUTPORT);

    return MI_SUCCESS;
}

MI_S32 VideoPutBufBack(void *pData)
{
    MI_S32 s32Ret;
    AVFrame *pFrame = (AVFrame *)pData;

    if (pFrame->opaque)
    {
        SS_Vdec_BufInfo *stVdecBuf = (SS_Vdec_BufInfo *)pFrame->opaque;
        //printf("frame->opaque addr : %p\n", pFrame->opaque);
        s32Ret = MI_SYS_ChnOutputPortPutBuf(stVdecBuf->stVdecHandle);
        if (MI_SUCCESS != s32Ret)
            printf("MI_SYS_ChnOutputPortPutBuf Failed!\n");
        return s32Ret;
    }

    return !MI_SUCCESS;
}

MI_S32 SetVideoRotate(MI_PHY yAddr, MI_PHY uvAddr)
{
    Surface_t srcY, dstY;
    Surface_t srcUV, dstUV;
    RECT_t r;
    srcY.eGFXcolorFmt   = E_MI_GFX_FMT_I8;
    srcY.h              = g_pstPlayStat->p_vcodec_ctx->height;
    srcY.phy_addr       = g_pstPlayStat->phy_addr;
    srcY.pitch          = g_pstPlayStat->p_vcodec_ctx->width;
    srcY.w              = g_pstPlayStat->p_vcodec_ctx->width;
    srcY.BytesPerPixel  = 1;

    dstY.eGFXcolorFmt   = E_MI_GFX_FMT_I8;
    dstY.h              = srcY.w;
    dstY.phy_addr       = yAddr;
    dstY.pitch          = ALIGN_UP(srcY.h, 16);
    dstY.w              = srcY.h;
    dstY.BytesPerPixel  = 1;
    r.left   = 0;
    r.top    = 0;
    r.bottom = srcY.h;
    r.right  = srcY.w;

    if (g_pstPlayStat->display_mode == E_MI_DISP_ROTATE_270) {
        SstarBlitCCW(&srcY, &dstY, &r);
    }
    else if (g_pstPlayStat->display_mode == E_MI_DISP_ROTATE_90) {
        SstarBlitCW(&srcY, &dstY, &r);
    }

    srcUV.eGFXcolorFmt  = E_MI_GFX_FMT_ARGB4444;
    srcUV.h             = g_pstPlayStat->p_vcodec_ctx->height / 2;
    srcUV.phy_addr      = g_pstPlayStat->phy_addr + g_pstPlayStat->p_vcodec_ctx->width * g_pstPlayStat->p_vcodec_ctx->height;
    srcUV.pitch         = g_pstPlayStat->p_vcodec_ctx->width;
    srcUV.w             = g_pstPlayStat->p_vcodec_ctx->width / 2;
    srcUV.BytesPerPixel = 2;

    dstUV.eGFXcolorFmt  = E_MI_GFX_FMT_ARGB4444;
    dstUV.h             = srcUV.w;
    dstUV.phy_addr      = uvAddr;
    dstUV.pitch         = ALIGN_UP(srcY.h, 16);
    dstUV.w             = srcUV.h;
    dstUV.BytesPerPixel = 2;
    r.left   = 0;
    r.top    = 0;
    r.bottom = srcUV.h;
    r.right  = srcUV.w;

    if (g_pstPlayStat->display_mode == E_MI_DISP_ROTATE_270) {
        SstarBlitCCW(&srcUV, &dstUV, &r);
    } else if (g_pstPlayStat->display_mode == E_MI_DISP_ROTATE_90) {
        SstarBlitCW(&srcUV, &dstUV, &r);
    }

    return MI_SUCCESS;
}

// MI display video
MI_S32 DisplayVideo(void *pData, bool bState)
{
    MI_SYS_BUF_HANDLE hHandle;
    MI_SYS_ChnPort_t stInputChnPort;
    MI_SYS_BufConf_t stBufConf;
    MI_SYS_BufInfo_t stBufInfo;

    AVFrame *pFrame = (AVFrame *)pData;

    if (g_pstPlayStat->decode_type == SOFT_DECODING)
    {
        memset(&stInputChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stInputChnPort.eModId    = E_MI_MODULE_ID_DIVP;
        stInputChnPort.u32ChnId  = 0;
        stInputChnPort.u32DevId  = 0;
        stInputChnPort.u32PortId = 0;

        memset(&stBufInfo , 0 , sizeof(MI_SYS_BufInfo_t));
        memset(&stBufConf , 0 , sizeof(MI_SYS_BufConf_t));

        stBufConf.eBufType       = E_MI_SYS_BUFDATA_FRAME;
        stBufConf.u64TargetPts   = 0;
        stBufConf.u32Flags       = MI_SYS_MAP_VA;
        stBufConf.stFrameCfg.eFormat        = E_MI_SYS_PIXEL_FRAME_YUV_SEMIPLANAR_420;
        stBufConf.stFrameCfg.eFrameScanMode = E_MI_SYS_FRAME_SCAN_MODE_PROGRESSIVE;
        stBufConf.stFrameCfg.stFrameBufExtraConf.u16BufHAlignment = 16;

        if (g_pstPlayStat->display_mode == E_MI_DISP_ROTATE_NONE) {
            stBufConf.stFrameCfg.u16Width  = pFrame->width;
            stBufConf.stFrameCfg.u16Height = pFrame->height;
        } else {
            stBufConf.stFrameCfg.u16Width  = pFrame->height;
            stBufConf.stFrameCfg.u16Height = pFrame->width;
        }

        //MI_SYS_SetChnMMAConf(E_MI_MODULE_ID_DIVP, 0, 0, (MI_U8 *)"MMU_MMA");
        if(MI_SUCCESS == MI_SYS_ChnInputPortGetBuf(&stInputChnPort, &stBufConf, &stBufInfo, &hHandle, 0))
        {
            if (g_pstPlayStat->p_frm_yuv->width * g_pstPlayStat->p_frm_yuv->height < 1024 * 600) {
                MI_SYS_FlushInvCache(g_pstPlayStat->vir_addr, g_pstPlayStat->buf_size);
            }

            if (g_pstPlayStat->display_mode == E_MI_DISP_ROTATE_NONE)
            {
                stBufInfo.stFrameData.eCompressMode = E_MI_SYS_COMPRESS_MODE_NONE;
                stBufInfo.stFrameData.eFieldType    = E_MI_SYS_FIELDTYPE_NONE;
                stBufInfo.stFrameData.eTileMode     = E_MI_SYS_FRAME_TILE_MODE_NONE;
                stBufInfo.bEndOfStream              = FALSE;

                int length = g_pstPlayStat->p_frm_yuv->width * g_pstPlayStat->p_frm_yuv->height;
                for (int index = 0; index < stBufInfo.stFrameData.u16Height; index ++)
                {
                    MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[0] + index * stBufInfo.stFrameData.u32Stride[0],
                    g_pstPlayStat->phy_addr + index * g_pstPlayStat->p_frm_yuv->width, g_pstPlayStat->p_frm_yuv->width);
                }
                for (int index = 0; index < stBufInfo.stFrameData.u16Height / 2; index ++)
                {
                    MI_SYS_MemcpyPa(stBufInfo.stFrameData.phyAddr[1] + index * stBufInfo.stFrameData.u32Stride[1],
                    g_pstPlayStat->phy_addr + length + index * g_pstPlayStat->p_frm_yuv->width, g_pstPlayStat->p_frm_yuv->width);
                }
            }
            else
            {
                SetVideoRotate(stBufInfo.stFrameData.phyAddr[0], stBufInfo.stFrameData.phyAddr[1]);
            }

            MI_SYS_ChnInputPortPutBuf(hHandle ,&stBufInfo , FALSE);
        }
        VideoPutBufBack(pData);
    }
    else
    {
        MI_SYS_ChnPort_t  stInputChnPort;
        memset(&stInputChnPort, 0, sizeof(MI_SYS_ChnPort_t));
        stInputChnPort.eModId                    = E_MI_MODULE_ID_DISP;
        stInputChnPort.u32ChnId                  = 0;
        stInputChnPort.u32DevId                  = 0;
        stInputChnPort.u32PortId                 = 0;

        SS_Vdec_BufInfo *stVdecBuf = (SS_Vdec_BufInfo *)pFrame->opaque;

        if (MI_SUCCESS != MI_SYS_ChnPortInjectBuf(stVdecBuf->stVdecHandle, &stInputChnPort)) {
            printf("MI_SYS_ChnPortInjectBuf failed!\n");
        }
    }

    return MI_SUCCESS;
}

// MI play audio
MI_S32 PlayAudio(MI_U8 *pu8AudioData, MI_U32 u32DataLen, MI_S32 *s32BusyNum)
{
    MI_S32 data_idx = 0, data_len = u32DataLen;
    MI_AUDIO_Frame_t stAoSendFrame;
    MI_S32 s32RetSendStatus = 0;
    MI_AUDIO_DEV AoDevId = AUDIO_DEV;
    MI_AO_CHN AoChn = AUDIO_CHN;
    MI_AO_ChnState_t stState;

    MI_AO_QueryChnStat(AUDIO_DEV, AoChn, &stState);
    *s32BusyNum = stState.u32ChnBusyNum + 1024;

    //read data and send to AO module
    do {
        if (data_len <= AUDIO_MAX_DATA_SIZE)
            stAoSendFrame.u32Len = data_len;
        else
            stAoSendFrame.u32Len = AUDIO_MAX_DATA_SIZE;

        stAoSendFrame.apVirAddr[0] = &pu8AudioData[data_idx];
        stAoSendFrame.apVirAddr[1] = NULL;

        data_idx += AUDIO_MAX_DATA_SIZE;
        data_len -= AUDIO_MAX_DATA_SIZE;

        //printf("PlayAudio\n");
        do{
            s32RetSendStatus = MI_AO_SendFrame(AoDevId, AoChn, &stAoSendFrame, 128);
        }while(s32RetSendStatus == MI_AO_ERR_NOBUF);

        if(s32RetSendStatus != MI_SUCCESS)
        {
            printf("[Warning]: MI_AO_SendFrame fail, error is 0x%x: \n",s32RetSendStatus);
        }
    }while(data_len > 0);

    return 0;
}

// pause audio
MI_S32 PauseAudio()
{
    MI_AO_PauseChn(AUDIO_DEV, AUDIO_CHN);
    return 0;
}

// resume audio
MI_S32 ResumeAudio()
{
    MI_AO_ResumeChn(AUDIO_DEV, AUDIO_CHN);
    return 0;
}

// duration, format, width, height, I-frame/P-frame, etc.
MI_S32 GetStreamFileInfo()
{
    return 0;
}

MI_S32 GetStreamFileDuration(long long duration)
{
	char totalTime[32];
	long long durationSec = duration / AV_TIME_BASE;

	if (durationSec / 3600 > 99)
	{
		printf("file size is limited\n");
		return -1;
	}

	memset(totalTime, 0, sizeof(totalTime));
	sprintf(totalTime, "%02lld:%02lld:%02lld", durationSec/3600, (durationSec%3600)/60, durationSec%60);
	mTextview_durationPtr->setText(totalTime);
	g_duration = duration;

	return 0;
}

MI_S32 GetStreamFilePlayPos(long long currentPos, long long frame_duration)
{
    char curTime[32];
    long long curSec = 0;
    int trackPos = 0;

    g_lastpos = currentPos;

    if (currentPos > g_duration)
    {
        printf("curPos exceed duration, curPos:%lld, duration:%lld\n", currentPos, g_duration);
        currentPos = g_duration;
    }

    // update playtime static
    if (g_firstPlayPos < 0)
        curSec = 0;
    else
    {
        long long pos = currentPos % PROGRESS_UPDATE_TIME_INTERVAL;
        //printf("pos:%lld, frame_duration:%lld, curPos:%lld, firstPos:%lld\n", pos, frame_duration, currentPos, g_firstPlayPos);
        if (pos > frame_duration/2 && pos <= (PROGRESS_UPDATE_TIME_INTERVAL - frame_duration/2))
            return 0;
    }

    curSec = currentPos / AV_TIME_BASE;

    memset(curTime, 0, sizeof(curTime));
    sprintf(curTime, "%02lld:%02lld:%02lld", curSec/3600, (curSec%3600)/60, curSec%60);
    mTextview_curtimePtr->setText(curTime);

    // update progress bar
    trackPos  = (currentPos * mSeekbar_progressPtr->getMax()) / g_duration;
    mSeekbar_progressPtr->setProgress(trackPos);

    if (g_firstPlayPos < 0)
        g_firstPlayPos = currentPos;

    return 0;
}

// stay in playing page, clear play status
MI_S32 PlayStreamFileComplete()
{
	LOGD("-----------------PlayStreamFileComplete----------------\n");
	SetPlayingStatus(false);
	mTextview_speedPtr->setText("");
	g_bShowPlayToolBar = FALSE;

	pthread_mutex_lock(&g_playFileMutex);
	g_bPlayCompleted = true;
	pthread_mutex_unlock(&g_playFileMutex);
    return 0;
}

// stay in playing page , clear play status,
MI_S32 PlayStreamFileError(int error)
{
    if (error == -101)
        mTextview_msgPtr->setText("请检查网络连接！");
    else if (error == -2)
        mTextview_msgPtr->setText("PlayStreamFileError 不支持播放720P以上的视频！");
    else if (error == -3)
        mTextview_msgPtr->setText("解码速度不够，请降低视频帧率！");
    else if (error == -4)
        mTextview_msgPtr->setText("硬件解码器卡死！");
    else
        mTextview_msgPtr->setText("Other Error Occur!");
        
    mWindow_errMsgPtr->setVisible(true);

    pthread_mutex_lock(&g_playFileMutex);
	g_bPlayError = true;
	pthread_mutex_unlock(&g_playFileMutex);

    return 0;
}

static void SetStreamPlayerControlCallBack(player_stat_t *is)
{
	is->playerController.fpGetMediaInfo = GetStreamFileInfo;
	is->playerController.fpGetDuration = GetStreamFileDuration;
	is->playerController.fpGetCurrentPlayPos = GetStreamFilePlayPos;
	is->playerController.fpGetCurrentPlayPosFromVideo = NULL;
	is->playerController.fpGetCurrentPlayPosFromAudio = NULL;
	is->playerController.fpDisplayVideo = DisplayVideo;
	is->playerController.fpVideoPutBufBack = VideoPutBufBack;
	is->playerController.fpPlayAudio = PlayAudio;
	is->playerController.fpPauseAudio = PauseAudio;
	is->playerController.fpResumeAudio = ResumeAudio;
	is->playerController.fpPlayComplete = PlayStreamFileComplete;
	is->playerController.fpPlayError = PlayStreamFileError;
	is->playerController.fpSetVideoDisplay = SetVideoDisplay;
	is->playerController.fpResetVideoDisplay = ResetVideoDisplay;
}
#endif

int GetImageFileDuration(long long duration)
{
	char totalTime[32] = {0};
	long long durationSec = 0;
	LOGD(" GetImageFileDuration %lld\n", duration);
	g_duration = duration;
	durationSec = g_duration / AV_TIME_BASE;
	sprintf(totalTime, "%02lld:%02lld:%02lld", durationSec/3600, (durationSec%3600)/60, durationSec%60);
//	mTextview_durationPtr->setText(totalTime);
//	mTextview_curtimePtr->setText("00:00:00");
//	mSeekbar_progressPtr->setProgress(0);

	return 0;
}

int GetImageFilePlayPos(long long currentPos)
{
//	char curTime[32];
//	long long curSec = 0;
//	int trackPos = 0;
//
//	printf("Enter GetImageFilePlayPos\n");
//	printf("currentPos is %lld\n", currentPos);
//	curSec = currentPos / AV_TIME_BASE;
//	memset(curTime, 0, sizeof(curTime));
//	sprintf(curTime, "%02lld:%02lld:%02lld", curSec/3600, (curSec%3600)/60, curSec%60);
//	printf("image curTime: %s\n", curTime);
//	printf("Leave GetImageFilePlayPos\n");

	return 0;
}

MI_S32 PlayImageFileComplete()
{
	printf("Enter PlayImageFileComplete\n");
	if(!isSplitDisplay){
		mTextViewPicPtr->setVisible(false);
	}

	pthread_mutex_lock(&g_playFileMutex);
	g_bPlayCompleted = true;
	pthread_mutex_unlock(&g_playFileMutex);
	printf("Leave PlayImageFileComplete\n");

	return 0;
}

static void ResetSpeedMode()
{
    g_ePlayDirection = E_PLAY_FORWARD;
    g_ePlayMode = E_PLAY_NORMAL_MODE;
    g_eSpeedMode = E_NORMAL_SPEED;
    g_u32SpeedNumerator = 1;
    g_u32SpeedDenomonator = 1;
}

void DetectUsbHotplug(UsbParam_t *pstUsbParam)		// action 0, connect; action 1, disconnect
{
	if (!pstUsbParam->action)
	{
		g_bPlaying = FALSE;
		g_bPause = FALSE;

		g_bPlayFileThreadExit = true;
		if (g_playFileThread)
		{
			pthread_join(g_playFileThread, NULL);
			g_playFileThread = NULL;
		}

		ResetSpeedMode();
		g_bShowPlayToolBar = FALSE;

//		EASYUICONTEXT->goHome();
	}
}

#define USE_POPEN       1
#define PANT_TIME       5
FILE *player_fd = NULL;
extern int errno;

static void StartPlayStreamFile(char *pFileName,int carousel)
{
    printf("Start to StartPlayStreamFile\n");
#ifdef SUPPORT_PLAYER_PROCESS
    int ret;
    struct timeval time_start, time_wait;
    void *shm = NULL;

    ResetSpeedMode();

    memset(&recvevt, 0, sizeof(IPCEvent));

    if(!i_server.Init()) {
        printf("[%s %d]create i_server fail!\n", __FILE__, __LINE__);
        fprintf(stderr, "Error：%s\n", strerror(errno));
        return;
    }

    #if USE_POPEN
    //创建共享内存
    shm_id = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
    if(shm_id < 0) {
        fprintf(stderr, "shmget failed\n");
        goto next;
    }

    //将共享内存连接到当前进程的地址空间
    shm = shmat(shm_id, (void*)NULL, 0);
    if(shm < 0) {
        fprintf(stderr, "shmat failed\n");
        goto next;
    }

    shm_addr = (struct shared_use_st *)shm;
    memset(shm_addr, 0x0, sizeof(struct shared_use_st));
    printf("shared memory attached at %x\n", (int)shm);

    player_fd = popen(MYPLAYER_PATH, "w");
    if (NULL == player_fd) {
        printf("my_player is not exit!\n");
        goto next;
    }
    printf("popen myplayer progress done!\n");

    gettimeofday(&time_start, NULL);
    while (i_server.Read(recvevt) <= 0
           || (recvevt.EventType != IPC_COMMAND_CREATE)) {
        usleep(10 * 1000);
        gettimeofday(&time_wait, NULL);
        if (time_wait.tv_sec - time_start.tv_sec > 2) {
            printf("myplayer progress create failed!\n");
            break;
        }
    }
next:
    if (recvevt.EventType == IPC_COMMAND_CREATE) {
        printf("myplayer progress create success!\n");
    } else {
        if (shm_addr) {
            //把共享内存从当前进程中分离
            ret = shmdt((void *)shm_addr);
            if (ret < 0) {
                fprintf(stderr, "shmdt failed\n");
            }

            //删除共享内存
            ret = shmctl(shm_id, IPC_RMID, NULL);
            if(ret < 0) {
                fprintf(stderr, "shmctl(IPC_RMID) failed\n");
            }
        }
        shm_addr = NULL;
        shm_id = 0;

        LOGD("ERROR --- Other Error Occur!");

        pthread_mutex_lock(&g_playFileMutex);
        g_bPlayError = true;
        pthread_mutex_unlock(&g_playFileMutex);

        return;
    }
    #endif

    if(!o_client.Init()) {
        printf("[%s %d]my_player process not start!\n", __FILE__, __LINE__);
        fprintf(stderr, "Error：%s\n", strerror(errno));
        return;
    }

    memset(&sendevt, 0, sizeof(IPCEvent));
    sendevt.EventType = IPC_COMMAND_OPEN;
    strcpy(sendevt.stPlData.filePath, pFileName);
    printf("list file name to play = %s, %s\n", sendevt.stPlData.filePath, pFileName);

    // 旋转开关
    #if ENABLE_ROTATE
    sendevt.stPlData.rotate = E_MI_DISP_ROTATE_270;
	#elif ENABLE_ROTATE_180
    sendevt.stPlData.rotate = E_MI_DISP_ROTATE_180;
    #else
    sendevt.stPlData.rotate = E_MI_DISP_ROTATE_NONE;
    #endif

	//honestar karl.hong add 20210826
	if(GetRotateSetting() == 90){
		sendevt.stPlData.rotate = E_MI_DISP_ROTATE_270;
	}else if(GetRotateSetting() == 180){
		sendevt.stPlData.rotate = E_MI_DISP_ROTATE_180;
	}else if(GetRotateSetting() == 270){
		sendevt.stPlData.rotate = E_MI_DISP_ROTATE_270;
	}else{
		sendevt.stPlData.rotate = E_MI_DISP_ROTATE_NONE;
	}
	//honestar karl.hong add 20210826

	
    sendevt.stPlData.x = 0;
    sendevt.stPlData.y = 0;
    sendevt.stPlData.width  = g_playViewWidth;
    sendevt.stPlData.height = g_playViewHeight;
    sendevt.stPlData.aodev = AUDIO_DEV;
    sendevt.stPlData.audio_only = false;
    sendevt.stPlData.video_only = false;
    sendevt.stPlData.play_mode  = carousel;    // 0: 单次播放,1: 循环播放(seek to start)
    o_client.Send(sendevt);
    printf("try to open file: %s\n", pFileName);

    memset(&recvevt, 0, sizeof(IPCEvent));
    gettimeofday(&time_start, NULL);
    while (i_server.Read(recvevt) <= 0
           || ((recvevt.EventType != IPC_COMMAND_ACK)
           && (recvevt.EventType != IPC_COMMAND_ERROR))) {
        usleep(10 * 1000);
        gettimeofday(&time_wait, NULL);
        if (time_wait.tv_sec - time_start.tv_sec > 10) {
        	printf("--------Read timeout----------\n");
            memset(&sendevt, 0, sizeof(IPCEvent));
            #if USE_POPEN
            sendevt.EventType = IPC_COMMAND_EXIT;
            #else
            sendevt.EventType = IPC_COMMAND_CLOSE;
            #endif
            o_client.Send(sendevt);
            //修复超时后视频播放停止
        	// pthread_mutex_lock(&g_playFileMutex);
        	// g_bPlayCompleted = true;
        	// pthread_mutex_unlock(&g_playFileMutex);
        	//修复超时后视频播放停止
            (*((user_player_callback)userCallback))(NEED_CONTINUE,"StartPlayStreamFile timeout",recvevt.stPlData.status);
            break;
        }
    }
    if (recvevt.EventType == IPC_COMMAND_ACK) {
        printf("receive ack from my_player!\n");

        memset(&sendevt, 0, sizeof(IPCEvent));
        sendevt.EventType = IPC_COMMAND_GET_DURATION;
        o_client.Send(sendevt);
    } else if(recvevt.EventType == IPC_COMMAND_ERROR) {
        if (recvevt.stPlData.status == -101)
        	LOGD("ERROR --- 请检查网络连接！\n");
        else if (recvevt.stPlData.status == -2)
        	LOGD("ERROR --- StartPlayStreamFile 不支持播放720P以上的视频！\n");
        else if (recvevt.stPlData.status == -3)
        	LOGD("ERROR --- 解码速度不够，请降低视频帧率！\n");
        else if (recvevt.stPlData.status == -4)
        	LOGD("ERROR --- 读取网络超时！\n");
        else
        	LOGD("ERROR --- Other Error Occur！\n");
        (*((user_player_callback)userCallback))(NEED_CONTINUE,"StartPlayStreamFile",recvevt.stPlData.status);
        pthread_mutex_lock(&g_playFileMutex);
        g_bPlayError = true;
        pthread_mutex_unlock(&g_playFileMutex);
    }
    MEDIAPLAYER->setPlayerVol(g_s32VolValue);
//    StartPlayAudio();
#else
    // ffmpeg_player初始化 & ui初始化
    mWindow_errMsgPtr->setVisible(false);
    // init player
    ResetSpeedMode();
    StartPlayVideo();
    StartPlayAudio();

    g_pstPlayStat = player_init(pFileName);
    if (!g_pstPlayStat)
    {
        StopPlayAudio();
        StopPlayVideo();
        printf("Initilize player failed!\n");
        return;
    }
    // 旋转开关
#if ENABLE_ROTATE
    g_pstPlayStat->display_mode = E_MI_DISP_ROTATE_270;
#elif ENABLE_ROTATE_180
    g_pstPlayStat->display_mode = E_MI_DISP_ROTATE_180;
#else
    g_pstPlayStat->display_mode = E_MI_DISP_ROTATE_NONE;
#endif

	//honestar karl.hong add 20210826
	if(GetRotateSetting() == 90){
		g_pstPlayStat->display_mode = E_MI_DISP_ROTATE_270;
	}else if(GetRotateSetting() == 180){
		g_pstPlayStat->display_mode = E_MI_DISP_ROTATE_180;
	}else if(GetRotateSetting() == 270){
		g_pstPlayStat->display_mode = E_MI_DISP_ROTATE_270;
	}else{
		g_pstPlayStat->display_mode = E_MI_DISP_ROTATE_NONE;
	}	
	//honestar karl.hong add 20210826
	
    // 设置视频显示位置与窗口
    g_pstPlayStat->pos_x = 0;
    g_pstPlayStat->pos_y = 0;
    g_pstPlayStat->in_width  = g_playViewWidth;
    g_pstPlayStat->in_height = g_playViewHeight;
    printf("video file name is : %s, panel w/h = [%d %d]\n", g_pstPlayStat->filename, g_playViewWidth, g_playViewHeight);

    SetStreamPlayerControlCallBack(g_pstPlayStat);
    printf("open_demux\n");
    open_demux(g_pstPlayStat);
    printf("open_video\n");
    open_video(g_pstPlayStat);
    printf("open_audio\n");
    open_audio(g_pstPlayStat);
//   SetPlayerVolumn(g_s32VolValue);
    MEDIAPLAYER->setPlayerVol(g_s32VolValue);
//    StartPlayAudio();
#endif
    printf("End to StartPlayStreamFile\n");
}

static void StopPlayStreamFile()
{
    printf("Start to StopPlayStreamFile\n");
#ifdef SUPPORT_PLAYER_PROCESS
    int ret;
    struct timeval time_start, time_wait;

    if(o_client.Init()) {
    	printf("Start to StopPlayStreamFile1\n");
        memset(&sendevt, 0, sizeof(IPCEvent));
        #if USE_POPEN
        sendevt.EventType = IPC_COMMAND_EXIT;
        o_client.Send(sendevt);
        printf("Start to StopPlayStreamFile2\n");
        memset(&recvevt, 0, sizeof(IPCEvent));
        gettimeofday(&time_start, NULL);
        printf("Start to StopPlayStreamFile3\n");
        while ((i_server.Read(recvevt) <= 0 || recvevt.EventType != IPC_COMMAND_DESTORY) &&
               (shm_addr && (shm_addr->written || !shm_addr->flag))) {
            usleep(10 * 1000);
            gettimeofday(&time_wait, NULL);
            if (time_wait.tv_sec - time_start.tv_sec > 2) {
                printf("myplayer progress destory failed!\n");
                break;
            }
        }
        printf("Start to StopPlayStreamFile4\n");
        #else
        sendevt.EventType = IPC_COMMAND_CLOSE;
        o_client.Send(sendevt);

        memset(&recvevt, 0, sizeof(IPCEvent));
        gettimeofday(&time_start, NULL);
        printf("Start to StopPlayStreamFile5\n");
        while (i_server.Read(recvevt) <= 0 || recvevt.EventType != IPC_COMMAND_ACK) {
            usleep(10 * 1000);
            gettimeofday(&time_wait, NULL);
            if (time_wait.tv_sec - time_start.tv_sec > 2) {
                printf("myplayer progress close failed!\n");
                break;
            }
        }
        #endif
    } else {
        printf("my_player is not start!\n");
        fprintf(stderr, "Error：%s\n", strerror(errno));
    }

    #if USE_POPEN
    if ((shm_addr && shm_addr->flag) || recvevt.EventType == IPC_COMMAND_DESTORY) {
        printf("myplayer progress destory done!\n");
    }

    if (shm_addr) {
        //把共享内存从当前进程中分离
        ret = shmdt((void *)shm_addr);
        if (ret < 0) {
            fprintf(stderr, "shmdt failed\n");
        }

        //删除共享内存
        ret = shmctl(shm_id, IPC_RMID, NULL);
        if(ret < 0) {
            fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        }
    }
    shm_addr = NULL;
    shm_id = 0;

    if (player_fd) {
        pclose(player_fd);
        player_fd = NULL;
    }

    i_server.Term();
    o_client.Term();
    system("rm -rf /appconfigs/server_input");
    printf("remove server_input file\n");
    #endif

    g_bPlaying = false;
    g_bPause = false;

    ResetSpeedMode();
//    SetPlayingStatus(false);
//    mTextview_speedPtr->setText("");
//    mTextview_curtimePtr->setText("00:00:00");
//    mSeekbar_progressPtr->setProgress(0);
    g_firstPlayPos = PLAY_INIT_POS;
#else
    // ffmpeg_player反初始化 & ui反初始化
    g_bPlaying = false;
    g_bPause = false;
    StopPlayVideo();
    player_deinit(g_pstPlayStat);
    StopPlayAudio();
    ResetSpeedMode();

    SetPlayingStatus(false);
    mTextview_speedPtr->setText("");
    mTextview_curtimePtr->setText("00:00:00");
    mSeekbar_progressPtr->setProgress(0);

    // reset pts
    g_firstPlayPos = PLAY_INIT_POS;
#endif
    printf("End of StopPlayStreamFile\n");
}

static void TogglePlayStreamFile()
{
#ifdef SUPPORT_PLAYER_PROCESS
    if(!o_client.Init()) {
        printf("my_player is not start!\n");
        return;
    }

    memset(&sendevt, 0, sizeof(IPCEvent));
	if (!g_bPause) {
		sendevt.EventType = IPC_COMMAND_RESUME;
	} else {
		sendevt.EventType = IPC_COMMAND_PAUSE;
	}
    o_client.Send(sendevt);
#else
	toggle_pause(g_pstPlayStat);
#endif
}

/*
*图片播放未用
*/
static void StartDisplayImage(char *pFileName)
{
	ImagePlayerCtrl_t stPlayerCtrl;

	printf("Enter StartDisplayImage, fileName is %s\n", pFileName);
	stPlayerCtrl.fpGetDuration = GetImageFileDuration;
	stPlayerCtrl.fpGetCurrentPlayPos = GetImageFilePlayPos;
	stPlayerCtrl.fpDisplayComplete = PlayImageFileComplete;
	stPlayerCtrl.time = curDisplayImageTime;
	StartPlayAudio();

	g_pstImagePlayer = ImagePlayer_Init(&stPlayerCtrl);
	if (!g_pstImagePlayer)
	{
		StopPlayAudio();
		printf("ImagePlayer_Init exec failed\n");
		return;
	}

	int imgWidth = 0;
	int imgHeight = 0;
	int colorBits = 0;
	unsigned char *data = stbi_load(pFileName, &imgWidth, &imgHeight, &colorBits, 0);
	printf("img width=%d, height=%d, colorbits=%d\n", imgWidth, imgHeight, colorBits);
	stbi_image_free(data);

	LayoutPosition imgPosition = mTextViewPicPtr->getPosition();
	printf("imgPosition.mWidth=%d, imgPosition.mHeight=%d\n", imgPosition.mWidth , imgPosition.mHeight);
//	imgPosition.mLeft = imgPosition.mLeft + (imgPosition.mWidth - imgWidth) / 2;
//	imgPosition.mTop = imgPosition.mTop + (imgPosition.mHeight - imgHeight) / 2;
//	imgPosition.mWidth = 1920;
//	imgPosition.mHeight = 1080;
	mTextViewPicPtr->setPosition(imgPosition);
	mTextViewPicPtr->setBackgroundPic(pFileName);

	mTextViewPicPtr->setVisible(true);
	printf("Leave StartDisplayImage\n");
}

static void StopDisplayImage()
{
	printf("Enter StopDisplayImage\n");
	if(g_pstImagePlayer != NULL){
		ImagePlayer_Deinit(g_pstImagePlayer);
	}
	StopPlayAudio();

	if(!isSplitDisplay){
		mTextViewPicPtr->setVisible(false);
	}
	printf("Leave StopDisplayImage\n");
}

static void TogglePlayImageFile()
{
	ImagePlayer_TogglePause(g_pstImagePlayer);
}

static void StopPlayFile()
{
	if (g_playStream)
		StopPlayStreamFile();
	else
		StopDisplayImage();
}

static void StartPlayFile(char *pFileName,int carousel)
{
	g_bPlayCompleted = false;
	//g_playStream = 1;//g_playStream = IsMediaStreamFile(pFileName);
	LOGD("---------StartPlayFile---------%s\n", g_playStream);
	// if (g_playStream)
	// 	StartPlayStreamFile(pFileName);
	// else
	// 	StartDisplayImage(pFileName);
    static int flag = 0;
    while (flag)
    {
        sleep(3);
    }
    flag = 1;
	StartPlayStreamFile(pFileName,carousel);

	g_bPlaying = TRUE;
	g_bPause   = FALSE;
    flag = 0;
//	char filePath[256];
//	char *p = NULL;
//	memset(filePath, 0, sizeof(filePath));
//	strcpy(filePath, pFileName);
//	p = strrchr(filePath, '/');
//	*p = 0;
//	mTextview_fileNamePtr->setText(pFileName+strlen(filePath)+1);
//	SetPlayingStatus(true);
	//AutoDisplayToolbar();
}

static void TogglePlayFile()
{
	if (g_bPlaying)
	{
		g_bPause = !g_bPause;
//		SetPlayingStatus(!g_bPause);

		if (g_playStream)
			TogglePlayStreamFile();
		else
			TogglePlayImageFile();
	}
}

RepeatMode_e operator++(RepeatMode_e& cmd)
{
	RepeatMode_e t = cmd;
	cmd = static_cast<RepeatMode_e>(cmd+1);
	return t;
}

/*
*图片播放未使用
*/
static void *PlayPicProc(void){
	while(!g_bPlayFileThreadExit){
		JsonParse::MediaResource* resource = MEDIAMANAGER->getMediaResourcesForPlay();
		LOGD("PlayPicProc - %d\n", resource);
		if(resource && MEDIAMANAGER->idPicMediaType(*resource) && resource->srcPicArray.size() > 0){
			//加载图片播放
			LOGD("播放图片\n");
			for(Json::ArrayIndex i = 0; i < resource->srcPicArray.size(); i++){
				string url = resource->srcPicArray[i].asString();
				string path = MEDIAMANAGER->getMediaPath(url);
				LOGD("播放图片 -%s\n", path.c_str());

//				curDisplayImageTime = resource->interval;
//				StartDisplayImage(path.c_str());

				int imgWidth = 0;
				int imgHeight = 0;
				int colorBits = 0;
				unsigned char *data = stbi_load(path.c_str(), &imgWidth, &imgHeight, &colorBits, 0);
				printf("img width=%d, height=%d, colorbits=%d\n", imgWidth, imgHeight, colorBits);
				stbi_image_free(data);

				mTextViewPicPtr->setBackgroundPic(path.c_str());

				sleep(resource->interval);
			}
		}else{
			sleep(15);
		}
	}
	return NULL;
}

static void *PlayFileProc(void *callback)
{
	printf("get in PlayFileProc!-\n");
    userCallback = callback;
	bool bPlayCompleted = false;
	bool bPlayError = false;
	struct timeval pant_start, pant_wait;

    gettimeofday(&pant_start, NULL);

	while (!g_bPlayFileThreadExit)
	{
		pthread_mutex_lock(&g_playFileMutex);
		if (bPlayError != g_bPlayError)
		{
			bPlayError = g_bPlayError;
			g_bPlayError = false;
		}
		if (bPlayCompleted != g_bPlayCompleted)
		{
			bPlayCompleted = g_bPlayCompleted;
			g_bPlayCompleted = false;
		}
		pthread_mutex_unlock(&g_playFileMutex);

		if (bPlayError)
		{
			printf("occur error when playing file\n");
			//break;
		}
        if (i_server.Read(recvevt) > 0) {
            switch (recvevt.EventType){
                case IPC_COMMAND_COMPLETE : {
//                        SetPlayingStatus(false);
//                        mTextview_speedPtr->setText("");
                    g_bShowPlayToolBar = FALSE;
                    g_bPantStatus = false;
                    LOGD("IPC_COMMAND_COMPLETE\n");
                    pthread_mutex_lock(&g_playFileMutex);
                    g_bPlayCompleted = true;
                    pthread_mutex_unlock(&g_playFileMutex);
                    if (callback!=NULL)
                    {
                        (*((user_player_callback)callback))(NEED_CONTINUE,"PlayFileProc IPC_COMMAND_COMPLETE",recvevt.stPlData.status);
                    }
                    // printf("[%s] xxxxxxxxxxxxxxxxxxxxxxxxplay complete! \n", user_get_play_filePath());
                }
                break;
                case IPC_COMMAND_ERROR : {
                    if (recvevt.stPlData.status == -101)
                        LOGD("ERROR --- 请检查网络连接！\n");
                    else if (recvevt.stPlData.status == -2)
                        LOGD("ERROR ---PlayFileProc 不支持播放720P以上的视频...！\n");
                    else if (recvevt.stPlData.status == -3)
                        LOGD("ERROR --- 解码速度不够，请降低视频帧率！\n");
                    else if (recvevt.stPlData.status == -4)
                        LOGD("ERROR --- 读取网络超时！\n");
                    else
                        LOGD("ERROR --- Other Error Occur！- %d\n", recvevt.stPlData.status);

                    pthread_mutex_lock(&g_playFileMutex);
                    g_bPlayError = true;
                    pthread_mutex_unlock(&g_playFileMutex);
//                      printf("[%s] play error!\n", curFileName);
                    if (callback!=NULL)
                    {
                        (*((user_player_callback)callback))(NEED_CONTINUE,"PlayFileProc IPC_COMMAND_ERROR",recvevt.stPlData.status);
                    }
                    
                }
                break;
                case IPC_COMMAND_DESTORY:
                    if (callback!=NULL)
                    {
                        (*((user_player_callback)callback))(NEED_CONTINUE,"PlayFileProc IPC_COMMAND_DESTORY",recvevt.stPlData.status);
                    }  
                break;
                default:
                    (*((user_player_callback)callback))(NEED_IGNORE,"PlayFileProc default",recvevt.EventType);//这个
                break;         
            }
        }
		//LOGD("bPlayCompleted===================[%d]\n", bPlayCompleted);
        
// 		if (bPlayCompleted)
// 		{
// 		    JsonParse::MediaResource* resource = MEDIAMANAGER->getMediaResourcesForPlay();
// 		    while(!resource){
// 		    	resource = MEDIAMANAGER->getMediaResourcesForPlay();
// 		    	LOGD("playThread - %d\n", resource);
// 		    	sleep(5);
// 		    }
// 			if(resource && MEDIAMANAGER->idPicMediaType(*resource) && (resource->srcPicArray.size() > 0 || resource->filePath.size() > 0) && !isSplitDisplay){
// 				//加载图片播放
// 				LOGD("-播放图片\n");
// 				StopPlayFile();
// 				if(resource->srcPicArray.size() > 0){
// 					for(Json::ArrayIndex i = 0; i < resource->srcPicArray.size(); i++){
// 						string url = resource->srcPicArray[i].asString();
// 						string path = MEDIAMANAGER->getMediaPath(url);
// 						LOGD("-播放图片 -%s\n", path.c_str());

// 						g_playStream = 0;
// 						curDisplayImageTime = resource->interval;
// 						StartPlayFile(path.c_str());
// 						sleep(resource->interval);

// 					}
// 				}else if(resource->filePath.size() > 0){
// 					g_playStream = 0;
// 					curDisplayImageTime = 10;
// 					StartPlayFile(resource->filePath.c_str());
// 					sleep(10);
// 				}

// 			}else if(resource && (resource->mSrcUrl.size() > 0  || resource->filePath.size() > 0)){
// 				string path = resource->filePath.length() < 0 ? MEDIAMANAGER->getMediaPath(resource->mSrcUrl) : resource->filePath;
// 				if(path.length() > 0){
// 					//播放视频
// 					LOGD("播放视频 -%s - %d\n", path.c_str(), g_playStream);

// 					StopPlayFile();
// 					g_playStream = 1;
// 					StartPlayFile(path.c_str());
// 				}
// 			}else{
// //				StopPlayFile();
// //				g_playStream = 1;
// //				StartPlayFile("/vendor/udisk_sda/testVideo.mp4");
// 				LOGD("------bPlayCompleted---%d\n", bPlayCompleted);
//                 pthread_mutex_lock(&g_playFileMutex);
//                 g_bPlayCompleted = true;
//                 pthread_mutex_unlock(&g_playFileMutex);
// 			}
// 		}
#ifdef SUPPORT_PLAYER_PROCESS
        if (g_playStream) {
            memset(&recvevt, 0, sizeof(IPCEvent));
            if (i_server.Read(recvevt) > 0) {
                switch (recvevt.EventType)
                {
                    case IPC_COMMAND_GET_DURATION : {
                        char totalTime[32];
                        long int durationSec = recvevt.stPlData.misc / 1.0;

                        if (durationSec / 3600 < 99) {
                            memset(totalTime, 0, sizeof(totalTime));
                            sprintf(totalTime, "%02d:%02d:%02d", durationSec/3600, (durationSec%3600)/60, durationSec%60);
//                            mTextview_durationPtr->setText(totalTime);
                            g_duration = durationSec;
                            printf("file duration time = %lld\n", g_duration);
                        }
                    }
                    break;

                    case IPC_COMMAND_GET_POSITION : {
                        char curTime[32];
                        int curSec = recvevt.stPlData.misc / 1.0;
                        int trackPos;
                        //printf("get video current position time = %d\n", curSec);
                        memset(curTime, 0, sizeof(curTime));
                        sprintf(curTime, "%02d:%02d:%02d", curSec/3600, (curSec%3600)/60, curSec%60);
//                        mTextview_curtimePtr->setText(curTime);

//                        trackPos  = (curSec * mSeekbar_progressPtr->getMax()) / g_duration;
//                        mSeekbar_progressPtr->setProgress(trackPos);
                    }
                    break;

                    case IPC_COMMAND_ERROR : {
                        if (recvevt.stPlData.status == -101)
                        	LOGD("ERROR --- 请检查网络连接！\n");
                        else if (recvevt.stPlData.status == -2)
                        	LOGD("ERROR --- 不支持播放720P以上的视频！xxx\n");
                        else if (recvevt.stPlData.status == -3)
                        	LOGD("ERROR --- 解码速度不够，请降低视频帧率！\n");
                        else if (recvevt.stPlData.status == -4)
                        	LOGD("ERROR --- 读取网络超时！\n");
                        else
                        	LOGD("ERROR --- Other Error Occur！- %d\n", recvevt.stPlData.status);

                        pthread_mutex_lock(&g_playFileMutex);
                        g_bPlayError = true;
                        pthread_mutex_unlock(&g_playFileMutex);
//                      printf("[%s] play error!\n", curFileName);
                    }
                    break;

                    case IPC_COMMAND_COMPLETE : {
//                        SetPlayingStatus(false);
//                        mTextview_speedPtr->setText("");
                        g_bShowPlayToolBar = FALSE;
                        g_bPantStatus = false;
                        LOGD("IPC_COMMAND_COMPLETE\n");
                        pthread_mutex_lock(&g_playFileMutex);
                        g_bPlayCompleted = true;
                        pthread_mutex_unlock(&g_playFileMutex);
                        // printf("[%s] xxxxxxxxxxxxxxxxxxxxxxxxplay complete!\n", "curFileName");
                    }
                    break;

                    case IPC_COMMAND_PANT : {
                        g_bPantStatus = true;
                        gettimeofday(&pant_start, NULL);
                        if(!o_client.Init()) {
                            printf("[%s %d]my_player process not start!\n", __FILE__, __LINE__);
                            fprintf(stderr, "Error：%s\n", strerror(errno));
                        } else {
                            memset(&sendevt, 0, sizeof(IPCEvent));
                            sendevt.EventType = IPC_COMMAND_PANT;
                            o_client.Send(sendevt);
                        }
                    }
                    break;

                    default : break;
                }
            }

            //心跳包判断
            gettimeofday(&pant_wait, NULL);
            if (pant_wait.tv_sec - pant_start.tv_sec > 2 * PANT_TIME && g_bPantStatus) {
                LOGD("ERROR ---- Other Error Occur!\n");
                pthread_mutex_lock(&g_playFileMutex);
                g_bPlayError = true;
                pthread_mutex_unlock(&g_playFileMutex);
                printf("myplayer has exit abnormallity!\n");
            }
        }
#endif
        usleep(200 * 1000);//200ms
    }

    StopPlayFile();
//    g_fileName = curFileName;
    g_bPantStatus = false;
    printf("### PlayFileProc Exit ###\n");
    return NULL;
}
#endif


void MediaPlayer::initPlayer(ZKVideoView* mVideoview_videoPtr, ZKTextView* mTextView_PicPtr){
#ifdef SUPPORT_PLAYER_MODULE
	mTextViewPicPtr = mTextView_PicPtr;
    // init play view real size
    LayoutPosition layoutPos = mVideoview_videoPtr->getPosition();

	#if ENABLE_ROTATE
		g_playViewHeight = layoutPos.mWidth * PANEL_MAX_WIDTH / UI_MAX_WIDTH;
		g_playViewWidth = ALIGN_DOWN(layoutPos.mHeight * PANEL_MAX_HEIGHT / UI_MAX_HEIGHT, 2);
	#else
		g_playViewWidth = layoutPos.mWidth * PANEL_MAX_WIDTH / UI_MAX_WIDTH;
		g_playViewHeight = ALIGN_DOWN(layoutPos.mHeight * PANEL_MAX_HEIGHT / UI_MAX_HEIGHT, 2);
	#endif
	//honestar karl.hong add 20210826
	if((GetRotateSetting() == 90) || (GetRotateSetting() == 270))
    {
		g_playViewHeight = layoutPos.mWidth * GetPanelHeigth() / GetFbHeight();
		g_playViewWidth = ALIGN_DOWN(layoutPos.mHeight * GetPanelWidth() / GetFbWidth(), 2);
	}
    else if(1 == Get1280x1024SpecialDemand())
    {
        g_playViewWidth = layoutPos.mWidth * GetPanelWidth() / GetFbWidth();
		g_playViewHeight = ALIGN_DOWN(layoutPos.mHeight * GetPanelWidth() / GetFbWidth(), 2);
    }
    else
    {
		g_playViewWidth = layoutPos.mWidth * GetPanelWidth() / GetFbWidth();
		g_playViewHeight = ALIGN_DOWN(layoutPos.mHeight * GetPanelHeigth() / GetFbHeight(), 2);
	}
	//honestar karl.hong add 20210826
    printf("play view size: w=%d, h=%d\n", g_playViewWidth, g_playViewHeight);

    //SSTAR_RegisterUsbListener(DetectUsbHotplug);
    // init pts
    g_firstPlayPos = PLAY_INIT_POS;

    // divp use window max width & height default, when play file, the inputAttr of divp will be set refer to file size.
    CreatePlayerDev();

    pthread_mutex_init(&g_playFileMutex, NULL);		// playFile mutex init
#endif
}

extern int register_user_thread(void *p_thread, void *function_callback, void *param);
void MediaPlayer::showPlayer(bool isSplit,user_player_callback function_callback){
#ifdef SUPPORT_PLAYER_MODULE
	int defVol = StoragePreferences::getInt(SPKEY_DEF_VOL, -1);
	if(defVol > 0){
		g_s32VolValue = defVol;
	}
    g_bPlayFileThreadExit = false;
    pthread_create(&g_playFileThread, NULL, PlayFileProc, function_callback);


    // isSplitDisplay = isSplit;
    // if(isSplitDisplay){
    	// pthread_create(&g_playPicThread, NULL, PlayPicProc, NULL);
    // }

#endif
}

void MediaPlayer::quitPlayer(){
#ifdef SUPPORT_PLAYER_MODULE
    pthread_mutex_destroy(&g_playFileMutex);
    DestroyPlayerDev();
    g_firstPlayPos = PLAY_INIT_POS;

    printf("start to UnRegisterUsbListener\n");
    SSTAR_UnRegisterUsbListener(DetectUsbHotplug);
    printf("end of UnRegisterUsbListener\n");
#endif
}

void MediaPlayer::setMute(){
#ifdef SUPPORT_PLAYER_MODULE
	#ifdef SUPPORT_PLAYER_PROCESS
    if(!o_client.Init()) {
        printf("my_player is not start!\n");
        return;
    }
	g_bMute = !g_bMute;
	memset(&sendevt, 0, sizeof(IPCEvent));
	sendevt.EventType = IPC_COMMAND_SET_MUTE;
	sendevt.stPlData.mute = g_bMute;
	o_client.Send(sendevt);
	printf("set mute to %d\n", g_bMute);
	#else
	g_bMute = !g_bMute;
	MI_AO_SetMute(AUDIO_DEV, g_bMute);
	printf("set mute to %d\n", g_bMute);
	#endif
#endif
}

void MediaPlayer::setPlayerVol(int newVol){
	printf("set voice newVol = [%d]\n", newVol);
	g_s32VolValue = newVol;
#ifdef SUPPORT_PLAYER_MODULE
	#ifdef SUPPORT_PLAYER_PROCESS
    if(!o_client.Init()) {
        printf("my_player is not start!\n");
        return;
    }
	memset(&sendevt, 0, sizeof(IPCEvent));
	sendevt.EventType = IPC_COMMAND_SET_VOLUMN;
	sendevt.stPlData.volumn = newVol * 0.14;
	g_s32VolValue = newVol;
	printf("set voice volumn = [%d]\n", sendevt.stPlData.volumn);
	o_client.Send(sendevt);
	#else
	MI_S32 vol = 0;
	MI_AO_ChnState_t stAoState;

	printf("voice changed!!!!!!!!\n");
	g_s32VolValue = newVol;//GetPlayerVolumn();
	if (g_s32VolValue)
	{
		//vol = g_s32VolValue * (MAX_ADJUST_AO_VOLUME - MIN_ADJUST_AO_VOLUME) / 100 + MIN_ADJUST_AO_VOLUME;
		s32SetVolumeDb = 0.14 * g_s32VolValue * (MAX_ADJUST_AO_VOLUME-MIN_ADJUST_AO_VOLUME) / 100 + MIN_ADJUST_AO_VOLUME;
		g_bMute = false;
	}
	else
	{
		vol = MIN_AO_VOLUME;
		g_bMute = true;
	}

	memset(&stAoState, 0, sizeof(MI_AO_ChnState_t));
	if (MI_SUCCESS == MI_AO_QueryChnStat(AUDIO_DEV, AUDIO_CHN, &stAoState))
	{
		MI_AO_SetVolume(AUDIO_DEV, vol);
		MI_AO_SetMute(AUDIO_DEV, g_bMute);
	}
#endif

#endif

}

MediaPlayer::MediaPlayer(){

}

MediaPlayer* MediaPlayer::getInstance(){
	static MediaPlayer mInstance;
	return &mInstance;
}


////////////////////////add it by adien//////////////////////
// static char playFilePath[100] = {0};
// const char *user_get_play_filePath(void)
// {
//    return playFilePath;
// }
#include "main_app/hardware_tool/user_gpio.h"
void user_stop_play_video(void)
{
    audio_close_control();
    usleep(200*1000);
    StopPlayStreamFile();
    printf("StopPlayStreamFile...................\n");
}

void user_start_play_video(char *videoPath,int carousel)
{
    if (carousel!=1)
    {
        StartPlayFile(videoPath,carousel);
        usleep(200*1000);
        audio_open_control();
        // strncpy(playFilePath,videoPath,100);
        // StartPlayStreamFile(videoPath);//不能直接调用 因为需要其他变量设置
        printf("user_start_play_video[%s]...................\n",videoPath);
    }
}

////////////////////////add it by adien//////////////////////


