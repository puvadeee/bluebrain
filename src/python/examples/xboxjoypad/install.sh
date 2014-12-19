if [ "$EUID" -ne 0 ]
  then echo "Please run as root"
  exit
fi
apt-get install xboxdrv
cp -r etc /
chmod +x /etc/init.d/xboxdrv
update-rc.d xboxdrv start
echo "Please reboot"
