package main

import (
	"go/ast"
	"go/parser"
)

func main() {
	expr, _ := parser.ParseExpr(`"9257"`)
	ast.Print(nil, expr)

	expr, _ = parser.ParseExpr(`x`)
	ast.Print(nil, expr)
}
