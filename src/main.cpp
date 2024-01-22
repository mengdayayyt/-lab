#include <iostream>
#include <fstream>
#include <cassert>
#include <cstring>

#include "AST.hpp"
#include "ASM.hpp"
#include "koopa.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);

std::ofstream my_koopa;
std::ofstream my_riscv;

int main(int argc, const char *argv[])
{
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  if (strcmp(argv[3], "-o") != 0)
  {
    cout << "we need:build/compiler -koopa hello.cpp -o hello.koopa || build/compiler -riscv hello.cpp -o hello.s" << endl;
  }
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // mode<<endl;
  //  打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // parse input file
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);
  cout << "LOG 1" << endl;
  if (strcmp(mode, "-koopa") == 0)
  {
    // dump AST
    cout << "LOG 2" << endl;
    my_koopa = std::ofstream(output);
    cout << "LOG 3" << endl;
    cout << "ast is " << ast.get() << endl;
    ast->Dump();
    cout << "LOG 4" << endl;
    my_koopa.close();
  }
  cout << "LOG 5" << endl;

  if (strcmp(mode, "-riscv") == 0)
  {
    my_koopa = std::ofstream("riscv_tmp.koopa");
    ast->Dump();
    my_koopa.close();
    my_riscv = std::ofstream(output);

    // char *koopaIR = new char [1 << 20];
    // my__koopa.read(koopaIR, 1 << 20);

    std::ifstream my_koopa("riscv_tmp.koopa");
    std::string koopaIR((std::istreambuf_iterator<char>(my_koopa)), std::istreambuf_iterator<char>());
    char *koopa2IR = new char[koopaIR.length() + 1];
    std::strcpy(koopa2IR, koopaIR.c_str());

    parse_string(koopa2IR);

    delete[] koopa2IR;
  }
  //

  return 0;
}
