# Initial ARM port
set architecture arm
# set architecture aarch64
target remote 127.0.0.1:2331
# display/i $pc
# monitor speed 4000
monitor reset
monitor reg cpsr = 0xd3
monitor speed 4000
set remote memory-write-packet-size 1024
set remote memory-write-packet-size fixed
# -- Procedures
define show-process

    p (char*)(((struct dispatcher_shared_generic*)$r9)->name)
 
end

define change-process

    ## Load symbols for process shown by `show-process'. 
    ## NB User has to prefix process name with path, e.g. arm/sbin/mem_serv.
    ## XXX No string concat in gdb (?). alt is use gdb with python support.

    #-- Flush old symbols
    symbol-file
    #-- Reload cpu driver symbols
    file arm/sbin/cpu
    #-- Add process symbols at default process load address
    add-symbol-file $arg0 0x00400000

end

# -- Misc 

#add-symbol-file arm/sbin/init 0x400000
#set kernel_log_subsystem_mask = 0x7fffffff

symbol-file
add-symbol-file ./armv7/sbin/boot_tegrak1 0x8000ADC0
add-symbol-file ./armv7/sbin/cpu_tegrak1 0x80015000

break boot_driver.c:439
continue
#commands
#    set kernel_loglevel = 0x7fffff
#    set kernel_log_subsystem_mask = 0x7fffff
#    cont
#end
# load armv7_jetsontk1_image
