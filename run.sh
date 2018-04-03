#!/bin/zsh

cd build_arm
make armv7/sbin/ed -j5
make armv7/sbin/examples/xmpl-hello -j5
sudo cp ./armv7_jetsontk1_image /var/lib/tftpboot/barimg
make JetsonTK1 -j5
cd ~/mCode/barrelfish
