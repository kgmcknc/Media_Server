# This file deploys the Media_Server application code into your system
# Calling this file forces the git depot into the deploy folder

if [ "$#" -eq 0 ]; then
	echo "Error! Needs command line argument"
	echo "Specify Main or Pi for which side to deploy"
	exit
else
	if [ "$1" = "Main" ] ; then
		echo "Arg was Main!"
		deploy_main=1;
		deploy_pi=0;
		deploy_common=0;
	else
		if [ "$1" = "Pi" ] ; then
			echo "Arg was Pi!"
			deploy_main=0;
			deploy_pi=1;
			deploy_common=0;
		else
			if [ "$1" = "Common" ] ; then
				echo "Arg was Common!"
				deploy_main=0;
				deploy_pi=0;
				deploy_common=1;
			else
				echo "Error! Not valid command line argument"
				echo "Specify Main or Pi for which side to deploy"
				exit
			fi
		fi
	fi
fi

sys_deploy_dir="/usr/share/media_server/"
www_deploy_dir="/var/www/html/media_server/"
rx_fifo="/var/www/html/media_server/control/web_control/rxwebfifo"

if [ "$deploy_main" -eq 1 ] ; then
	echo "Deploying Server Side System Code to $sys_deploy_dir"
fi
if [ "$deploy_pi" -eq 1 ] ; then
	echo "Deploying Client Side System Code to $sys_deploy_dir"
fi
if [ "$deploy_common" -eq 1 ] ; then
	echo "Deploying Common System Code to $sys_deploy_dir"
fi

if [ -d "$sys_deploy_dir" ] ; then
	if [ -L "$sys_deploy_dir" ] ; then
		#echo "existed as link"
		skip=1
	else
		#echo "existed as dir"
		skip=1
	fi
else
	echo "$sys_deploy_dir does not exist... creating"
	mkdir $sys_deploy_dir
fi

echo "Copying All Files to $sys_deploy_dir"

if [ "$deploy_main" -eq 1 ] ; then
	for file_name in ./Main/sys/*; do
		cp -r "$file_name" "$sys_deploy_dir"
	done
fi
if [ "$deploy_pi" -eq 1 ] ; then
	for file_name in ./Pi/sys/*; do
		cp -r "$file_name" "$sys_deploy_dir"
	done
fi
if [ "$deploy_common" -eq 1 ] ; then
	for file_name in ./Common/sys/*; do
		cp -r "$file_name" "$sys_deploy_dir"
	done
fi

if [ "$deploy_main" -eq 1 ] ; then
	echo "Deployed Server Side System Files"
	echo "Deploying Server Side Web Code to $www_deploy_dir"
fi
if [ "$deploy_pi" -eq 1 ] ; then
	echo "Deployed Client Side System Files"
	echo "Deploying Client Side Web Code to $www_deploy_dir"
fi
if [ "$deploy_common" -eq 1 ] ; then
	echo "Deployed Common Side System Files"
	echo "Deploying Common Side Web Code to $www_deploy_dir"
fi

if [ -d "$www_deploy_dir" ] ; then
	if [ -L "$www_deploy_dir" ] ; then
		#echo "existed as link"
		skip=1
	else
		#echo "existed as dir"
		skip=1
	fi
else
	echo "$www_deploy_dir does not exist... creating"
	mkdir $www_deploy_dir
fi

echo "Copying All Files to $www_deploy_dir"

if [ "$deploy_main" -eq 1 ] ; then
	for file_name in ./Main/www/*; do
		cp -r "$file_name" "$www_deploy_dir"
	done
	mkfifo "$rx_fifo"
fi
if [ "$deploy_pi" -eq 1 ] ; then
	for file_name in ./Pi/www/*; do
		cp -r "$file_name" "$www_deploy_dir"
	done
	mkfifo "$rx_fifo"
fi
if [ "$deploy_common" -eq 1 ] ; then
	for file_name in ./Common/www/*; do
		cp -r "$file_name" "$www_deploy_dir"
	done
fi


if [ "$deploy_main" -eq 1 ] ; then
	echo "Deployed Server Side WWW Files"
fi
if [ "$deploy_pi" -eq 1 ] ; then
	echo "Deployed Client Side WWW Files"
fi
if [ "$deploy_common" -eq 1 ] ; then
	echo "Deployed Common Side WWW Files"
fi

