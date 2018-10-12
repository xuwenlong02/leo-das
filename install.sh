#########################################################################
# File Name: install.sh
# Author: xuwenlong
# mail: myxuan475@126.com
# Created Time: 2018��02��11�� ������ 09ʱ31��56��
#########################################################################
#!/bin/bash

show_opts()
{
	echo "install.sh �����ڰ�װ���ſƼ���˾�������ݲɼ�ϵͳ�Ĺ���"
	echo "��������:"
	echo "  install  ��װ����"
	echo "  uninstall ж�س���"
	echo "������ϸ˵����"
	echo "  -u,--update �Ƿ��������,0:������,1:����"
	echo "  -h,--help ��ʾ������Ϣ"
}

create_dir()
{
    echo "create $1"
	if [ ! -d "$1" ];then
		mkdir -p "$1"
		echo "create $1"
	else
		echo "$1 exist"
	fi
}

copy_files()
{
    echo "copy $1"
	cp -afv $1 $2
	if [ $? -ne 0 ];then
		echo "copy $1 failed"
		exit 1
	fi
}

copy_dir()
{
    echo "copy $1"
	cp -rfv $1 $2
	if [ $? -ne 0 ];then
		echo "copy $1 failed"
		exit 1
	fi
}

BASE_DIR=$(cd `dirname $0`;pwd)
ROOT=/leo-das
UPDATE_CONFIG=$1

updateconfig()
{

	read -p "�Ƿ�������ݿ⡢�ļ�����?[y/N]:" select
	#read select
	if [ $select = "y" ];then
	    copy_dir $BASE_DIR/config $ROOT/config
	    echo "��ǰĬ��������Ϣ:"
		cat $ROOT/config/Config.ini
		
		echo "��ǰϵͳ����COM�˿�:"
		dmesg|grep ttyS*
		
		read -p "�Ƿ��޸�����?[y/N]:" m
		if [ $m = "y" ];then
		    vi $ROOT/config/Config.ini
	    fi
        chmod a+rwx -R $ROOT/data/
		rm -f $ROOT/data/*
    fi
}

install()
{
    create_dir $ROOT
    create_dir $ROOT/bin
    create_dir $ROOT/log
	create_dir $ROOT/data
    copy_dir $BASE_DIR/bin $ROOT/
    copy_dir $BASE_DIR/libs   $ROOT/
    copy_dir $BASE_DIR/script $ROOT/
   # copy_dir boa $ROOT/

    if [ "$UPDATE_CONFIG" != "0" ];then
		updateconfig
	fi

	chmod a+rwx -R $ROOT/config/
    chmod a+rwx -R $ROOT/data/
    chmod a+rwx -R $ROOT/bin/
    chmod a+rwx -R $ROOT/log/
    chmod a+rwx -R $ROOT/script/
#    chmod a+rwx -R $ROOT/boa/
	
	if [ `grep -c "$ROOT/bin/das-run.sh" /etc/init.d/rcS` -eq '0' ];then
	    echo "$ROOT/bin/das-run.sh &" >> /etc/init.d/rcS
	fi
	
	if [ `grep -c "$ROOT/bin/watchdog" /etc/init.d/rcS` -eq '0' ];then
	    echo "sleep 3" >> /etc/init.d/rcS
	    echo "$ROOT/bin/watchdog &" >> /etc/init.d/rcS
	fi
	
	mv /etc/boa /etc/boa-beta
	mv /opt/dapc /opt/dapc-beta
	mv /usr/boa /usr/boa-beta
	
    echo "--------install success---------------"
}


uninstall()
{

    if [ -f "$ROOT/bin/leo-das" ];then
       rm -fv $ROOT/bin/leo-das
    fi
    
    if [ -f "$ROOT/bin/das-run.sh" ];then
       rm -fv $ROOT/bin/das-run.sh
    fi
    
    echo "--------uninstall success---------------"
}

parse_opts()
{
	shift
	ARGS=`getopt -o h:u: -l help:,update: -- "$@"`
	eval set -- "$ARGS"

	while true
	do
		case $1 in
			-u|--update)
				if [ $2 -eq 0 ];then
					UPDATE_CONFIG=0
				else
					UPDATE_CONFIG=1
				fi
				shift 2
				;;
			-h|--help)
				show_opts
				shift 2
				exit 0
				;;
			*)
				UPDATE_CONFIG=0
				break
				;;
		esac
	done
}

#case $1 in
#   install)
#   	parse_opts "$@"
#   	install
#   	exit 0
#   	;;
#   uninstall)
#   	parse_opts "$@"
#   	uninstall
#   	exit 0
#   	;;
#   *)
#   	parse_opts "$@"
#   	exit 0
#   	;;
#esac

install


