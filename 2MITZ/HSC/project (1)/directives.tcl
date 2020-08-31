############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
set_directive_pipeline -II 1 "fir_fixed"
set_directive_allocation -limit 66 -type operation "fir_fixed" mul
set_directive_array_partition -type complete -dim 1 "fir_fixed" h
set_directive_array_partition -type complete -dim 1 "fir_fixed" regs
set_directive_array_partition -type complete -dim 1 "fir_fixed" mult_temp
set_directive_interface -mode ap_ctrl_none "fir_fixed"
