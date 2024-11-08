#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "ast_parser.h"
#include "json_c.c"



using namespace std;


char* jsonfile :: read(const char* filename, int* filesize) {
    fp = fopen(filename, "rb");
    if (fp == NULL) { printf("Fail to open file\n"); }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    printf("file size: %d\n", size);
    fseek(fp, 0, SEEK_SET);
    *filesize = size;

    buf = (char*)malloc(size + 1);
    memset(buf, 0, size + 1);
    fread(buf, size, 1, fp);

    fclose(fp);

    return buf;
}


createAST :: createAST(char* buf) : jsonData(buf) {
    json = json_create(jsonData);
}


json_value createAST :: getJson() { return json; }


int createAST :: func_cnt(json_value json) {
    cnt = 0;

    for(int i = 0; i < json_len(json); i++) {
	    obj = json_get(json, i);
	    body = json_get(obj, "body");
	    if(body.type == JSON_OBJECT) { cnt++; }
    }
    return cnt;
}


ASTparse :: ASTparse(int cnt) : func_cnt(cnt) {
    ast = (AST*)malloc(sizeof(AST) * func_cnt);
    memset(ast, 0, sizeof(ast) * func_cnt);
}


AST* ASTparse :: getAST() { return ast; }


void ASTparse :: parse_function_name(json_value decl, AST* ast, int ASTnumber) {
	decl_name = json_get_string(decl, "name");
	strcpy(ast[ASTnumber].function_name, decl_name);
}


void ASTparse :: parse_return_type(json_value type, AST* ast, int ASTnumber) {
    ptr = 0;
    
	while(1) {
		type = json_get(type, "type");
		nodetype = json_get_string(type, "_nodetype");

        if(strcmp(nodetype, cmp_PtrDecl) == 0) { ptr = 1; }

		if(strcmp(nodetype, cmp_IdentifierType) == 0) {
			returnType = json_get(type, "names");
			returntype = json_get_string(returnType, 0);

            if(ptr == 1) { strcat(returntype, "*"); }

			strcpy(ast[ASTnumber].return_type, returntype);
			break;
		}
	}
}


void ASTparse :: parse_param_name(json_value params, AST* ast, int ASTnumber) {
    ast[ASTnumber].param_name = (char**)malloc((json_len(params) + 1) * sizeof(char*));

	for(int j = 0; j < json_len(params); j++) {
		obj = json_get(params, j);
		paramName = json_get_string(obj, "name");

		ast[ASTnumber].param_name[j] = (char*)malloc(20 * sizeof(char));
		strcpy(ast[ASTnumber].param_name[j], paramName);
	}
	ast[ASTnumber].param_name[json_len(params)] = NULL;
}


void ASTparse :: parse_param_type(json_value params, AST* ast, int ASTnumber) {
    ast[ASTnumber].param_type = (char**)malloc((json_len(params) + 1) * sizeof(char*));

	for(int i = 0; i < json_len(params); i++) {
        ptr = 0;
		obj = json_get(params, i);
		type = json_get(obj, "type");
        nodetype = json_get_string(type, "_nodetype");
        if(strcmp(nodetype, cmp_PtrDecl) == 0) { ptr = 1; }

		while(1) {
			type = json_get(type, "type");
			nodetype = json_get_string(type, "_nodetype");

			if(strcmp(nodetype, cmp_IdentifierType) == 0) {
				paramType = json_get(type, "names");
				paramtype = json_get_string(paramType, 0);

                if(ptr == 1) { strcat(paramtype, "*"); }

				ast[ASTnumber].param_type[i] = (char*)malloc(10 * sizeof(char));
				strcpy(ast[ASTnumber].param_type[i], paramtype);
				break;
			}
		}
	}
	ast[ASTnumber].param_type[json_len(params)] = NULL;
}


void ASTparse :: count_if(json_value block_items, AST* ast, int ASTnumber) {
	if_cnt = 0;

	for(int i = 0; i < json_len(block_items); i++) {
        obj = json_get(block_items, i);
        find_if(obj, &if_cnt);

        stmt = json_get(obj, "stmt");
        if(stmt.type == JSON_OBJECT) {
            stmt_block = json_get(stmt, "block_items");
            for(int j = 0; j < json_len(stmt_block); j++) {
                obj = json_get(stmt_block, j);
                find_if(obj, &if_cnt);
            }
        }
    }
    ast[ASTnumber].if_num = if_cnt;
}


void find_if(json_value obj, int* if_cnt) {
    char *nodetype;
    json_value iftrue, true_block, iffalse, true_obj;
    const char* cmp_If = "If";


  	nodetype = json_get_string(obj, "_nodetype");

    if(strcmp(nodetype, cmp_If) == 0) { 
        (*if_cnt)++;

        iffalse = json_get(obj, "iffalse");
        if(iffalse.type == JSON_OBJECT) { find_if(iffalse, if_cnt); }
        
        iftrue = json_get(obj, "iftrue");
        true_block = json_get(iftrue, "block_items");
        for(int k = 0; k < json_len(true_block); k++) {
            true_obj = json_get(true_block, k);
            find_if(true_obj, if_cnt);
        }
    }
    else { return; }
}


void saveAST(AST* ast, int func_num) {
    FILE *fp;
    int j;

    fp = fopen("output.json", "w");


    fprintf(fp, "[\n");
    for(int i = 0; i < func_num; i++) {
        j = 0;

        fprintf(fp, "\t{\n");
        fprintf(fp, "\t\t\"number\": %d,\n", ast[i].num);
        fprintf(fp, "\t\t\"name\": \"%s\",\n", ast[i].function_name);
        fprintf(fp, "\t\t\"return_type\": \"%s\",\n", ast[i].return_type);

        if(ast[i].param_type[0] != NULL) {
            fprintf(fp, "\t\t\"parameters\": [\n");

	        while(ast[i].param_type[j] != NULL) {
                fprintf(fp, "\t\t\t{\n");
                fprintf(fp, "\t\t\t\t\"type\": \"%s\",\n", ast[i].param_type[j]);
                fprintf(fp, "\t\t\t\t\"name\": \"%s\"\n", ast[i].param_name[j]);

                if(ast[i].param_type[j + 1] == NULL) { fprintf(fp, "\t\t\t}\n"); }
                else { fprintf(fp, "\t\t\t},\n"); }
                j++;
            }

            fprintf(fp, "\t\t],\n");
        }
        else { fprintf(fp, "\t\t\"parameters\": [],\n"); }
        fprintf(fp, "\t\t\"number_of_if\": %d\n", ast[i].if_num);

        if(i + 1 == func_num) { fprintf(fp, "\t}\n"); }
        else { fprintf(fp, "\t},\n"); }
        
    }
    fprintf(fp, "]\n");

    fclose(fp);

    return;
}