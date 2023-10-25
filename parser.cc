#include "parser.h"

using namespace std;


void syntax_error()
{
    cout << "SYNTAX ERROR\n";
    exit(1);
}


Token expect(TokenType expected_type)
{
    Token t = lexer.GetToken();
    if (t.token_type != expected_type)
        syntax_error();
    return t;
}


void location(string var) {
    if (isNumber(var) == false) {
        mem[next_available] = 0;
        memory[var] = next_available;
        next_available++;
    }

    else {
        mem[next_available] = stoi(var);
        memory[var] = next_available;
        next_available++;
    }
    

}

struct InstructionNode* parse_body() {
    expect(LBRACE);
    InstructionNode* node;
    node = parse_stmt_list();
    expect(RBRACE);
    return node;
}

InstructionNode* append(InstructionNode* node)
{
    while (node->next != NULL) 
        node = node->next;
    return node;
}

struct InstructionNode* parse_stmt_list() {
    InstructionNode* stmnt = new InstructionNode();
    InstructionNode* stmnt_list;
    
    stmnt = parse_statement();

    Token t = lexer.peek(1);
    if (t.token_type == RBRACE) {
        stmnt_list = NULL; //I hate you so much, took me hours
    }
    else {
        stmnt_list = parse_stmt_list();
    }
    InstructionNode *temp = stmnt;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = stmnt_list;

    return stmnt;
    
}

struct InstructionNode* parse_statement() {
    InstructionNode* node = new InstructionNode();
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        //done
        node = parse_assignment_statement();
    }
    else if (t.token_type == WHILE) {
        node = parse_while_statement();
    }
    else if (t.token_type == IF) {
        node = parse_if_statement();
    }
    else if (t.token_type == SWITCH) {
        node = parse_switch_statement();
    }
    else if (t.token_type == FOR) {
        node = parse_for_statement();
    }
    else if (t.token_type == INPUT) {
        //done
        node = parse_input_statement();
    }
    else if (t.token_type == OUTPUT) {
        //done
        node = parse_output_statement();
    }
    else {
        syntax_error();
    }
    return node;
}
int parse_primary() {
    Token t = lexer.peek(1);
    if (t.token_type == NUM) {
        
        location(t.lexeme);
        
        expect(NUM);
        return memory[t.lexeme];
    }
    else if (t.token_type == ID) {
        expect(ID);
        return memory[t.lexeme];
    }
}
struct InstructionNode* parse_assignment_statement() {
    Token t = lexer.peek(1);
    expect(ID);
    InstructionNode* node = new InstructionNode();
 
    node->type = ASSIGN;
    node->assign_inst.left_hand_side_index = memory[t.lexeme];

    expect(EQUAL);
    node->assign_inst.operand1_index = parse_primary();

    t = lexer.peek(1);
    //we dont really need parse_expression, we just need to make sure there's an
    //operator or not
    if (t.token_type != PLUS && t.token_type != MINUS && t.token_type != MULT
        && t.token_type != DIV) {
        node->assign_inst.op = OPERATOR_NONE;
    }

    else {
        expect(t.token_type);
        if (t.token_type == PLUS) {
            node->assign_inst.op = OPERATOR_PLUS;
        }
        else if (t.token_type == MINUS) {
            node->assign_inst.op = OPERATOR_MINUS;
        }
        else if (t.token_type == MULT) {
            node->assign_inst.op = OPERATOR_MULT;
        }
        else if (t.token_type == DIV) {
            node->assign_inst.op = OPERATOR_DIV;
        }

        node->assign_inst.operand2_index = parse_primary();
    }
    expect(SEMICOLON);
    return node;

}
//input and output trivial
struct InstructionNode* parse_input_statement() {
    expect(INPUT);
    Token t;
    t = lexer.peek(1);
    expect(ID);
    InstructionNode* node = new InstructionNode();
    node->type = IN;
    node->input_inst.var_index = memory[t.lexeme];
    expect(SEMICOLON);
    return node;
}

struct InstructionNode* parse_output_statement() {
    expect(OUTPUT);
    Token t;
    t = lexer.peek(1);
    expect(ID);
    InstructionNode* node = new InstructionNode();
    node->type = OUT;
    node->output_inst.var_index = memory[t.lexeme];
    expect(SEMICOLON);
    return node;
}

struct InstructionNode* parse_if_statement() {
    expect(IF);
    InstructionNode* node = new InstructionNode();
    InstructionNode* outer = new InstructionNode();
    
    node->type = CJMP;
    outer->type = NOOP;
    outer->next = NULL;

   
    node->cjmp_inst.operand1_index = parse_primary();
    node->cjmp_inst.condition_op = parse_relop();
    node->cjmp_inst.operand2_index = parse_primary();
    node->next = parse_body();
    
    node->cjmp_inst.target = outer;
    append(node)->next = outer;

    return node;
    

}

struct InstructionNode* parse_while_statement() {
    expect(WHILE);
    InstructionNode* node = new InstructionNode();
    InstructionNode* jump = new InstructionNode();
    InstructionNode* outer = new InstructionNode();

    node->type = CJMP;
    outer->type = NOOP;
    outer->next = NULL;


    node->cjmp_inst.operand1_index = parse_primary();
    node->cjmp_inst.condition_op = parse_relop();
    node->cjmp_inst.operand2_index = parse_primary();
    node->next = parse_body();
    node->cjmp_inst.target = outer;

    jump->type = JMP;
    jump->jmp_inst.target = node;
    jump->next = outer;
    node->cjmp_inst.target = outer;
    InstructionNode* temp = node;

   
    append(temp)->next = jump;

    return node;


}

struct InstructionNode* parse_for_statement() {

    InstructionNode* node = new InstructionNode();
    InstructionNode* outer = new InstructionNode();
    InstructionNode* jump = new InstructionNode();

    node->type = CJMP;
    outer->type = NOOP;
    outer->next = NULL;

    expect(FOR);
    expect(LPAREN);
    //start with first assignment (i = 0 etc) 
    InstructionNode* assignIn = parse_assignment_statement();
    //set that assignment's NEXT command to our inner condition (i < 10) 
    assignIn->next = node;
    node->cjmp_inst.operand1_index = parse_primary();
    node->cjmp_inst.condition_op = parse_relop();
    node->cjmp_inst.operand2_index = parse_primary();
    expect(SEMICOLON);
    // now here's the outer statement, we will just consider this as inside the loop
    InstructionNode* outerAssign  = parse_assignment_statement();
    expect(RPAREN);
    
    node->next = parse_body();
 
    node->cjmp_inst.target = outer; //NOOP

    //JMP shit
    jump->type = JMP;
    jump->next = outer; // noop aha
    jump->jmp_inst.target = node; //loop around
    outerAssign->next = jump;
    InstructionNode* temp = node;
    // append the outer assignment to end of linked list
    append(temp->next)->next = outerAssign;
    assignIn->next = node;

    return assignIn;
}


struct InstructionNode* parse_switch_statement() {
    expect(SWITCH);
    Token t = lexer.peek(1);
    int var_index = memory[t.lexeme];
    expect(ID);
    expect(LBRACE);
    
    InstructionNode* noop = new InstructionNode();
    noop->next = NULL;
    noop->type = NOOP;

    Token t1 = lexer.peek(1);
    InstructionNode* node = parse_case_list(var_index, noop);
    

    t1 = lexer.peek(1);

    if (t1.token_type == DEFAULT) { //if default
        InstructionNode* def = new InstructionNode();
        def = parse_default();
        append(node)->next = def;
    }
    InstructionNode* temp = node;

    append(temp)->next = noop;
    expect(RBRACE);

    return node;

}

struct InstructionNode* parse_default() {
    expect(DEFAULT);
    expect(COLON);
    return parse_body();
}

struct InstructionNode *parse_case_list(int index, struct InstructionNode* n) {
    InstructionNode *c = parse_case(index, n);
    InstructionNode* case_list;

    Token t = lexer.peek(1);

    if (t.token_type == CASE) {
        case_list = parse_case_list(index, n);
    }

    else {
        case_list = NULL;
    }

    InstructionNode* temp = c;
    while (temp->next != NULL) {
        temp = temp->next;
    }

    temp->next = case_list;

    return c;


}

struct InstructionNode* parse_case(int index, struct InstructionNode* n) {
    expect(CASE);
   
    Token t = lexer.peek(1);
    location(t.lexeme);
    expect(NUM);
    expect(COLON);

    //allocating memory for our num

    InstructionNode* node = new InstructionNode();
    InstructionNode* jump = new InstructionNode();

    node->type = CJMP;
    
    node->cjmp_inst.operand1_index = index;
    node->cjmp_inst.condition_op = CONDITION_NOTEQUAL;
    node->cjmp_inst.operand2_index = memory[t.lexeme];

    node->cjmp_inst.target = parse_body(); //we want it to be wrong so we go to the body.
    


    jump->type = JMP;
    jump->jmp_inst.target = n;
    jump->next = node->next;

    InstructionNode* temp = node;
    append(temp->cjmp_inst.target)->next = jump;

    return node;
}

ConditionalOperatorType parse_relop() {
    Token t = lexer.peek(1);
    if (t.token_type == NOTEQUAL) {
        expect(NOTEQUAL);
        return CONDITION_NOTEQUAL;
    }
    else if (t.token_type == LESS) {
        expect(LESS);
        return CONDITION_LESS;
    }
    else if (t.token_type == GREATER) {
        expect(GREATER);
        return CONDITION_GREATER;
    }

}

struct InstructionNode* parse_program() {
    parse_var_section();
    InstructionNode* node;
    node = parse_body();
    parse_inputs();
    expect(END_OF_FILE);

    return node;

}

void parse_inputs() {
    Token t = lexer.peek(1);

    expect(NUM);
    inputs.push_back(stoi(t.lexeme));

    t = lexer.peek(1);
    if (t.token_type == NUM) {
        parse_inputs();
    }
    
}

void parse_var_section() {
    parse_ID_list();
    expect(SEMICOLON);
}

void parse_ID_list() {
    Token t = lexer.peek(1);
    if (t.token_type == ID) {
        location(t.lexeme);
    }
    expect(ID);
    Token t1 = lexer.peek(1);
    if (t1.token_type == COMMA) {
        expect(COMMA);
        parse_ID_list();
    }
}


struct InstructionNode* parse_generate_intermediate_representation() {
    return parse_program();
}



bool isNumber(const string& str)
{
    for (char const& c : str) {
        if (std::isdigit(c) == 0) return false;
    }
    return true;
}