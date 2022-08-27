read_verilog alu.v
hierarchy -check
proc; opt; opt; fsm; memory; opt
write_verilog alu_synth.v
show -format dot -prefix ./alu
