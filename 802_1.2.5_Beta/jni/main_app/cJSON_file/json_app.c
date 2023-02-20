#include "json_app.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int create_json_table(cJSON *json_table, add_type type, ...)
{
	va_list ap;
	va_start(ap, type);
	char *key = NULL, *value_char = NULL;
	int value_int = 0;
	long int value_long = 0;
	double value_doubel = 0.0;
	if (NULL == json_table)
	{
		return -1;
	}
	if (J_Array != type)
	{
		key = va_arg(ap, char *);
	}
	switch (type)
	{
	case J_String:
		value_char = va_arg(ap, char *);
		if (NULL != value_char)
		{
			cJSON_AddStringToObject(json_table, key, value_char);
		}
		break;
	case J_Int:
		value_int = va_arg(ap, int);
		cJSON_AddNumberToObject(json_table, key, value_int);
		break;
	case J_Long:
		value_long = va_arg(ap, long int);
		cJSON_AddNumberToObject(json_table, key, value_long);
		break;
	case J_Double:
		value_doubel = va_arg(ap, double);
		cJSON_AddNumberToObject(json_table, key, value_doubel);
		break;
	case J_Bool:
		value_int = va_arg(ap, int);
		cJSON_AddBoolToObject(json_table, key, value_int);
		break;
	case J_Item:
		cJSON_AddItemToObject(json_table, key, va_arg(ap, cJSON *));
		break;
	case J_Array:
		cJSON_AddItemToArray(json_table, va_arg(ap, cJSON *));
		break;
	default:
		break;
	}
	va_end(ap);
	return 0;
}

int parse_json_table(cJSON *json_table, add_type type, char *key, void *value)
{
	if (NULL == json_table)
	{
		return -1;
	}
	
	if (type != J_Array)
	{
		if (cJSON_HasObjectItem(json_table, key))
		{
			switch (type)
			{
			case J_String:
				strcpy((char *)value, cJSON_GetObjectItem(json_table, key)->valuestring);
				break;
			case J_Bool:
			case J_Int:
				*(int *)value = cJSON_GetObjectItem(json_table, key)->valueint;
				break;
			case J_Long:
				*(long int *)value = (long int)((cJSON_GetObjectItem(json_table, key)->valuedouble));
				break;
			case J_Double:
				*(double *)value = cJSON_GetObjectItem(json_table, key)->valuedouble;
				break;
			default:
				break;
			}
		}
	}
	else
	{
		int i = 0;
		cJSON *data = NULL;
		for (i = 0; i < cJSON_GetArraySize(json_table); i++)
		{
			if (NULL != (data = cJSON_GetArrayItem(json_table, i)))
			{
				if (NULL != (data = cJSON_GetObjectItem(data, key)))
				{
					*(cJSON *)value = *data;
				}
			}
		}
	}
	return 0;
}
