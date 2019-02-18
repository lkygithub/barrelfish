BARREL_PATH=~/mCode/barrelfish
BUILD_PATH=build_arm
MOUNT_PATH=/mnt/sdcard
SBIN_PATH=armv8/sbin
ECFILE=eclipseclp_ramfs.cpio.gz
SKBFILE=skb_ramfs.cpio.gz
MEMULIST=menu.lst.armv8_zynqmp

# build barrelfish
cd $BUILD_PATH
make ZynqmpZCU104 -j9

#cd armv8/sbin
#aarch64-linux-gnu-objcopy -I elf64-littleaarch64 -O elf64-littleaarch64 --change-addresses 0xffff00007737a000 cpu_zynqmp cpu_zynqmp_relocated
#echo "kernel rellocated at 0xffff00007737a000\n"

cd $SBIN_PATH
aarch64-linux-gnu-objcopy -I elf64-littleaarch64 -O elf64-littleaarch64 --change-addresses 0xffff000077378000 cpu_zynqmp cpu_zynqmp_relocated
cd -

# mount sdcard
if [ ! -d "/mnt/sdcard" ]; then
    sudo mkdir $MOUNT_PATH
fi

sudo mount /dev/sdb1 $MOUNT_PATH

# cp sbin
echo "####: waiting for copy.\n"
if [ $# -eq 0 ]; then
    sudo rm -r $MOUNT_PATH/$SBIN_PATH
    sudo rm $MOUNT_PATH/$ECFILE
    sudo rm $MOUNT_PATH/$SKBFILE
    sudo cp -r $BARREL_PATH/$BUILD_PATH/$SBIN_PATH $MOUNT_PATH/$SBIN_PATH
    sudo cp $BARREL_PATH/$BUILD_PATH/$ECFILE $MOUNT_PATH/$ECFILE
    sudo cp $BARREL_PATH/$BUILD_PATH/$SKBFILE $MOUNT_PATH/$SKBFILE
    sudo cp $BARREL_PATH/hake/$MEMULIST $MOUNT_PATH/menu.lst
else
    sudo rm $MOUNT_PATH/$SBIN_PATH/$1
    sudo cp $BARREL_PATH/$BUILD_PATH/$SBIN_PATH/$1 $MOUNT_PATH/$SBIN_PATH/$1
fi
echo "####: copy done.\n"

#umount sdcard
sudo umount $MOUNT_PATH
# eject sdcard
sudo eject /dev/sdb

echo "All is OK!"
