#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast_parser.h"
#include "ast_parser.cpp"
#include "json_c.c"




int main() 
{
    const char* filename = "target.json";
    char* buf;
    int filesize, func_num, ASTnumber;
    jsonfile astfile;
    json_value json, ext, obj, body, decl, type, args, params, block_items;
    AST* ast;


    buf = astfile.read(filename, &filesize);

    createAST create(buf);
    json = create.getJson();

    ext = json_get(json, "ext");
    func_num = create.func_cnt(ext);
	printf("number of function: %d\n", func_num);

    ASTparse parse(func_num);
    ast = parse.getAST();
    
    ASTnumber = 0;

    for(int i = 0; i < json_len(ext); i++) {
        ast[ASTnumber].num = ASTnumber;

        obj = json_get(ext, i);
		body = json_get(obj, "body");
        decl = json_get(obj, "decl");

		if(body.type != JSON_OBJECT) { continue; }
        parse.parse_function_name(decl, ast, ASTnumber);

        type = json_get(decl, "type");
        parse.parse_return_type(type, ast, ASTnumber);

        args = json_get(type, "args");
        params = json_get(args, "params");
        parse.parse_param_name(params, ast, ASTnumber);
        parse.parse_param_type(params, ast, ASTnumber);

        block_items = json_get(body, "block_items");
        parse.count_if(block_items, ast, ASTnumber);

        ASTnumber++;
    }

    saveAST(ast, func_num);

    return 0;
}