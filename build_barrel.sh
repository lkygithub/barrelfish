cd build
make ZynqmpZCU104 -j9
cd armv8/sbin
aarch64-linux-gnu-objcopy -I elf64-littleaarch64 -O elf64-littleaarch64 --change-addresses 0xffff00007737a000 cpu_zynqmp cpu_zynqmp_relocated
echo "kernel rellocated at 0xffff00007737a000\n"
