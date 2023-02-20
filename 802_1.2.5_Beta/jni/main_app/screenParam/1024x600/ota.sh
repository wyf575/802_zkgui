#!/bin/sh
ota_flag=/data/ota_flag
ota_path=/data/ota_path
if [ -f "$ota_flag" ]&&[ -f "$ota_path" ]; then
	echo "0">$ota_flag
	if [ -f "/data/SStarOta.bin.gz" ]; then
		rm -rf /data/SStarOta.bin.gz
		sync
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
			/bin/otaunpack -x $localPath
		elif [ "$test" = "2" ];then
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
	# fuser -km /customer 
	# fuser -km /config
	# fuser -km /appconfigs
	# sleep 1
	# umount /customer
	# umount /config
	# umount /appconfigs
	# /bin/otaunpack -x /data/SStarOta.bin.gz
	sleep 6
done
# mount: mounting none on /sys failed: Device or resource busy
# mount: mounting none on /sys/kernel/debug/ failed: Device or resource busy
