package main

import (
	"fmt"
	"go/ast"
	"go/parser"
	"go/token"
	"log"
)

func main() {
	fset := token.NewFileSet()
	f, err := parser.ParseFile(fset, "hello.go", src, parser.AllErrors)
	if err != nil {
		log.Fatal(err)
	}

	for _, decl := range f.Decls {
		if v, ok := decl.(*ast.GenDecl); ok {
			for _, spec := range v.Specs {
				fmt.Printf("%T\n", spec)
			}
		}
	}

	fmt.Println("--------------------------------")

	f, err = parser.ParseFile(fset, "hello1.go", src1, parser.AllErrors)
	if err != nil {
		log.Fatal(err)
	}

	for _, decl := range f.Decls {
		if v, ok := decl.(*ast.GenDecl); ok {
			for _, spec := range v.Specs {
				fmt.Printf("%T\n", spec)
			}
		}
	}

	fmt.Println("--------------------------------")
	f, err = parser.ParseFile(fset, "hello2.go", src2, parser.AllErrors)
	if err != nil {
		log.Fatal(err)
	}

	for _, decl := range f.Decls {
		if v, ok := decl.(*ast.GenDecl); ok {
			fmt.Printf("token: %v\n", v.Tok)
			for _, spec := range v.Specs {
				ast.Print(nil, spec)
			}
		}
	}
}

const src = `package foo
type MyInt1 int
type MyInt2 = int
`

const src1 = `package foo
const Pi = 3.14
const E float64 = 2.71828
`

const src2 = `package foo
var Pi = 3.14
`
