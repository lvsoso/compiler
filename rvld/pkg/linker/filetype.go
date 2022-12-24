package linker

import (
	"debug/elf"
	"rvld/pkg/utils"
)

type FileType = uint8

const (
	FileTypeUnknown FileType = iota
	FileTypeEmpty
	FileTypeObject
)

func GetFileType(contents []byte) FileType {
	if len(contents) == 0 {
		return FileTypeEmpty
	}

	if CheckMagic(contents) {
		et := elf.Type(utils.Read[uint16](contents[16:]))
		switch et {
		case elf.ET_REL:
			return FileTypeObject
		}
		return FileTypeUnknown
	}

	return FileTypeUnknown
}
