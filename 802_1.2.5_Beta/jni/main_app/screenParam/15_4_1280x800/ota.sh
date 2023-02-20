#!/bin/sh

#远程升级部分用到的变量
ota_flag=/data/ota_flag
ota_path=/data/ota_path
if [ -f "$ota_flag" ]&&[ -f "$ota_path" ]; then
	echo "0">$ota_flag
	if [ -f "/data/SStarOta.bin.gz" ]; then
		rm -rf /data/SStarOta.bin.gz
		sync
	fi
fi
#U盘升级部分用到的变量
otaPath=/vendor/udisk_sda1
versionPath=/data/version.txt
upgradeResult=0

#升级线程永不退出
while true
do
    #U盘升级脚本内容
    files=$(ls $otaPath)
    for filename in $files
    do 
		otatype=${filename%_*}
		echo "ls file is $filename otatype is $otatype"

		if [ "$otatype" = "SStarOta.bin.gz" ];then
			echo "match ota file is $otatype"
			#匹配到升级文件后 再校验版本号
			oldVersion=`cat $versionPath` 
			oldVersion=${oldVersion#*\"app_version\":\"}
			oldVersion=${oldVersion%%\"*}
			newVersion=${filename#*_}
			echo "oldVersion is $oldVersion; newVersion is $newVersion"
			#去掉开头的字母
			oldVersion=${oldVersion#*.}
			newVersion=${newVersion#*.}
			#比较最高位版本
			oldNum=${oldVersion%%.*}
			newNum=${newVersion%%.*}
			oldVersion=${oldVersion#*.}
			newVersion=${newVersion#*.}
			echo "$newNum ===1=== $oldNum"
			if [ "$newNum" -eq "$oldNum" ];then
				echo "$newNum = $oldNum"
				#比较次高位版本
				oldNum=${oldVersion%%.*}
				newNum=${newVersion%%.*}
				oldVersion=${oldVersion#*.}
				newVersion=${newVersion#*.}
				echo "$newNum ===2=== $oldNum"
				if [ "$newNum" -eq "$oldNum" ];then
					echo "$newNum = $oldNum"
					#比较最低位版本
					oldNum=${oldVersion%%.*}
					newNum=${newVersion%%.*}
					echo "$newNum ===3=== $oldNum"
					if [ "$newNum" -gt "$oldNum" ];then
						#新版本为高版本 可以升级
						echo "$newNum > $oldNum"
						upgradeResult=1
					fi
				elif [ "$newNum" -gt "$oldNum" ];then
					#新版本为高版本 可以升级
					echo "$newNum > $oldNum"
					upgradeResult=1
				fi
			elif [ "$newNum" -gt "$oldNum" ];then
				#新版本为高版本 可以升级
				echo "$newNum > $oldNum"
				upgradeResult=1
			fi
			echo "upgradeResult========$upgradeResult"
			if [ "$upgradeResult" = "1" ];then
				#进入升级流程
				pkill MyPlayer
				pkill zkgui
				# fuser -km /customer 
				# fuser -km /config
				# fuser -km /appconfigs
				sleep 1
				umount /sys
				umount /sys/kernel/debug/
				# umount /customer
				# umount /config
				# umount /appconfigs
				/bin/otaunpack -x "$otaPath/$filename"
				echo "$otaPath/$filename"
			fi
		fi
    done

    #远程升级脚本内容
	echo "------ota -----"
	if [ -f "$ota_flag" ]&&[ -f "$ota_path" ]; then
		test=`cat $ota_flag` 
		echo "$test"
		if [ "$test" = "2" ];then
			echo "=========start to ota http========="
			httpPath=`cat $ota_path` 
			echo "$httpPath"
			sleep 1
			pkill MyPlayer
			pkill zkgui
			# fuser -km /customer 
			# fuser -km /config
			# fuser -km /appconfigs
			sleep 1
			umount /sys
			umount /sys/kernel/debug/
			# umount /customer
			# umount /config
			# umount /appconfigs
			/bin/otaunpack -x $httpPath
		fi
	fi
    #轮询升级程序的时间
	sleep 6
done



