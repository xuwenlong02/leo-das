#########################################################################
# File Name: das-run.sh
# Author: xuwenlong
# mail: myxuan475@126.com
# Created Time: 2018年02月11日 星期日 09时41分11秒
#########################################################################
#!/bin/sh

#set -v

ROOT=/leo-das
LOG_DIR=$ROOT/log
run=1
count=0
record=0

if [ -e /mnt/yaffs2/net.conf ];then
source /mnt/yaffs2/net.conf
if [ -z "$CARD" ];then
  CARD=eth0
fi

NEWIP=$(echo $IPADDR|sed -e 's/\(.*\)\.\(.*\)/\1.244/')
ifconfig $CARD:1 $NEWIP netmask 255.255.255.0
ifconfig $CARD:1 up
fi

#$ROOT/boa/bin/boa &

# version
update_packet()
{
if [ -e /tmp/install/bin/leo-das ];then
    NEWVER=$(/tmp/install/bin/leo-das -v)
	if [ $? -eq 0 ];then
		VER=$($ROOT/bin/leo-das -v)
		if [ "$NEWVER" != "$VER" ];then
			/tmp/install/install.sh 0
		fi
	fi
fi
}
update_packet

export DAS_CONFIG_DIR=$ROOT
count=`ps l|grep $ROOT/bin/leo-das|grep -v 'grep'|wc -l`

if [ $count -eq 0 ];then
	$ROOT/bin/script-server &
	$ROOT/bin/leo-das
	if [ $? -eq 0 ];then
		echo "update_packet"
		update_packet
	else
	    echo "error execute"
		return 1
	fi
	mv $LOG_DIR/leo-das.log $LOG_DIR/err.log
	echo "error ${record}" >> $LOG_DIR/err.log
else
    echo "One instance has been running!"
	return 0
fi

reboot
echo "exit das"

