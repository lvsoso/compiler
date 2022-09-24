OUTPUT_FORMAT(elf32-littleriscv)
MEMORY
{
  RAM (xrw) : ORIGIN = 0x10000 , LENGTH = 0x50000
}
SECTIONS
{
	.text :
	{
		*(.text) *(.text.*)
	} > RAM
	.data :
	{
    	*(.data .data.*) *(.sdata .sdata.*) *(.sdata2.*) *(.rodata) *(.rodata*)
	} > RAM
	.bss :
    {
  	    *(.sbss*) *(.bss*) *(COMMON*)
	} > RAM
}