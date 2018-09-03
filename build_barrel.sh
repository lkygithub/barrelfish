cd build
make ZynqmpZCU104 -j9
cd armv8/sbin
aarch64-linux-gnu-objcopy -I elf64-littleaarch64 -O elf64-littleaarch64 --change-addresses 0xffff000077378000 cpu_zynqmp cpu_zynqmp_relocated

