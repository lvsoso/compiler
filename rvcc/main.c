#include "rvcc.h"

// target file path
static char *OptO;

// input file path
static char *InputPath;

static void usage(int Status)
{
  fprintf(stderr, "rvcc [ -o  <path> ] <file>\n");
  exit(Status);
}

// parse args
static void parseArgs(int Argc, char **Argv)
{
  for (int l = 1; l < Argc; l++)
  {
    if (!strcmp(Argv[l], "--help"))
    {
      usage(0);
    }

    if (!strcmp(Argv[l], "-o"))
    {
      if (!Argv[++l])
      {
        usage(1);
      }

      OptO = Argv[l];
      continue;
    }

    // parse "-oxxx"
    if (!strncmp(Argv[l], "-o", 2))
    {
      // 目标文件的路径
      OptO = Argv[l] + 2;
      continue;
    }

    // parse "-"
    if (Argv[l][0] == '-' && Argv[l][1] != '\0')
      error("unknown argument: %s", Argv[l]);

    InputPath = Argv[l];
  }

  if (!InputPath)
  {
    error("no input files");
  }
}

static FILE *openFile(char *Path) {
  if (!Path || strcmp(Path, "-") == 0)
    return stdout;

  // open file use "w" mode
  FILE *Out = fopen(Path, "w");
  if (!Out)
    error("cannot open output file: %s: %s", Path, strerror(errno));
  return Out;
}

int main(int Argc, char **Argv)
{
  parseArgs(Argc, Argv);

  // parsing argv, generate token stream
  Token *Tok = tokenizeFile(InputPath);

  // parse token stream
  Obj *Prog = parse(Tok);

  // 生成代码
  FILE *Out = openFile(OptO);

  // .file file-no filename
  fprintf(Out, ".file 1 \"%s\"\n", InputPath);
  codegen(Prog, Out);
  return 0;
}
