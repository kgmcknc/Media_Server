# This file deploys the Media_Server application code into your system
# Calling this file forces the git depot into the deploy folder
# This file will auto-run makefiles for all deployed c programs
# Specify Server or Client as an input argument when calling this script

if [ "$#" -eq 0 ] ; then
	echo "Error! Needs command line argument"
	echo "Specify Server or Client for which side to deploy"
	echo "Common code automatically deployed unless -nc argument is appended"
	exit
else
	if [ "$1" = "Server" ] || [ "$1" = "server" ] ; then
		echo "Arg was Server!"
		deploy_server=1;
		deploy_client=0;
		if [ "$#" -eq 2 ] ; then
			if [ "$2" = "-nc" ] ; then
				deploy_common=0;
				echo "Not Copying Common Code!"
			else
				deploy_common=1;
			fi
		else
			deploy_common=1;
		fi
	else
		if [ "$1" = "Client" ] || [ "$1" = "client" ] ; then
			echo "Arg was Client!"
			deploy_server=0;
			deploy_client=1;
			if [ "$#" -eq 2 ] ; then
				if [ "$2" = "-nc" ] ; then
					deploy_common=0;
					echo "Not Copying Common Code!"
				else
					deploy_common=1;
				fi
			else
				deploy_common=1;
			fi
		else
			if [ "$1" = "Common" ] || [ "$1" = "common" ] ; then
				echo "Arg was Common!"
				deploy_server=0;
				deploy_client=0;
				if [ "$#" -eq 2 ] ; then
					if [ "$2" = "-nc" ] ; then
						deploy_common=0;
						echo "Not Copying Common Code!"
					else
						deploy_common=1;
					fi
				else
					deploy_common=1;
				fi
			else
					echo "Error! Needs command line argument"
					echo "Specify Server or Client for which side to deploy"
					echo "Common code automatically deployed unless -nc argument is appended"
				exit
			fi
		fi
	fi
fi

sys_deploy_dir="/usr/share/media_server"
sys_config_file="/usr/share/media_server/sys_control/sys_config.kmf"
sys_status_file="/usr/share/media_server/sys_control/sys_status.kmf"
sys_names_file="/usr/share/media_server/sys_control/sys_names.kmf"
sys_moviedir_file="/usr/share/media_server/sys_control/moviectrl/moviedir.kmf"
sys_musicdir_file="/usr/share/media_server/sys_control/musicctrl/musicdir.kmf"
sys_playlistdir_file="/usr/share/media_server/sys_control/playlistdir.kmf"
www_deploy_dir="/var/www/html/media_server"
rx_fifo="/var/www/html/media_server/control/web_control/rxwebpipe"
tx_fifo="/var/www/html/media_server/control/web_control/txwebpipe"

if [ "$deploy_server" -eq 1 ] ; then
	echo "Deploying Server Side System Code to $sys_deploy_dir"
fi
if [ "$deploy_client" -eq 1 ] ; then
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

if [ "$deploy_server" -eq 1 ] ; then
	for file_name in ./Server/sys/*; do
		cp -r "$file_name" "$sys_deploy_dir"
	done
fi
if [ "$deploy_client" -eq 1 ] ; then
	for file_name in ./Client/sys/*; do
		cp -r "$file_name" "$sys_deploy_dir"
	done
fi
if [ "$deploy_common" -eq 1 ] ; then
	for file_name in ./Common/sys/*; do
		cp -r "$file_name" "$sys_deploy_dir"
	done
fi

if [ "$deploy_server" -eq 1 ] ; then
	echo "Deployed Server Side System Files"
	echo "Deploying Server Side Web Code to $www_deploy_dir"
fi
if [ "$deploy_client" -eq 1 ] ; then
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

if [ "$deploy_server" -eq 1 ] ; then
	for file_name in ./Server/www/*; do
		cp -r "$file_name" "$www_deploy_dir"
	done
fi
if [ "$deploy_client" -eq 1 ] ; then
	for file_name in ./Client/www/*; do
		cp -r "$file_name" "$www_deploy_dir"
	done
fi
if [ "$deploy_common" -eq 1 ] ; then
	for file_name in ./Common/www/*; do
		cp -r "$file_name" "$www_deploy_dir"
	done
fi


if [ "$deploy_server" -eq 1 ] ; then
	echo "Deployed Server Side WWW Files"
fi
if [ "$deploy_client" -eq 1 ] ; then
	echo "Deployed Client Side WWW Files"
fi
if [ "$deploy_common" -eq 1 ] ; then
	echo "Deployed Common Side WWW Files"
fi

if [ -e $sys_config_file ] ; then
	echo "Sys Config File Already Exists"
else 
	touch $sys_config_file
fi

if [ -e $sys_status_file ] ; then
	echo "Sys Status File Already Exists"
else 
	touch $sys_status_file
fi

if [ -e $sys_names_file ] ; then
	echo "Sys Names File Already Exists"
else 
	touch $sys_names_file
fi

if [ -e $rx_fifo ] ; then
	echo "Rx Fifo Already Exists"
else 
	mkfifo $rx_fifo
fi

if [ -e $tx_fifo ] ; then
	echo "Tx Fifo Already Exists"
else
	mkfifo $tx_fifo
fi

if [ "$deploy_server" -eq 1 ] ; then
	if [ -e $sys_moviedir_file ] ; then
		echo "MovieDir Config File Already Exists"
	else 
		touch $sys_moviedir_file
	fi
	
	if [ -e $sys_musicdir_file ] ; then
		echo "MovieDir Config File Already Exists"
	else 
		touch $sys_musicdir_file
	fi

	if [ -e $sys_playlistdir_file ] ; then
		echo "PlaylistDir Config File Already Exists"
	else
		touch $sys_playlistdir_file
	fi
fi

# Handle Compiling C Programs when Deploying Code
cd $sys_deploy_dir/sys_control/
make
cd $www_deploy_dir/control/web_control/
make

echo "Run sudo /usr/share/media_server/sys_control/sys_control -config"
