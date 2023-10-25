#include "compiler.h"
#include <string>
#include <map>
#include <stdio.h>
#include <iostream>
#include "lexer.h"
using namespace std;

map <string, int> memory;


struct InstructionNode* parse_program();
void parse_var_section();
void parse_ID_list();
struct InstructionNode* parse_body();
struct InstructionNode* parse_stmt_list();
struct InstructionNode* parse_statement();
void location(string var);
struct InstructionNode* parse_generate_intermediate_representation();
void syntax_error();
struct InstructionNode* parse_assignment_statement();
struct InstructionNode* parse_while_statement();
struct InstructionNode* parse_if_statement();
struct InstructionNode* parse_switch_statement();
struct InstructionNode* parse_for_statement();
struct InstructionNode* parse_input_statement();
struct InstructionNode* parse_output_statement();
int parse_primary();
void parse_inputs();
ConditionalOperatorType parse_relop();
bool isNumber(const string& str);
struct InstructionNode* parse_case(int index, InstructionNode* n);
struct InstructionNode* parse_case_list(int index, InstructionNode *n);
struct InstructionNode* parse_default();

LexicalAnalyzer lexer;
Token expect(TokenType expected_type);
