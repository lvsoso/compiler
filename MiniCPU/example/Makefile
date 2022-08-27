.PHONY: compile show img clean

compile:
	iverilog -o wave -y ./  alu_tb.v  alu.v
	vvp -n wave -lxt2

show:
	gtkwave wave.vcd

img:
	yosys < yosys.do
	dot -Tpng -o alu.png alu.dot

clean:
	rm -f wave wave.vcd alu_synth.v alu.dot alu.png