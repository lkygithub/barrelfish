%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Copyright (c) 2017, ETH Zurich.
% All rights reserved.
%
% This file is distributed under the terms in the attached LICENSE file.
% If you do not find this file, copies can be found by writing to:
% ETH Zurich D-INFK, Universitaetsstrasse 6, CH-8092 Zurich. Attn: Systems Group.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Statically-initialised hardware facts for the Xilinx ZynqMP SoC
boot_driver("/armv8/sbin/boot_armv8_generic").
cpu_driver("/armv8/sbin/cpu_zynqmp").
monitor("/armv8/sbin/monitor").


% One cluster of four Cortex A53s
arm_core(16'000000, cortexA57).
arm_core(16'000001, cortexA57).
arm_core(16'000002, cortexA57).
arm_core(16'000003, cortexA57).

% boot dirver entry points
entry_symbol(armBootBSP, "boot_entry_bsp").
entry_symbol(armBootPSCI, "boot_entry_psci").
entry_symbol(armBootParking, "boot_entry_parking").

% ACPI quirks
acpi_quirk(AcpiInitializeObjects, 16'20).
