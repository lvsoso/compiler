#include "rvcc.h"

int main(int Argc, char **Argv)
{
  if (Argc != 2)
  {
    error("%s: invalid number of arguments", Argv[0]);
  }

  // parsing argv, generate token stream
  Token *Tok = tokenizeFile(Argv[1]);

  // parse token stream
  Obj *Prog = parse(Tok);

  codegen(Prog);

  return 0;
}
