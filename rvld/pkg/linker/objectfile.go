package linker

import "debug/elf"

type ObjectFile struct {
	InputFile

	SymtabSec *Shdr
}

func NewObjectFile(file *File) *ObjectFile {
	o := &ObjectFile{InputFile: NewInputFile(file)}
	return o
}

func (o *ObjectFile) Parse() {
	// symbols table,  info save first local variable index
	o.SymtabSec = o.FindSection(uint32(elf.SHT_SYMTAB))
	if o.SymtabSec != nil {
		o.FirstGlobal = int64(o.SymtabSec.Info)
		o.FillUpElfSyms(o.SymtabSec)
		o.SymbolStrtab = o.GetBytesFromIdx(int64(o.SymtabSec.Link))
	}
}
