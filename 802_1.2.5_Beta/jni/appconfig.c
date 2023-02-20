/*
 * panelconfig.c
 *
 *  Created on: 2019年10月22日
 *      Author: koda.xu
 */

#include "appconfig.h"

//honestar karl.hong add 20210826
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#define PANEL_NAME_SIZE 48
#define PANEL_CONFIG_FILE	"/config/panel.ini"
#define PARAM_TARGET_PANEL "m_targetPanel"

//honestar karl.hong add 20210826


#if USE_PANEL_1024_600
#include "SAT070CP50_1024x600.h"
#else
//#include "SAT070AT50_800x480.h"
#include "MV190E0M_1280x1024_MIPI.h"
#endif

//honestar karl.hong add 20210826

#define CHECK_FILE_EXIST(path) do{\
	if(0 != access(path, F_OK)){\
		printf("%s not exist\n",path);\
		return -1;}}while(0);


int g_RotateSetting;
int g_PanelWidth;
int g_PanelHeight;
int g_UIWidth;
int g_UIHeight;


static int readline(FILE *fp,char * stream)
{
	char dat;
	do{
		dat = fgetc(fp);
		*stream = dat;
		stream ++;
		if(dat == EOF){
			printf("======>EOF\n");
			return -1;
		}
	}while(dat != '\n');

	return 0;
}
/*
* 获取当前在使用屏幕名称
*/
static int get_target_panel_name(FILE *fp, char* panelName)
{
	int err;
	char stream[100];
	char *p;
	
	fseek(fp,0,SEEK_SET);
	
	readline(fp,stream);
	
	err = strncasecmp(PARAM_TARGET_PANEL,stream,strlen(PARAM_TARGET_PANEL));
	if(0 != err){
		printf("[line:%d]get target panel name fail\n",__LINE__);
		return -1;
	}

	p = strchr(stream,'=');
	
	p ++;
	for(int i = 0;i < PANEL_NAME_SIZE; i++){
		if(*p == ';'){
			*panelName = '\0';
			printf("get panel name success\n");
			return 0;
		}
	
		if(isspace(*p) == 0){
			*panelName = *p;
			panelName ++;
		}
		p++;
	}
	
	printf("[line:%d]get target panel name fail\n",__LINE__);
	return -1;
}

/*used to read panel.ini and fbdev.ini*/
static int getParamByName(FILE *fp,char* panelName, char* paramName)
{
	char *p;
	int param;
	char stream[100];
	char targetPanel[50];
	
	fseek(fp,0,SEEK_SET);
	
	sprintf(targetPanel,"[%s]",panelName);
	
	do{
		memset(&stream,0,sizeof(stream));
		if(-1 == readline(fp, stream)){
			printf("[line:%d]get  panel param %s fail\n",__LINE__,paramName);
			return -1;
		}
	}while(strncasecmp(targetPanel,stream,strlen(targetPanel)) != 0);

	do{
		memset(&stream,0,sizeof(stream));
		if(-1 == readline(fp, stream)){
			printf("[line:%d]get  panel param %s fail\n",__LINE__,paramName);
			return -1;
		}
	}while(0 != strncasecmp(paramName,stream,strlen(paramName)));

	p = strchr(&stream[0],'=');
	
	p ++;
	param =atoi(p);
	
	return param;
}

/*used to read EasyUI.cfg*/
static int getParamByName2(FILE *fp,char* paramName)
{
	char *p;
	int err;
	int param;
	char stream[100];


	fseek(fp,0,SEEK_SET);
	
	do{
		memset(&stream,0,sizeof(stream));
		if(-1 == readline(fp, stream)){
			printf("[line:%d]get  panel param %s fail\n",__LINE__,paramName);
			return -1;
		}

		p = strstr(stream,"\"");
		err = 1;
		if(p != 0){
			p ++;
			err = strncasecmp(paramName,p,strlen(paramName));
		}
	}while(0 != err);

	p = strchr(&stream[0],':');
	
	p ++;
	param =atoi(p);
	
	return param;
}


void GetPanelSetting(void)
{
	FILE *fp;
	char panelName[48];	

	CHECK_FILE_EXIST(PANEL_CONFIG_FILE);

	fp = fopen(PANEL_CONFIG_FILE,"r");
	if(fp == NULL ){
		printf("open %s fail\n",PANEL_CONFIG_FILE);
		return -1;
	}

	if(-1 == get_target_panel_name(fp,panelName)) goto err;
	/*根据屏幕配置名称获取分辨率数据*/
	g_PanelWidth = getParamByName(fp,panelName,"m_wPanelWidth");
	if(g_PanelWidth == -1) goto err;
	
	g_PanelHeight = getParamByName(fp,panelName,"m_wPanelHeight");
	if(g_PanelHeight == -1) goto err;
	fclose(fp);

	fp = fopen("/etc/EasyUI.cfg","r");
	if(fp == NULL){
		printf("open EasyUI.cfg fail\n");
		return -1;
	}

	g_RotateSetting = getParamByName2(fp,"rotateScreen");
	if(g_RotateSetting == -1) goto err;

	fclose(fp);

	fp = fopen("/config/fbdev.ini","r");
	if(fp == NULL){
		printf("open fbdev.ini fail\n");
		return -1;
	}
	
	g_UIWidth = getParamByName(fp,"FB_DEVICE","FB_WIDTH");
	g_UIHeight = getParamByName(fp,"FB_DEVICE","FB_HEIGHT");
	
	fclose(fp);
				
	printf("g_PanelWidth: %d, g_PanelHeight: %d, g_RotateSetting: %d, g_UIHeight: %d, g_UIHeight: %d\n",g_PanelWidth,g_PanelHeight,g_RotateSetting,g_UIHeight,g_UIHeight);
	
	return 0;
err:
	fclose(fp);
	return -1;
}

int GetRotateSetting(void)
{
	return g_RotateSetting;
}

int GetPanelWidth(void)
{
	return g_PanelWidth;
}

int GetPanelHeigth(void)
{
	return g_PanelHeight;
}

int GetFbWidth(void)
{
	return g_UIWidth;
}

int GetFbHeight(void)
{
	return g_UIHeight;
}

//honestar karl.hong add 20210826


