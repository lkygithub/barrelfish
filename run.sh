#!/bin/zsh

cd build
make JetsonTK1 -j5
sudo cp ./armv7_jetsontk1_image /var/lib/tftpboot/barimg
cd ~/mCode/barrelfish
