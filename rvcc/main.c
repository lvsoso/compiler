#include "rvcc.h"

int main(int Argc, char **Argv)
{
  if (Argc != 2)
  {
    error("%s: invalid number of arguments", Argv[0]);
  }

  // parsing argv, generate token stream
  Token *Tok = tokenize(Argv[1]);

  // parse token stream
  Node *Nd = parse(Tok);

  codegen(Nd);

  return 0;
}
