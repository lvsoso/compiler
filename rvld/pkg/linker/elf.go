package linker

import (
	"bytes"
	"unsafe"
)

const EhdrSize = int(unsafe.Sizeof(Ehdr{}))
const ShdrSize = int(unsafe.Sizeof(Shdr{}))
const SymSize = int(unsafe.Sizeof(Sym{}))

type Ehdr struct {
	Ident     [16]uint8
	Type      uint16
	Machine   uint16
	Version   uint32
	Entry     uint64
	PhOff     uint64
	ShOff     uint64
	Flags     uint32
	EhSize    uint16
	PhEntSize uint16
	PhNum     uint16
	ShEntSize uint16
	ShNum     uint16
	ShStrndx  uint16
}

type Shdr struct {
	Name      uint32 // index, can get name from strtab
	Type      uint32
	Flags     uint64
	Addr      uint64
	Offset    uint64
	Size      uint64
	Link      uint32
	Info      uint32 // save extra info accoding to the section type
	AddrAlign uint64
	EntSize   uint64
}

type Sym struct {
	Name  uint32
	Info  uint8
	Other uint8
	Shndx uint16
	Val   uint64
	Size  uint64
}

func ElfGetName(strTab []byte, offset uint32) string {
	length := uint32(bytes.Index(strTab[offset:], []byte{0}))
	return string(strTab[offset : offset+length])
}
