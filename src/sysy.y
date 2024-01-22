%code requires {
  #include <memory>
  #include <string>
  #include<cstring>
  #include"AST.hpp"
  #include <vector>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include"AST.hpp"

extern std::unordered_map<std::string, ValType> Map[1024]; 
extern int BlockDepth;
// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况

%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  std::vector<std::unique_ptr<BaseAST>> *base_vec_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN OP1 OP2 OP3 OP4 OP5 OP6 OP7 OP8 OP9 OP10 OP11 OP12 OP13 OP14 CONST IF ELSE WHILE BREAK CONTINUE VOID
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
//%type <ast_val> FuncDef Block Stmt
%type <base_vec_val> MoreBlockItems MoreConstDefs MoreVarDefs MoreCompUnits MoreFuncParams MoreCallParams
%type <ast_val> FuncDef Block Stmt Exp PrimaryExp PrimaryExp1 UnaryOp Number UnaryExp UnaryExp1 MulExp AddExp MulExp1 AddExp1 UnaryOp1 UnaryOp2 UnaryOp3 UnaryOp4 RelExp EqExp LAndExp LOrExp  RelExp1 EqExp1 LAndExp1 LOrExp1 UnaryOp5 UnaryOp6 Decl ConstDecl BType ConstDef ConstInitVal BlockItem LVal LLVal ConstExp VarDecl VarDef InitVal FuncParam CompUnit1; 
%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担

 CompUnit
  : MoreCompUnits CompUnit1 {
     auto comp_unit = make_unique<CompUnitAST>();
    ($1)->push_back(unique_ptr<BaseAST>($2));
    comp_unit->comp_units = std::move(*($1));
    ast = std::move(comp_unit);
  };

MoreCompUnits:
  MoreCompUnits CompUnit1 {
    ($1)->push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };

CompUnit1:
  FuncDef {
    $$ = $1;
  } |
  Decl {
    $$ = $1;
  } ;



FuncDef
  :BType IDENT '(' FuncParam MoreFuncParams ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    
    ast->ident = *unique_ptr<string>($2);
    ($5)->insert(($5)->begin(), unique_ptr<BaseAST>($4));
    ast->func_params = std::move(*($5));
    ast->block = unique_ptr<BaseAST>($7);
    ast->block->op="x";
    $$ = ast;
  } |
  BType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    cout<<ast->func_type->number<<"******()()()"<<endl;
    cout<<$1<<endl;
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    ast->block->op="x";
    $$ = ast;
  } 
  ;
 
MoreFuncParams:
  MoreFuncParams ',' FuncParam {
    ($1)->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };

FuncParam:
  BType IDENT {
    auto ast = new FuncParamAST();
    ast->ident = *unique_ptr<string>($2);
     cerr << "error: 161" << endl;
    $$ = ast;
  } 
  ;


Number
  : INT_CONST {
   // $$ = new string(to_string($1));
  auto ast=new NumberAST();
  ast->number=($1);
  $$=ast;
  }
  ;


Stmt: 
  LLVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Stmt_type::lvalexp;
    ast->lval = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  } |
  RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Stmt_type::return_;
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  } | 
  RETURN ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Stmt_type::return0;
    ast->exp = nullptr;
    $$ = ast;
  } |
  Block {
    auto ast = new StmtAST();
    ast->type = StmtAST::Stmt_type::block;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } |
  Exp ';' {
    auto ast = new StmtAST();
    ast->type = StmtAST::Stmt_type::exp;
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  } |
  ';' {
    auto ast = new StmtAST();
   ast->type = StmtAST::Stmt_type::exp0;
   ast->exp = nullptr;
    $$ = ast;
  }|IF '(' Exp ')' Stmt {
    auto ast = new StmtAST();
   ast->type = StmtAST::Stmt_type::if0;
   ast->exp = unique_ptr<BaseAST>($3);
   ast->exp_stmt = unique_ptr<BaseAST>($5);
   ast->exp_stmt ->op="x";
    $$ = ast;
  }|IF '(' Exp ')' Stmt ELSE Stmt {
  auto ast = new StmtAST();
   ast->type = StmtAST::Stmt_type::if_;
   ast->exp = unique_ptr<BaseAST>($3);
   ast->exp_stmt = unique_ptr<BaseAST>($5);
   ast->else_stmt = unique_ptr<BaseAST>($7);
   ast->exp_stmt->op="x";
   ast->else_stmt ->op="x";
    $$ = ast;
  }|WHILE '(' Exp ')' Stmt  {
     auto ast = new StmtAST();
      ast->type = StmtAST::Stmt_type::while_;
      ast->exp = unique_ptr<BaseAST>($3);
       ast->exp_stmt = unique_ptr<BaseAST>($5);
        ast->exp_stmt->op="x";
        $$ = ast;
  }| CONTINUE ';' {
auto ast = new StmtAST();
 ast->str_="continue";
 ast->type = StmtAST::Stmt_type::continue_;
 $$ = ast;
  }| BREAK ';'{
auto ast = new StmtAST();
ast->type = StmtAST::Stmt_type::break_;
ast->str_="break";
 $$ = ast;
  }
  ;

Exp
:LOrExp{
   auto ast=new ExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
}
;

UnaryExp
: PrimaryExp{
    auto ast=new UnaryExpAST();
    ast->type = "none";
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
} | UnaryExp1{
    auto ast=new UnaryExpAST();  
    ast->type = "none";
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
}| IDENT '(' Exp MoreCallParams ')' {
    auto ast = new UnaryExpAST();
    
    ast->type = "morecall";
    ast->ident = *unique_ptr<string>($1);
    cout<<"______________"<<ast->ident<<endl;
    ($4)->insert(($4)->begin(), unique_ptr<BaseAST>($3));
    ast->call_params = std::move(*($4));
    $$ = ast;
  } |
  IDENT '(' ')' {
    auto ast = new UnaryExpAST();
    ast->type = "singlecall";
    ast->ident = *unique_ptr<string>($1);
    ast->call_params = std::move(*(new vector<unique_ptr<BaseAST>>()));
    $$ = ast;
  }
  ;

MoreCallParams:
  MoreCallParams ',' Exp {
    ($1)->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };


UnaryExp1
:UnaryOp UnaryExp{
    auto ast=new UnaryExp1AST();  
    ast->exp1=unique_ptr<BaseAST>($1);
    ast->exp2=unique_ptr<BaseAST>($2);
    if(ast->exp1->op=="+")
    {
      ast->number=ast->exp2->number;
    }
    else if(ast->exp1->op=="-")
    {
      ast->number=-(ast->exp2->number);
    }
    else if(ast->exp1->op=="!")
    {
      ast->number=((ast->exp2->number)==0);
    }
    $$=ast;
}
;

UnaryOp
: OP1{
  auto stmt =new UnaryOpAST();
  stmt->op = *unique_ptr<string>(new string("+"));
 $$ = move(stmt);
} | OP2{

  auto stmt =new UnaryOpAST();
  stmt->op = *unique_ptr<string>(new string("-"));
   $$ = move(stmt);
} | OP3{

  auto stmt =new UnaryOpAST();
  stmt->op = *unique_ptr<string>(new string("!"));
   $$ = move(stmt);
}
;

UnaryOp1
: OP1{
  auto stmt =new UnaryOp1AST();
  stmt->op = *unique_ptr<string>(new string("+"));
 $$ = move(stmt);
} | OP2{

  auto stmt =new UnaryOp1AST();
  stmt->op = *unique_ptr<string>(new string("-"));
   $$ = move(stmt);
}
;

UnaryOp2
: OP4{
  auto stmt =new UnaryOp2AST();
  stmt->op = *unique_ptr<string>(new string("*"));
 $$ = move(stmt);
} | OP5{

  auto stmt =new UnaryOp2AST();
  stmt->op = *unique_ptr<string>(new string("/"));
   $$ = move(stmt);
} | OP6{

  auto stmt =new UnaryOp2AST();
  stmt->op = *unique_ptr<string>(new string("%"));
   $$ = move(stmt);
}
;

UnaryOp3
: OP7{
  auto stmt =new UnaryOp1AST();
  stmt->op = *unique_ptr<string>(new string("<"));
 $$ = move(stmt);
} | OP8{

  auto stmt =new UnaryOp1AST();
  stmt->op = *unique_ptr<string>(new string(">"));
   $$ = move(stmt);
}
| OP9{

  auto stmt =new UnaryOp1AST();
  stmt->op = *unique_ptr<string>(new string("<="));
   $$ = move(stmt);
}
| OP10{

  auto stmt =new UnaryOp1AST();
  stmt->op = *unique_ptr<string>(new string(">="));
   $$ = move(stmt);
}
;

UnaryOp4
: OP11{
  auto stmt =new UnaryOp1AST();
  stmt->op = *unique_ptr<string>(new string("=="));
 $$ = move(stmt);
} | OP12{

  auto stmt =new UnaryOp1AST();
  stmt->op = *unique_ptr<string>(new string("!="));
   $$ = move(stmt);
}
;

UnaryOp5
: OP13{
  auto stmt =new UnaryOp5AST();
  stmt->op = *unique_ptr<string>(new string("||"));
 $$ = move(stmt);
}
;

UnaryOp6
:OP14 {
  auto stmt =new UnaryOp6AST();
  stmt->op = *unique_ptr<string>(new string("&&"));
   $$ = move(stmt);
}
;
//AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
//MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;

MulExp
: UnaryExp{
    auto ast=new MulExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
} | MulExp1{
    auto ast=new MulExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
}
;

MulExp1
:MulExp UnaryOp2 UnaryExp {
   auto ast=new MulExp1AST();  
    ast->exp1=unique_ptr<BaseAST>($1);
    ast->exp2=unique_ptr<BaseAST>($2);
    ast->exp3=unique_ptr<BaseAST>($3);
    if(ast->exp2->op=="*")
    {
      ast->number=(ast->exp1->number)*(ast->exp3->number);
    }
    else if(ast->exp2->op=="/")
    {
      if((ast->exp3->number)!=0)
      {ast->number=(ast->exp1->number)/(ast->exp3->number);
      }
    }
    else if(ast->exp2->op=="%")
    {
      ast->number=(ast->exp1->number)%(ast->exp3->number);
    }
    $$=ast;
}
;

AddExp
: MulExp{
    auto ast=new AddExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
} | AddExp1{
    auto ast=new AddExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
}
;

AddExp1
:AddExp UnaryOp1 MulExp {
   auto ast=new AddExp1AST();  
    ast->exp1=unique_ptr<BaseAST>($1);
    ast->exp2=unique_ptr<BaseAST>($2);
    ast->exp3=unique_ptr<BaseAST>($3);
    if(ast->exp2->op=="+")
    {
      ast->number=(ast->exp1->number)+(ast->exp3->number);
    }
    else if(ast->exp2->op=="-")
    {
      ast->number=(ast->exp1->number)-(ast->exp3->number);
    }
    $$=ast;
}
;

/*
RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
*/


RelExp
: AddExp{
    auto ast=new RelExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
} | RelExp1{
    auto ast=new RelExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
}
;

RelExp1
:RelExp UnaryOp3 AddExp{
   auto ast=new RelExp1AST();  
    ast->exp1=unique_ptr<BaseAST>($1);
    ast->exp2=unique_ptr<BaseAST>($2);
    ast->exp3=unique_ptr<BaseAST>($3);
    if(ast->exp2->op=="<")
    {
      ast->number=((ast->exp1->number)<(ast->exp3->number));
    }
    else if(ast->exp2->op==">")
    {
      ast->number=((ast->exp1->number)>(ast->exp3->number));
    }
    else if(ast->exp2->op=="<=")
    {
      ast->number=((ast->exp1->number)<=(ast->exp3->number));
    }
    else if(ast->exp2->op==">=")
    {
      ast->number=((ast->exp1->number)>=(ast->exp3->number));
    }
    $$=ast;
}
;


EqExp
: RelExp{
    auto ast=new EqExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
} | EqExp1{
    auto ast=new EqExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
}
;

EqExp1
:EqExp UnaryOp4 RelExp{
   auto ast=new EqExp1AST();  
    ast->exp1=unique_ptr<BaseAST>($1);
    ast->exp2=unique_ptr<BaseAST>($2);
    ast->exp3=unique_ptr<BaseAST>($3);
    if(ast->exp2->op=="==")
    {
      ast->number=((ast->exp1->number)==(ast->exp3->number));
    }
    else if(ast->exp2->op=="!=")
    {
      ast->number=((ast->exp1->number)!=(ast->exp3->number));
    }
    $$=ast;
}
;

/*
LAndExp     ::= EqExp | LAndExp "&&" EqExp;
LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
*/

LAndExp
: EqExp{
    auto ast=new LAndExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
} | LAndExp1{
    auto ast=new LAndExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
}
;

LAndExp1
:LAndExp UnaryOp6 EqExp{
   auto ast=new LAndExp1AST();  
    ast->exp1=unique_ptr<BaseAST>($1);
     ast->exp2=unique_ptr<BaseAST>($2);
    ast->exp3=unique_ptr<BaseAST>($3);
    ast->number=((ast->exp1->number)&&(ast->exp3->number));

    $$=ast;
}
;

LOrExp
: LAndExp{
    auto ast=new LOrExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
} | LOrExp1{
    auto ast=new LOrExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;
}
;

LOrExp1
:LOrExp UnaryOp5 LAndExp{
   auto ast=new LOrExp1AST();  
     ast->exp1=unique_ptr<BaseAST>($1);
     ast->exp2=unique_ptr<BaseAST>($2);
    ast->exp3=unique_ptr<BaseAST>($3);
    ast->number=((ast->exp1->number)||(ast->exp3->number));

    $$=ast;
}
;






Decl
:ConstDecl{
  auto ast = new DeclAST();
  ast->exp=unique_ptr<BaseAST>($1);
  $$=ast;
} | VarDecl{
  auto ast = new DeclAST();
  ast->exp=unique_ptr<BaseAST>($1);
    cerr << "error: 555" << endl;
  $$=ast;
}
;

ConstDecl:
  CONST BType ConstDef MoreConstDefs ';' {
    auto ast = new ConstDeclAST();
    ($4)->insert(($4)->begin(), unique_ptr<BaseAST>($3));
    ast->const_defs = std::move(*($4));
    $$ = ast;
  };

MoreConstDefs:
  MoreConstDefs ',' ConstDef {
    ($1)->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };


BType
  : INT {
    auto Btype = new BTypeAST();
    //Btype->op = *unique_ptr<string>(new string("int"));
    Btype->op = "int";
    Btype->number=1;
    cerr << "error: 181" << endl;
    cout<<Btype->op<<"*(*(*(*(*(*)))))"<<endl;
    cout<<Btype<<endl;
    $$ = Btype;
  }| VOID {
    auto Btype = new BTypeAST();
    Btype->op = *unique_ptr<string>(new string("void"));
    Btype->number=0;
    cerr << "error: 171" << endl;
    $$ = Btype;
  }
  ;

Block:
 '{' MoreBlockItems '}' {
    auto ast = new BlockAST();
    ast->block_items = std::move(*($2));
    if (ast->block_items.empty()) {
      ast->op="x";
    }
    $$ = ast;
  };


MoreBlockItems:
  MoreBlockItems BlockItem {
    ($1)->push_back(unique_ptr<BaseAST>($2));
    $$ = $1;
  } | 
  %empty {
    $$ = new vector<unique_ptr<BaseAST>>();
  };


BlockItem:
  Decl {
     auto ast=new BlockItemAST();
     ast->exp=unique_ptr<BaseAST>($1);
    $$=ast;
  } | 
  Stmt {
      auto ast=new BlockItemAST();
     ast->exp=unique_ptr<BaseAST>($1);
    $$=ast;
  };

ConstDef
: IDENT '=' ConstInitVal{
auto ast = new ConstDefAST();
ast->ident = *unique_ptr<string>($1);
ast->exp = unique_ptr<BaseAST>($3);
Map[BlockDepth][ast->ident] = ValType(ValType::ValTypeEnum::const_type, ast->exp->number);
$$ = ast;
}
;


ConstInitVal
:ConstExp{
    $$ = $1;
}
;

LVal
:IDENT{
  auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
      auto it = Map[BlockDepth].find(ast->ident);
    if (it != Map[BlockDepth].end()) {
        ast->number = it->second.val;
    } else {
        // 如果没有找到，可以设置一个默认值或者处理缺失的情况
        ast->number = 0; // 这里设置一个默认值
    }
}
;


LLVal
:IDENT{
  auto ast = new LLValAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
      auto it = Map[BlockDepth].find(ast->ident);
    if (it != Map[BlockDepth].end()) {
        ast->number = it->second.val;
    } else {
        // 如果没有找到，可以设置一个默认值或者处理缺失的情况
        ast->number = 0; // 这里设置一个默认值
    }
}
;

PrimaryExp
: PrimaryExp1 {
    auto ast=new PrimaryExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
    $$=ast;

} | LVal{
  auto ast=new PrimaryExpAST();
  ast->exp=unique_ptr<BaseAST>($1);

  ast->number=ast->exp->number;
    $$=ast;

} | Number{
    auto ast=new PrimaryExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    ast->number=ast->exp->number;
      $$=ast;
} 
;

PrimaryExp1
:'(' Exp ')'{
      auto ast=new PrimaryExp1AST();
      ast->exp=unique_ptr<BaseAST>($2);
      ast->number=ast->exp->number;
      $$=ast;
}
;



ConstExp
: Exp{
 auto ast=new ConstExpAST();
  ast->exp=unique_ptr<BaseAST>($1);
  ast->number=ast->exp->number;
  $$=ast;
}
;

VarDecl:
  BType VarDef MoreVarDefs ';' {
    auto ast = new VarDeclAST();
    ($3)->insert(($3)->begin(), unique_ptr<BaseAST>($2));
    ast->var_defs = std::move(*($3));
      cerr << "error: 333" << endl;
    $$ = ast;
  };
  
MoreVarDefs:
  MoreVarDefs ',' VarDef {
    ($1)->push_back(unique_ptr<BaseAST>($3));
    $$ = $1;
  } | 
  %empty {
      cerr << "error: 222" << endl;
    $$ = new vector<unique_ptr<BaseAST>>();
  };

VarDef 
: IDENT{
   auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->exp = nullptr;
    $$ = ast;

} | IDENT '=' InitVal{
  auto ast = new VarDefAST();
  ast->ident = *unique_ptr<string>($1);
  ast->exp = unique_ptr<BaseAST>($3);
  $$ = ast;
}
;

InitVal
:Exp{
   $$ = $1;
}
;

%%



// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
   extern int yylineno;    // defined and maintained in lex
    extern char *yytext;    // defined and maintained in lex
    int len=strlen(yytext);
    int i;
    char buf[512]={0};
    for (i=0;i<len;++i)
    {
        sprintf(buf,"%s%d ",buf,yytext[i]);
    }
    fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);

  cerr << "error: " << s << endl;
}
