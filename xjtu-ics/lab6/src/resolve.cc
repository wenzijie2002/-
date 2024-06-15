#include "resolve.h"

#include <iostream>

#define FOUND_ALL_DEF 0
#define MULTI_DEF 1
#define NO_DEF 2

std::string errSymName;

int callResolveSymbols(std::vector<ObjectFile> &allObjects);

void resolveSymbols(std::vector<ObjectFile> &allObjects)
{
    int ret = callResolveSymbols(allObjects);
    if (ret == MULTI_DEF)
    {
        std::cerr << "multiple definition for symbol " << errSymName << std::endl;
        abort();
    }
    else if (ret == NO_DEF)
    {
        std::cerr << "undefined reference for symbol " << errSymName << std::endl;
        abort();
    }
}

/* bind each undefined reference (reloc entry) to the exact valid symbol table entry
 * Throw correct errors when a reference is not bound to definition,
 * or there is more than one definition.
 */
std::unordered_map<std::string, Symbol *> symbolDefs;

int callResolveSymbols(std::vector<ObjectFile> &allObjects)
{
    std::string errSymName;
    int result = FOUND_ALL_DEF;

    // 收集所有全局符号的定义，并检测多重定义
    for (ObjectFile &obj : allObjects)
    {
        for (Symbol &sym : obj.symbolTable)
        {
            if (sym.bind == STB_GLOBAL && sym.index != SHN_UNDEF && sym.index != SHN_COMMON)
            {
                auto it = symbolDefs.find(sym.name);
                if (it != symbolDefs.end())
                {
                    // 如果符号已经存在，则存在多重定义
                    errSymName = sym.name;
                    result = MULTI_DEF;
                    goto end;
                }
                symbolDefs[sym.name] = &sym;
            }
            else if (sym.bind == STB_GLOBAL && sym.index == SHN_COMMON){
                auto it = symbolDefs.find(sym.name);
                if (it == symbolDefs.end())
                {
                    symbolDefs[sym.name] = &sym;
                }
            }
        }
    }
    // 检查所有重定位条目是否能够正确绑定
    for (ObjectFile &obj : allObjects)
    {
        for (RelocEntry &entry : obj.relocTable)
        {
            bool found = false;
            // 查找符号定义
            auto it = symbolDefs.find(entry.name);
            if (it != symbolDefs.end())
            {
                found = true;
                // 绑定重定位条目的符号指针到找到的符号定义
                entry.sym = it->second;
            }
            else
            {
                // 如果没有找到符号定义，报告错误
                errSymName = entry.name;
                result = NO_DEF;
                break;
            }
        }
        if (result == NO_DEF)
        {
            goto end;
        }
    }
end:
    if (result != FOUND_ALL_DEF)
    {
        // 输出错误信息并终止程序
        std::cerr << (result == MULTI_DEF ? "multiple definition for symbol " : "undefined reference for symbol ")
                  << errSymName << std::endl;
        std::abort(); // 退出机制
    }
    return result;
}
// 全局和弱符号定义的映射，键为符号名，值为符号定义的指针
/*std::unordered_map<std::string, Symbol *> symbolDefs;

int callResolveSymbols(std::vector<ObjectFile> &allObjects)
{
    std::string errSymName;
    int result = FOUND_ALL_DEF;

    // 收集所有全局符号和弱符号的定义
    for (ObjectFile &obj : allObjects)
    {
        for (Symbol &sym : obj.symbolTable)
        {
            if (sym.bind == STB_GLOBAL || sym.bind == STB_WEAK)
            {
                auto it = symbolDefs.find(sym.name);
                if (it == symbolDefs.end())
                {
                    // 如果符号尚未定义，则添加到映射中
                    symbolDefs[sym.name] = &sym;
                }
                else if (sym.bind == STB_GLOBAL && it->second->index != SHN_COMMON)
                {
                    // 如果已经存在一个弱定义，覆盖为强定义
                    symbolDefs[sym.name] = &sym;
                }
            }
        }
    }

    // 检查所有重定位条目是否能够正确绑定
    for (ObjectFile &obj : allObjects)
    {
        for (RelocEntry &entry : obj.relocTable)
        {
            bool found = false;
            // 查找符号定义
            auto it = symbolDefs.find(entry.name);
            if (it != symbolDefs.end())
            {
                found = true;
                // 绑定重定位条目的符号指针到找到的符号定义
                entry.sym = it->second;
            }
            else
            {
                // 如果没有找到符号定义，报告错误
                errSymName = entry.name;
                result = NO_DEF;
                break;
            }
        }
        if (result == NO_DEF)
        {
            goto end;
        }
    }

    // 检查是否有多个全局符号定义了相同的符号
    for (auto &pair : symbolDefs)
    {
        Symbol *sym = pair.second;
        if (sym->bind == STB_GLOBAL && sym->index != SHN_UNDEF && sym->index != SHN_COMMON)
        {
            auto otherIt = symbolDefs.find(sym->name);
            if (otherIt != symbolDefs.end() && otherIt->second != sym)
            {
                // 发现多个全局定义
                errSymName = sym->name;
                result = MULTI_DEF;
                goto end;
            }
        }
    }

end:
    if (result != FOUND_ALL_DEF)
    {
        // 输出错误信息并终止程序
        std::cerr << (result == MULTI_DEF ? "multiple definition for symbol " : "undefined reference for symbol ")
                  << errSymName << std::endl;
        std::abort(); // 或者其他适当的退出机制
    }
    return result;
}*/