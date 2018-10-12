#########################################################################
# File Name: install.sh
# Author: xuwenlong
# mail: myxuan475@126.com
# Created Time: 2018年02月11日 星期日 09时31分56秒
#########################################################################
#!/bin/bash

show_opts()
{
	echo "install.sh 是用于安装雷优科技公司网关数据采集系统的工具"
	echo "基本功能:"
	echo "  install  安装程序"
	echo "  uninstall 卸载程序"
	echo "参数详细说明："
	echo "  -u,--update 是否更新配置,0:不更新,1:更新"
	echo "  -h,--help 显示帮助信息"
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

	read -p "是否更新数据库、文件配置?[y/N]:" select
	#read select
	if [ $select = "y" ];then
	    copy_dir $BASE_DIR/config $ROOT/config
	    echo "当前默认配置信息:"
		cat $ROOT/config/Config.ini
		
		echo "当前系统可用COM端口:"
		dmesg|grep ttyS*
		
		read -p "是否修改配置?[y/N]:" m
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


