#pragma once
#include "json_c.c"



typedef struct AST {
	int num;
	char function_name[20];
	char return_type[10];
	char** param_name;
	char** param_type;
	int if_num;
} AST;


class jsonfile {
    FILE* fp;
	char* buf;
	int size;

public:
    char* read(const char* filename, int* filesize);
};


class createAST {
    char* jsonData;
    int cnt, ASTnumber;
    json_value json, obj, body, decl;

public:
    createAST(char* buf);
    json_value getJson();
    int func_cnt(json_value json);
};


class ASTparse {
    int func_cnt, ASTnumber, if_cnt, ptr;
    char *decl_name, *nodetype, *returntype, *paramName, *paramtype;
    const char *cmp_PtrDecl = "PtrDecl";
    const char *cmp_IdentifierType = "IdentifierType";
    const char *cmp_If = "If";
    const char *cmp_Compound = "Compound";
    AST *ast;
    json_value json, body, decl, type, returnType, obj, paramType;

public:
    ASTparse(int cnt);

    AST* getAST();

    void parse_function_name(json_value decl, AST* ast, int ASTnumber);

    void parse_return_type(json_value type, AST* ast, int ASTnumber);

    void parse_param_name(json_value params, AST* ast, int ASTnumber);

    void parse_param_type(json_value params, AST* ast, int ASTnumber);

    void count_if(json_value block_items, AST* ast, int ASTnumber);
};


void find_if(json_value obj, int* if_cnt);
void saveAST(AST* ast, int func_num);