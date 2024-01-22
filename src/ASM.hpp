
#pragma once

#include <fstream>
#include <cassert>
#include "koopa.h"

extern std::ofstream my_riscv;

int allstack;//栈帧的总大小
std::unordered_map<koopa_raw_value_t, int> Map_;//映射到栈上的相对位置
std::string funcname = ""; // 当前所处的函数名字


void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &integer);
void Visit(const koopa_raw_store_t &st);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_branch_t &branch);
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void Visit(const koopa_raw_binary_t &bin, const koopa_raw_value_t &value);
void Visit(const koopa_raw_call_t &call, const koopa_raw_value_t &value);
void Visit(const koopa_raw_func_arg_ref_t &fargs);
void Visit(const koopa_raw_global_alloc_t &globals, const koopa_raw_value_t &value);

// 把sp改变
void change_pointer(int offset)
{
    my_riscv << "    addi sp, sp, " << offset << "\n";
}
// 分配一个单元大小的值在栈上
void alloc_single_or_size(koopa_raw_value_t value)
{
   allstack += 4;
    Map_[value] = -allstack;
}

// 分配指定大小的空间在栈上
int alloc_single_or_size(int size)
{
    allstack += size;
    return -allstack;
}
void load_value_to_register(koopa_raw_value_t value, std::string dest_reg)
{
    if (Map_.find(value) != Map_.end())
    {
        my_riscv << "    li " << dest_reg << ", " << Map_[value] << "\n";
        my_riscv << "    add " << dest_reg << ", " << dest_reg << ", fp\n";
        my_riscv << "    lw " << dest_reg << ", (" << dest_reg << ")\n";
        return;
    }
    else if (value->kind.tag == KOOPA_RVT_INTEGER)
    {
        my_riscv << "    li " << dest_reg << ", " << value->kind.data.integer.value << "\n";
    }
    else if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {

        my_riscv << "    la " << dest_reg << ", " << value->name + 1 << "\n"; // value->name[0] = '@'
    }
}
void alloc(koopa_raw_value_t value)
{
    // 确保 value 的类型是指针
    assert(value->ty->tag == KOOPA_RTT_POINTER);

    // 分配空间用于存储数据和指针
    int dataallstack = alloc_single_or_size(4);
    alloc_single_or_size(value);

    // 计算实际地址
    my_riscv << "    li t0, " << dataallstack << "\n";
    my_riscv << "    add t0, t0, fp\n";
    my_riscv << "    li t1, " << Map_[value] << "\n";
    my_riscv << "    add t1, t1, fp\n";

    // 将数据的地址存储到指针的地址中
    my_riscv << "    sw t0, (t1)\n";
}
void Visit(const koopa_raw_return_t &ret)
{
    if (ret.value != nullptr)
        load_value_to_register(ret.value, "a0");
    my_riscv << "    j return_" << funcname << "\n";
}
//运算
void Visit(const koopa_raw_binary_t &bin, const koopa_raw_value_t &value)
{
    std::string lreg = "t0";
    std::string rreg = "t1";
    std::string reg = "t0";
    load_value_to_register(bin.lhs, lreg);
    load_value_to_register(bin.rhs, rreg);
    switch (bin.op)
    {
    case KOOPA_RBO_NOT_EQ:
        my_riscv << "    xor " << reg << ", " << lreg << ", " << rreg << std::endl;
        my_riscv << "    snez " << reg << ", " << reg << std::endl;
        break;
    case KOOPA_RBO_EQ:
        my_riscv << "    xor " << reg << ", " << lreg << ", " << rreg << std::endl;
        my_riscv << "    seqz " << reg << ", " << reg << std::endl;
        break;
    case KOOPA_RBO_GT:
        my_riscv << "    sgt " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    case KOOPA_RBO_LT:
        my_riscv << "    slt " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    case KOOPA_RBO_GE:
        my_riscv << "    slt " << reg << ", " << lreg << ", " << rreg << std::endl;
        my_riscv << "    seqz " << reg << ", " << reg << std::endl;
        break;
    case KOOPA_RBO_LE:
        my_riscv << "    sgt " << reg << ", " << lreg << ", " << rreg << std::endl;
        my_riscv << "    seqz " << reg << ", " << reg << std::endl;
        break;
    case KOOPA_RBO_ADD:
        my_riscv << "    add " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    case KOOPA_RBO_SUB:
        my_riscv << "    sub " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    case KOOPA_RBO_MUL:
        my_riscv << "    mul " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    case KOOPA_RBO_DIV:
        my_riscv << "    div " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    case KOOPA_RBO_MOD:
        my_riscv << "    rem " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    case KOOPA_RBO_AND:
        my_riscv << "    and " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    case KOOPA_RBO_OR:
        my_riscv << "    or " << reg << ", " << lreg << ", " << rreg << std::endl;
        break;
    default:
        break;
    }
    alloc_single_or_size(value);
    my_riscv << "    li t1, " << Map_[value] << "\n";
    my_riscv << "    add t1, t1, fp\n";
    my_riscv << "    sw t0, (t1)\n";
}
void Visit(const koopa_raw_store_t &st)
{
    load_value_to_register(st.dest, "t1");
    load_value_to_register(st.value, "t0");
    my_riscv << "    sw t0, (t1)\n";
};
//跳转指令
void Visit(const koopa_raw_jump_t &jump)
{
    my_riscv << "    j " << jump.target->name + 1 << "\n";
}
//条件分支
void Visit(const koopa_raw_branch_t &branch)
{
    load_value_to_register(branch.cond, "t0");
    my_riscv << "    bnez t0, " << branch.true_bb->name + 1 << "\n";
    my_riscv << "    beqz t0, " << branch.false_bb->name + 1 << "\n";
}

void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value)
{
    load_value_to_register(load.src, "t0");
    my_riscv << "    lw t0, (t0)\n";
    alloc_single_or_size(value);
    my_riscv << "    li t1, " << Map_[value] << "\n";
    my_riscv << "    add t1, t1, fp\n";
    my_riscv << "    sw t0, (t1)\n";
}

void Visit(const koopa_raw_slice_t &slice)
{
    for (auto i = 0; i < slice.len; i++)
    {
        auto ptr = slice.buffer[i];
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION: // 函数
            Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK: // 块
            Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE: // 指令
            Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
            break;
        default:
            assert(false);
        }
    }
}
// 访问函数
void Visit(const koopa_raw_function_t &func)
{
    if (func->bbs.len == 0)
        return;

    my_riscv << "    .text\n";
    my_riscv << "    .globl " << func->name + 1 << "\n"; // 忽略第一个字符
    my_riscv << func->name + 1 << ":\n";

    allstack = 8;

    // 将返回地址 (ra) 和帧指针 (fp) 保存到栈中。
    my_riscv << "    sw ra, -4(sp)\n";
    my_riscv << "    sw fp, -8(sp)\n";
    my_riscv << "    mv fp, sp\n";
    change_pointer(-allstack);
    funcname = func->name + 1;

    Visit(func->bbs);

    // 访问块结束
    my_riscv << "return_" << func->name + 1 << ":\n";
    change_pointer(allstack); // 将sp改变一下，返回所有的状态
    my_riscv << "    mv sp, fp\n";
    my_riscv << "    lw fp, -8(sp)\n";
    my_riscv << "    lw ra, -4(sp)\n";
    my_riscv << "    ret\n";
}
void Visit(const koopa_raw_basic_block_t &bb)
{
    if (bb->name)
    {
        std::string bb_name = bb->name + 1;
        if (bb_name != "entry")
        {
            // 如果不是"entry"，则输出标签。(函数名字已经可以当label了捏，防止重复捏)
            my_riscv << bb_name << ":\n";
        }
    }
    Visit(bb->insts);
}
// 访问指令
void Visit(const koopa_raw_value_t &value)
{
    //    根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN: // 返回指令
        Visit(kind.data.ret);
        break;
    case KOOPA_RVT_BINARY: // 二进制
        Visit(kind.data.binary, value);
        break;
    case KOOPA_RVT_STORE: // 存储指令
        Visit(kind.data.store);
        break;
    case KOOPA_RVT_LOAD: // 加载指令
        Visit(kind.data.load, value);
        break;
    case KOOPA_RVT_ALLOC: // 分配内存指令
        alloc(value);
        break;
    case KOOPA_RVT_BRANCH: // 分支
        Visit(kind.data.branch);
        break;
    case KOOPA_RVT_JUMP: // 跳转
        Visit(kind.data.jump);
        break;
    default:
        assert(false);
    }
}
void Visit(const koopa_raw_program_t &program)
{
    Map_.clear();
    my_riscv << "   .data" << std::endl;
    // 访问所有全局变量
    Visit(program.values);
    // 访问所有函数
    Visit(program.funcs);
}
void parse_string(const char *str)
{
    koopa_program_t program;
    koopa_error_code_t ret = koopa_parse_from_string(str, &program);
    assert(ret == KOOPA_EC_SUCCESS);
    koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
    koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
    koopa_delete_program(program);

    Visit(raw);

    koopa_delete_raw_program_builder(builder);
}