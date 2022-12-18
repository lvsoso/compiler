package main

import (
	"os"
	"rvld/pkg/linker"
	"rvld/pkg/utils"
)

func main() {
	if len(os.Args) < 2 {
		utils.Fatal("wrong args")
	}

	file := linker.MustNewFile(os.Args[1])

	inputFile := linker.NewInputFile(file)
	utils.Assert(len(inputFile.ElfSections) == 11)
}
