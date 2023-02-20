# 广告机与蜜连平台交互API

- 重要提示：本文已经简化部分键值对，提取出与广告机业务密切相关的参数。
- 如有需要API交互的全文，请动手实际测试。

## MQTT通知设备参数

MQTT通知设备更新参数【TONFVE866193054522756，测试服务器】

```json
{
    "cmd": 3807,
    "msg": "request",
    "extend": null,
    "digital": 0,
    "seq": "5d147c354fda79303fd9089b"
}
{
    "cmd": 4807,
    "msg": "response",
    "extend": null,
    "digital": 0,
    "seq": "5d147c354fda79303fd9089b"
}
```

## 广告/二维码任务相关

(广告机设备端)广告机获取播放列表和参数：

```http
GET http://tmp.ibeelink.com/api/ad/play/list-param?sn=TONFVE&paramType=1
```

```json
{
    "data": {
        "param": [{
            "id": 380,
            "params": [{
                "key": "qr_link",
                "value": [{
                    "content": "www.ibeelink.com"
                }]
            }, {
                "key": "vol",
                "value": [{
                    "content": 30
                }]
            }, {
                "key": "qr_size",
                "value": [{
                    "content": 22
                }]
            }, {
                "key": "QRCodeDisplay",
                "value": [{
                    "content": 1
                }]
            }]
        }],
        "playList": [{
			"dateEnd": 1623167999000,
			"dateStart": 1622476800000,
			"mediaId": "acuH2v_9fGdOGWpD_o0b",
			"mediaResource": "https://mi-bucket-te969614.mp4",
			"mediaSize": 5158098,
			"mediaType": 2,
			"taskId": 378,
			"timeEnd": 2359,
			"timeStart": 0,
		}, {
			"dateEnd": 1623167999000,
			"dateStart": 1622476800000,
			"interval": 123,
			"mediaId": "TS0JKvXHhEtGfrfI1DhQ",
			"mediaSize": 127873,
			"mediaType": 1,
			"picResource": ["https://mi-bucket-0712.jpg"],
			"taskId": 380,
			"timeEnd": 2359,
			"timeStart": 0,
		}]
    },
    "errorCode": 0,
    "serverTime": 1620462628941
}
```

(广告机设备端)设备上报广告更新结果：

```http
POST http://tmp.ibeelink.com/api/ad/report
```



```json
{
  "success": [380,381,333], //通知服务器这些广告任务已经拉取成功[没有也需要占位][键：taskId]
  "failed": [380,381,333],  //通知服务器这些广告任务拉取失败[没有也需要占位]
  "delete": [380,381,333],  //通知服务器这些广告任务已经在设备端删除[没有也需要占位]
}
```

设备上报参数结果：

```http
POST http://tmp.ibeelink.com/api/config/success
```

```json
{
  "id": 380 //通知服务器该参数组已经拉取成功[键：id]
}
```

![image-20210508135727845](https://cdn.jsdelivr.net/gh/Forwardxiang/cloudimg/dataimage-20210508135727845.png)



## 推广二维码格式

二维码显示内容：

```http
GET http://tmp.ibeelink.com/popularize?sn=ETEJKG
```

```
在设备端生成对应的二维码即可
```



## 获取设备SN

设备端获取SN：

```http
GET http://tmp.ibeelink.com/api/device/exchange/sn?deviceId=9140D27E223872018A8924D7B65DBD54
```

```json
{
	"data": {
		"sn": "TONFVE"
	},
	"errorCode": 0
}
```



### 远程升级任务相关

设备远程升级：

```http
GET http://tmp.ibeelink.com/api/version/upgrade/did?deviceId=9140D27E223872018A8924D7B65DBD54&version=v1.0.1&type=app
```

```json
{
	"data": {
		"crc": 2674013826,
		"file": "https://mi-bucket-test.oss-cn-shanghai.aliyuncs.com/1c72939579.bin",
		"fileSize": 111367,
	},
	"errorCode": 0
}
```

