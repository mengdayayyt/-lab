// 所有 AST 的基类
#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

extern std::ofstream my_koopa;
class ValType
{
public:
  enum class ValTypeEnum
  {
    const_type,//常量
    var_type,//变量
    void_,//无返回值函数
    int_,//有返回值函数
  } type;
  int val;//对应的值
  ValType()
  {
    type = ValTypeEnum::const_type;
    val = 0;
  };
  ValType(ValTypeEnum type_, int val_)
  {
    type = type_;
    val = val_;
  };
};

static std::unordered_map<std::string, ValType> Map[1024]; // symbol table for different depth
static int BlockDepth = 0;
static int now = 0;//寄存器的编号

static int if_end[1024];//每一层的块有没有返回

static std::unordered_map<std::string, int> NameMap[1024];//防止每一层的名字出现重复分配
static std::string name(std::string ident, int x)//根据名字和块的深度生成 名字
{
  return ident + "_" + std::to_string(x) + std::to_string(NameMap[x][ident]);
}

static int if_cnt = 0;
static int wh_cnt = 0;
static int whf[1024];
static int now_wh = 0;

static int flag=1;
static int num=0;

static void output_block_status()
{
cout << "blockdepth:" << BlockDepth << " 是否返回" << if_end[BlockDepth] << endl;

}
static void clear_block_status()
{
Map[BlockDepth].clear();
if_end[BlockDepth]=0;
}

/*

CompUnit ::= [CompUnit] (Decl | FuncDef);
Decl          ::= ConstDecl | VarDecl;
FuncDef     ::= FuncType IDENT "(" [FuncFParams] ")" Block;

Block         ::= "{" {BlockItem} "}";
BlockItem     ::= Decl | Stmt;


Stmt ::= LVal "=" Exp ";" 
        | [Exp] ";" 
        | Block 
        | "return" [Exp] ";"
        | "while" "(" Exp ")" Stmt
        | "break" ";"
        | "continue" ";"
        ｜"if" "(" Exp ")" Stmt ["else" Stmt];
  
FuncType    ::= "void" | "int";
FuncFParams ::= FuncFParam {"," FuncFParam};
FuncFParam  ::= BType IDENT;



Number    ::= INT_CONST;
PrimaryExp    ::= "(" Exp ")" | LVal | Number;
Exp         ::= LOrExp;
LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
LAndExp     ::= EqExp | LAndExp "&&" EqExp;
EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp ｜IDENT "(" [FuncRParams] ")";
FuncRParams ::= Exp {"," Exp};
UnaryOp     ::= "+" | "-" | "!";



ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
VarDecl       ::= BType VarDef {"," VarDef} ";";
BType         ::= "int";
ConstDef      ::= IDENT "=" ConstInitVal;
VarDef        ::= IDENT | IDENT "=" InitVal;
ConstInitVal  ::= ConstExp;
InitVal       ::= Exp;
ConstExp      ::= Exp;
LVal          ::= IDENT;

*/

class BaseAST;
class CompUnitAST;
class FuncDefAST;
class StmtAST;
class ExpAST;
class PrimaryExp1AST;
class UnaryOpAST;
class NumberAST;
class UnaryExpAST;
class UnaryExp1AST;
class MulExpAST;
class AddExpAST;
class MulExp1AST;
class AddExp1AST;
class UnaryOp1AST;
class UnaryOp2AST;
class UnaryOp3AST;
class UnaryOp4AST;
class RelExpAST;
class EqExpAST;
class LAndExpAST;
class LOrExpAST;
class RelExp1AST;
class EqExp1AST;
class LAndExp1AST;
class LOrExp1AST;

class DeclAST;
class ConstDeclAST;
class BTypeAST;
class ConstDefAST;
class ConstInitValAST;
class BlockAST;
class BlockItemAST;
class LValAST;
class LLValAST;
class PrimaryExpAST;
class ConstExpAST;
class VarDeclAST;
class VarDefAST;
class InitValAST;

class BaseAST
{
public:
  int number = 5;//常量的nuber
  std::string op;

  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;
};

class CompUnitAST : public BaseAST
{
public:
  std::vector<std::unique_ptr<BaseAST>> comp_units;// FuncDef or Decl 

  void Dump() const override
  {
    /* lib_funcs */
    my_koopa << "decl @getint(): i32\n";
    my_koopa << "decl @getch(): i32\n";
    my_koopa << "decl @getarray(*i32): i32\n";
    my_koopa << "decl @putint(i32)\n";
    my_koopa << "decl @putch(i32)\n";
    my_koopa << "decl @putarray(i32, *i32)\n";
    my_koopa << "decl @starttime()\n";
    my_koopa << "decl @stoptime()\n\n";
    //第0层 为全局都可使用的函数
    Map[0]["getint"] = Map[0]["getch"] = Map[0]["getarray"] = ValType(ValType::ValTypeEnum::int_, 0);
    Map[0]["putint"] = Map[0]["putch"] = Map[0]["putarray"] = ValType(ValType::ValTypeEnum::void_, 0);

    for (auto &ptr : this->comp_units)
    {
      ptr->Dump();
    }
  }
};
class FuncDefAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::vector<std::unique_ptr<BaseAST>> func_params;

  void Dump() const override
  {
    my_koopa << "fun "
             << "@" << ident << "(";

    for (auto it = this->func_params.begin(); it != this->func_params.end(); ++it)
    {
      if (it != this->func_params.begin())
        my_koopa << ", ";
      (*it)->number = 0;//参数初始化为0
      (*it)->Dump();
    }
    my_koopa << ")";

    if (func_type->number == 1)
    {
      Map[0][ident] = ValType(ValType::ValTypeEnum::int_, 0);//把函数加入符号表
      my_koopa << ": i32 ";
    }
    else
    {
      Map[0][ident] = ValType(ValType::ValTypeEnum::void_, 0); 
    }

    my_koopa << "{"
             << "\n";
    my_koopa << "%entry:\n";

    for (auto it = this->func_params.begin(); it != this->func_params.end(); ++it)
    {
      (*it)->number = 1;
      (*it)->Dump();
    }

    
    BlockDepth++;
    cout<<"-------*0*---\n";
    output_block_status();
    block->Dump();
    
    cout<<"-------*0*---\n";
    output_block_status();

    if (!if_end[BlockDepth])
    {
      if (Map[0][ident].type == ValType::ValTypeEnum::int_)
        my_koopa << "    ret 0\n";
      else
        my_koopa << "   ret\n";
    }

    my_koopa << "}"
             << "\n";
    clear_block_status();   
    BlockDepth--;   
    if_end[BlockDepth] = 0;
  }
};
class FuncParamAST : public BaseAST
{
public:
  std::string ident;

  void Dump() const override
  {
    if (number == 0)
      my_koopa << "@" << ident << ":i32";
    if (number == 1)
    {
      NameMap[BlockDepth][ident]++;
      my_koopa << "    @" << name(ident, BlockDepth) << "= alloc i32\n";
      my_koopa << "    store @" << ident << ", @" << name(ident, BlockDepth) << "\n";
      Map[BlockDepth][ident] = ValType(ValType::ValTypeEnum::var_type, 0);
      cout << "BlockDepth" << Map[0].count(ident) << endl;
    }
  }
};

class ExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};

class UnaryExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  std::string type;  //
  std::string ident; //
  std::vector<std::unique_ptr<BaseAST>> call_params;

  void Dump() const override
  {
    cout << "++++++++++++++++++" << type << " " << ident << endl;
    if (type == "none")
    {
      exp->Dump();
    }
    else if (type == "morecall" || ident == "getarray" || ident == "putint" || ident == "putch" || ident == "putarray")
    {
      std::vector<int> Paras;

      for (auto it = this->call_params.begin(); it != this->call_params.end(); ++it)
      {
        (*it)->Dump();
        Paras.push_back(now - 1);
      }

      if (Map[0][ident].type == ValType::ValTypeEnum::void_)
      {
        my_koopa << "    call @" << ident << "(\n";
      }
      else
      {
        my_koopa << "    %" << now << "=call @" << ident << "(";
        now++;
      }

      for (auto it = Paras.begin(); it != Paras.end(); ++it)
      {
        if (it != Paras.begin())
          my_koopa << ",";
        my_koopa << "%" << (*it);
      }
      my_koopa << ")\n";
    }
    else if (type == "singlecall" || ident == "starttime" || ident == "stoptime")
    {
      if (Map[0][ident].type == ValType::ValTypeEnum::void_)
      {
        my_koopa << "    call @" << ident << "()\n";
      }
      else
      {
        my_koopa << "    %" << now << "=call @" << ident << "()\n";
        now++;
      }
    }
  }
};

class UnaryExp1AST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp1;
  std::unique_ptr<BaseAST> exp2;

  void Dump() const override
  {
    exp2->Dump();
    if (exp1->op == "-")
    {
      my_koopa << '%' << now << "= "
               << "sub 0," << '%' << now - 1 << std::endl;
      ++now;
    }
    else if (exp1->op == "+")
    {
      my_koopa << '%' << now << "= "
               << "add 0," << '%' << now - 1 << std::endl;
      ++now;
    }
    else if (exp1->op == "!")
    {
      my_koopa << '%' << now << "= "
               << "eq 0," << '%' << now - 1 << std::endl;
      ++now;
    }
  }
};

class UnaryOpAST : public BaseAST
{
public:
  void Dump() const override
  {
  }
};
class UnaryOp1AST : public BaseAST
{
public:
  void Dump() const override
  {
  }
};
class UnaryOp2AST : public BaseAST
{
public:
  void Dump() const override
  {
  }
};

class UnaryOp3AST : public BaseAST
{
public:
  void Dump() const override
  {
  }
};
class UnaryOp4AST : public BaseAST
{
public:
  void Dump() const override
  {
  }
};

class UnaryOp5AST : public BaseAST
{
public:
  void Dump() const override
  {
  }
};

class UnaryOp6AST : public BaseAST
{
public:
  void Dump() const override
  {
  }
};

class MulExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};
class MulExp1AST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp1;
  std::unique_ptr<BaseAST> exp2;
  std::unique_ptr<BaseAST> exp3;

  void Dump() const override
  {
    exp1->Dump();
    int now1 = now - 1;
    exp3->Dump();
    if (exp2->op == "*")
    {
      my_koopa << "%" << now << "= mul %" << now1 << ", %" << now - 1 << "\n";
    }
    else if (exp2->op == "/")
    {
      my_koopa << "%" << now << "= div %" << now1 << ", %" << now - 1 << "\n";
    }
    else if (exp2->op == "%")
    {
      my_koopa << "%" << now << "= mod %" << now1 << ", %" << now - 1 << "\n";
    }
    now++;
  }
};

class AddExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};
class AddExp1AST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp1;
  std::unique_ptr<BaseAST> exp2;
  std::unique_ptr<BaseAST> exp3;
  void Dump() const override
  {
    exp1->Dump();
    int now1 = now - 1;
    exp3->Dump();
    if (exp2->op == "+")
    {
      my_koopa << "%" << now << "= add %" << now1 << ", %" << now - 1 << "\n";
    }
    else if (exp2->op == "-")
    {
      my_koopa << "%" << now << "= sub %" << now1 << ", %" << now - 1 << "\n";
    }
    now++;
  }
};

class RelExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};
class RelExp1AST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp1;
  std::unique_ptr<BaseAST> exp2;
  std::unique_ptr<BaseAST> exp3;

  void Dump() const override
  {
    exp1->Dump();
    int now1 = now - 1;
    exp3->Dump();
    if (exp2->op == "<")
    {
      my_koopa << "%" << now << "= lt %" << now1 << ", %" << now - 1 << "\n";
    }
    else if (exp2->op == ">")
    {
      my_koopa << "%" << now << "= gt %" << now1 << ", %" << now - 1 << "\n";
    }
    else if (exp2->op == "<=")
    {
      my_koopa << "%" << now << "= le %" << now1 << ", %" << now - 1 << "\n";
    }
    else if (exp2->op == ">=")
    {
      my_koopa << "%" << now << "= ge %" << now1 << ", %" << now - 1 << "\n";
    }
    now++;
  }
};

class EqExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};
class EqExp1AST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp1;
  std::unique_ptr<BaseAST> exp2;
  std::unique_ptr<BaseAST> exp3;

  void Dump() const override
  {
    exp1->Dump();
    int now1 = now - 1;
    exp3->Dump();
    if (exp2->op == "==")
    {
      my_koopa << "%" << now << "= eq %" << now1 << ", %" << now - 1 << "\n";
    }
    else if (exp2->op == "!=")
    {
      my_koopa << "%" << now << "= ne %" << now1 << ", %" << now - 1 << "\n";
    }
    now++;
  }
};
class LAndExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};
class LAndExp1AST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp1;
  std::unique_ptr<BaseAST> exp2;
  std::unique_ptr<BaseAST> exp3;

  // void Dump() const override
  // {
  //   exp1->Dump();
  //   int now1 = now - 1;

  //   exp3->Dump();
  //   int now2 = now - 1;
  //   my_koopa << "%" << now << "= ne "
  //            << "0, %" << now1 << "\n"; // 5
  //   now++;                              // 6
  //   my_koopa << "%" << now << "= ne "
  //            << "0, %" << now2 << "\n"; // 6
  //   now++;                              // 7
  //   my_koopa << "%" << now << "= and %" << now - 1 << ", %" << now - 2 << "\n";
  //   now++;

  // }
  void Dump() const override
  {
    // a && b
    exp1->Dump();
    int now1 = now - 1;

    int temp = now;
    my_koopa << "@result_" << temp << " = alloc i32\n";
    my_koopa << "%" << now << "= ne 0, %" << now1 << "\n";
    my_koopa << "store %" << now << ", @result_"
             << temp << "\n";
    now++;

    if_cnt++;
    int now_if = if_cnt;

    my_koopa << "br %" << now1 << ", %then" << now_if << ", %end" << now_if << "\n";
    my_koopa << "\n";
    my_koopa << "%then" << now_if << ":\n";
    // a == 1
    exp3->Dump();
    int now2 = now - 1;
    my_koopa << "%" << now << "= ne "
             << "0, %" << now2 << "\n";
    now++;
    my_koopa << "store " << '%' << now - 1 << ", @result_" << temp << "\n";
    my_koopa << "jump %end" << now_if << "\n";
    // a != 1
    my_koopa << "\n";
    my_koopa << "%end" << now_if << ":\n";
    my_koopa << "%" << now << "= load @result_" << temp << "\n";
    now++;
  }
};
class LOrExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};
class LOrExp1AST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp1;
  std::unique_ptr<BaseAST> exp2;
  std::unique_ptr<BaseAST> exp3;

  // void Dump() const override
  // {
  //   exp1->Dump();
  //   int now1 = now - 1;

  //   exp3->Dump();
  //   int now2 = now - 1;
  //   my_koopa << "%" << now << "= ne "
  //            << "0, %" << now1 << "\n"; // 5
  //   now++;                              // 6
  //   my_koopa << "%" << now << "= ne "
  //            << "0, %" << now2 << "\n"; // 6
  //   now++;                              // 7
  //   my_koopa << "%" << now << "= or %" << now - 1 << ", %" << now - 2 << "\n";
  //   now++;
  //  }
  void Dump() const override
  {
    // exp1 || exp3
    exp1->Dump();
    int now1 = now - 1;

    int temp = now;
    my_koopa << "@result_" << temp << " = alloc i32\n";
    my_koopa << "%" << now << "= ne 0, %" << now1 << "\n";
    my_koopa << "store %" << now << ", @result_" << temp << "\n";
    now++;

    if_cnt++;
    int now_if = if_cnt;

    my_koopa << "br %" << now1 << ", %end" << now_if << ", %then" << now_if << "\n";
    my_koopa << "\n";
    my_koopa << "%then" << now_if << ":\n";

    exp3->Dump();
    int now2 = now - 1;
    my_koopa << "%" << now << "= ne 0, %" << now2 << "\n";
    now++;
    my_koopa << "store " << '%' << now - 1 << ", @result_" << temp << "\n";
    my_koopa << "jump %end" << now_if << "\n";

    my_koopa << "\n";
    my_koopa << "%end" << now_if << ":\n";
    my_koopa << "%" << now << "= load @result_" << temp << "\n";
    now++;
  }
};

class DeclAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};


class ConstDeclAST : public BaseAST
{
public:
  std::vector<std::unique_ptr<BaseAST>> const_defs;

  void Dump() const override
  {
    for (auto &ptr : this->const_defs)
    {
      ptr->Dump();
    }
  };
};

class BTypeAST : public BaseAST
{
public:
  std::string op; // 代表类型信息

  void Dump() const override
  {
  }
};

// 常量定义类 ConstDef      ::= IDENT "=" ConstInitVal;
class ConstDefAST : public BaseAST
{
public:
  std::string ident;
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    // 将 IDENT 和对应的 ast->exp->number 加入到 Map 中
    Map[BlockDepth][ident] = ValType(ValType::ValTypeEnum::const_type, exp->number);
    std::cout << "**" << ident << " " << exp->number << std::endl;
  }
};
// 常量初始化值类 ConstInitVal  ::= ConstExp;
class ConstInitValAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};

// Block         ::= "{" {BlockItem} "}";
class BlockAST : public BaseAST
{
public:
  std::vector<std::unique_ptr<BaseAST>> block_items;

  void Dump() const override
  {

    for (auto &ptr : this->block_items)
    {
      cout<<"^^^^^^^^^^^^^^^^^^"<<BlockDepth<<endl;
      ptr->Dump();
    }

    my_koopa << "\n";

  }
};

// 块项类BlockItem     ::= Decl | Stmt;
class BlockItemAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    if (if_end[BlockDepth])
      return;

    exp->Dump();
  }
};

// 左值类 LVal::= IDENT;
class LValAST : public BaseAST
{
public:
  std::string ident;

  void Dump() const override
  {
    cout << "11BlockDepth" << BlockDepth << endl;
    cout << Map[0].count(ident) << endl;
    for (int i = BlockDepth; i >= 0; i--)
    {
      if (Map[i].count(ident) > 0)
      {
        const ValType &valType = Map[i][ident];
        if (valType.type == ValType::ValTypeEnum::var_type)
        {
          my_koopa << "%" << now << " = load @" << name(ident, i) << std::endl;
          break;
        }
        else
        {
          cout << "*******2" << ident << std::endl;
          my_koopa << "%" << now << " = add 0, " << valType.val << std::endl;
          break;
        }
      }
    }

    now++;
  }
};
class LLValAST : public BaseAST
{
public:
  std::string ident;

  void Dump() const override
  {
    for (int i = BlockDepth; i >= 0; i--)
    {
      if (Map[i].count(ident) > 0)
      {
        const ValType &valType = Map[i][ident];
        if (valType.type == ValType::ValTypeEnum::var_type)
        {
          my_koopa << "store %" << now - 1 << ", @" << name(ident, i) << std::endl;
          break;
        }
      }
    }
  }
};

// PrimaryExp    ::= "(" Exp ")" | LVal | Number;
class PrimaryExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};
class PrimaryExp1AST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};

// 常量表达式类 ConstExp::= Exp;
class ConstExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }
};

class NumberAST : public BaseAST
{
public:
  void Dump() const override
  {
    my_koopa << "%" << now << "= add "
             << "0 ," << number << std::endl;
    now++;
  }
};


class VarDeclAST : public BaseAST
{
public:
  std::vector<std::unique_ptr<BaseAST>> var_defs;

  void Dump() const override
  {
    for (auto &ptr : this->var_defs)
    {
      ptr->Dump();
    }
  };
};
class VarDefAST : public BaseAST
{
public:
  std::string ident;
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    cout << "LOG 10.444" << endl;
    if (BlockDepth == 0)
    { // 全局变量
      int res = 0;
      if (exp != nullptr)
      {
        // exp->Dump();
        res = exp->number;
      }

      NameMap[BlockDepth][ident]++;
      my_koopa << "global @" << name(ident, BlockDepth) << " = alloc i32, " << res << "\n";

      if (exp != nullptr)
      {
        //  my_koopa << "store %" << now - 1 << ", @" << name(ident, BlockDepth) << "\n";
        Map[BlockDepth][ident] = ValType(ValType::ValTypeEnum::var_type, exp->number);
      }
      else
      {
        Map[BlockDepth][ident] = ValType(ValType::ValTypeEnum::var_type, 0);
      }
    }
    else
    {
      if (exp != nullptr)
      {
        exp->Dump();
      }
      NameMap[BlockDepth][ident]++;
      my_koopa << "@" << name(ident, BlockDepth) << " = alloc i32\n";

      if (exp != nullptr)
      {
        my_koopa << "store %" << now - 1 << ", @" << name(ident, BlockDepth) << "\n";
        Map[BlockDepth][ident] = ValType(ValType::ValTypeEnum::var_type, exp->number);
      }
      else
      {
        Map[BlockDepth][ident] = ValType(ValType::ValTypeEnum::var_type, 0);
      }
    }
  }
};

class InitValAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;
  void Dump() const override
  {
    exp->Dump();
  }
};

// Stmt ::= LVal "=" Exp ";" | [Exp] ";" | Block | "return" [Exp] ";";
class StmtAST : public BaseAST
{
public:
  enum class Stmt_type
  {
    lvalexp,
    exp,
    exp0,
    block,
    return_,
    return0,
    if_,
    if0,
    while_,
    continue_,
    break_,
  } type;

  std::unique_ptr<BaseAST> exp; // exp or block
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> exp_stmt;
  std::unique_ptr<BaseAST> else_stmt;
  std::string str_;

  void Dump() const override
  {
// LLVal '=' Exp ';' 
    if (type == StmtAST::Stmt_type::lvalexp)
    {
      exp->Dump();
      lval->Dump();
    }
//  RETURN Exp ';' 
    else if (type == StmtAST::Stmt_type::return_)
    {
      exp->Dump();
      my_koopa << "  "
               << "ret"
               << " "
               << "%" << now - 1 << "\n";
      if_end[BlockDepth] = 1;
      cout<<"yyt0"<<endl;
      output_block_status();
    }
    else if (type == StmtAST::Stmt_type::return0)
    {

      exp->Dump();
      my_koopa << "  "
               << "ret\n";
      if_end[BlockDepth] = 1;
           output_block_status();
    }
    else if (type == StmtAST::Stmt_type::block)
    {
    
       num++;
       int num_=num;
       if (!if_end[BlockDepth])
        my_koopa << "jump %starttt" << num_ << "\n";
       my_koopa << "%starttt" << num_ << ":\n";
       BlockDepth++;
       cout<<"yyt:1";
       output_block_status();
       exp->Dump();
       cout<<"yyt:2";
       output_block_status();
       if (!if_end[BlockDepth])
       my_koopa << "jump %enddd" << num_ << "\n";
      my_koopa << "\n";
      my_koopa << "%enddd" << num_ << ":\n";
      
       clear_block_status();
        BlockDepth--;

//good;
      // exp->Dump();

    }
    //IF '(' Exp ')' Stmt
    else if (type == StmtAST::Stmt_type::if0)
    {

      if_cnt++;
      int now_if = if_cnt;
      exp->Dump();
      my_koopa << "br %" << now - 1 << ", %then" << now_if << ", %end" << now_if << "\n";
      my_koopa << "\n";


      my_koopa << "%then" << now_if << ":\n";
      BlockDepth++;
      exp_stmt->Dump();
      //
      // cout << "-------*2*---\n";
      // output_block_status();

       if (!if_end[BlockDepth])
       my_koopa << "jump %end" << now_if << "\n";
       my_koopa << "\n";

      my_koopa << "%end" << now_if << ":\n";
      
      clear_block_status();
       BlockDepth--;

      // cout << "-------*3*---\n";
      // output_block_status();
    }
    //IF '(' Exp ')' Stmt ELSE Stmt
    else if (type == StmtAST::Stmt_type::if_)
    {
      if_cnt++;
      int now_if = if_cnt;
      exp->Dump();
      my_koopa << "br %" << now - 1 << ", %then" << now_if << ", %else" << now_if << "\n";
      my_koopa << "\n";


      my_koopa << "%then" << now_if << ":"
               << "\n";
     BlockDepth++;
      exp_stmt->Dump();
      if (!if_end[BlockDepth])
        my_koopa << "jump %end" << now_if << "\n";
      my_koopa << "\n";
       clear_block_status();
       BlockDepth--;


      my_koopa << "%else" << now_if << ":\n";
      BlockDepth++;
      else_stmt->Dump();
      if (!if_end[BlockDepth])
        my_koopa << "jump %end" << now_if << "\n";
      my_koopa << "\n";
      my_koopa << "%end" << now_if << ":\n";
     
  
       clear_block_status();
       BlockDepth--;

    }
    //WHILE '(' Exp ')' Stmt 
    else if (type == StmtAST::Stmt_type::while_)
    {
      wh_cnt++;
      whf[wh_cnt] = now_wh;
      now_wh = wh_cnt;

      if (!if_end[BlockDepth])
        my_koopa << "jump %whilecheck" << now_wh << "\n";
      my_koopa << "\n";
      my_koopa << "%whilecheck" << now_wh << ":\n";

      
      exp->Dump();
      if (!if_end[BlockDepth])
        my_koopa << "\tbr %" << now - 1 << ", %whilethen" << now_wh << ", %endwhile" << now_wh << "\n";
      my_koopa << "\n";
      my_koopa << "%whilethen" << now_wh << ":"
               << "\n";

      BlockDepth++;
      exp_stmt->Dump();

      if (!if_end[BlockDepth])
        my_koopa << "\tjump %whilecheck" << now_wh << "\n";

      my_koopa << "\n";
      my_koopa << "%endwhile" << now_wh << ":"
               << "\n";

       clear_block_status();
       BlockDepth--;
      now_wh = whf[now_wh];
    }
    else if (type == StmtAST::Stmt_type::continue_)
    {
      my_koopa << "jump %whilecheck" << now_wh << "\n";
      if_end[BlockDepth] = 1;
    }
    else if (type == StmtAST::Stmt_type::break_)
    {

      my_koopa << "jump %endwhile" << now_wh << "\n";
      if_end[BlockDepth] = 1;
    }
    else if (type == StmtAST::Stmt_type::exp)
    {
      exp->Dump();
    }
  }
};
