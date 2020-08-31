############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
open_project hsc
set_top fir_fixed
add_files fir_double.cpp
add_files fir_filter.h
add_files fir_fixed.cpp
add_files -tb tb_fir_fixed.cpp -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "solution1"
set_part {xc7z007s-clg225-1} -tool vivado
create_clock -period 10 -name default
config_sdx -target none
config_export -format ip_catalog -rtl vhdl -vivado_optimization_level 2 -vivado_phys_opt place -vivado_report_level 0
set_clock_uncertainty 12.5%
source "./hsc/solution1/directives.tcl"
csim_design
csynth_design
cosim_design -trace_level all -rtl vhdl -tool xsim
export_design -flow impl -rtl vhdl -format ip_catalog
