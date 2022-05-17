package main

import (
	"fmt"
	"go/ast"
	"go/parser"
)

func main() {

	expr, _ := parser.ParseExpr(`func(){}`)
	ast.Print(nil, expr)

	fmt.Println("-----------------------------")

	expr, _ = parser.ParseExpr(`[...]int{1,2:3}`)
	ast.Print(nil, expr)

	fmt.Println("-----------------------------")
	expr, _ = parser.ParseExpr(`struct{X int}{X:1}`)
	ast.Print(nil, expr)

	fmt.Println("-----------------------------")
	expr, _ = parser.ParseExpr(`struct{X int}{1}`)
	ast.Print(nil, expr)
}
