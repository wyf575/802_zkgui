#!/bin/sh
ota_flag=/data/ota_flag
ota_path=/data/ota_path
if [ -f "$ota_flag" ]&&[ -f "$ota_path" ]; then
	echo "0">$ota_flag
	if [ -f "/data/SStarOta.bin.gz" ]; then
		rm -rf /data/SStarOta.bin.gz
	fi
fi
while true
do
	echo "------ota -----"
	if [ -f "$ota_flag" ]&&[ -f "$ota_path" ]; then
		test=`cat $ota_flag` 
		echo "$test"
		if [ "$test" = "1" ];then
			echo "=========start to ota local========="
			localPath=`cat $ota_path` 
			echo "$localPath"
			fuser -km /customer 
			fuser -km /config
			fuser -km /appconfigs
			sleep 1
			umount /customer
			umount /config
			umount /appconfigs
			pkill zkgui
			/bin/otaunpack -x $localPath
		elif [ "$test" = "2" ];then
			echo "=========start to ota http========="
			httpPath=`cat $ota_path` 
			echo "$httpPath"
			fuser -km /customer 
			fuser -km /config
			fuser -km /appconfigs
			sleep 1
			umount /customer
			umount /config
			umount /appconfigs
			pkill zkgui
			/bin/otaunpack -x $httpPath
		fi
	fi
	# fuser -km /customer 
	# fuser -km /config
	# fuser -km /appconfigs
	# sleep 1
	# umount /customer
	# umount /config
	# umount /appconfigs
	# /bin/otaunpack -x /data/SStarOta.bin.gz
	sleep 30
done
