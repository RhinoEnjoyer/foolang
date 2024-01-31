#include <sstream>
#include "./parser.hpp"
#include <stdint.h>
#include <map>
#include <iostream>

namespace parser{

  std::string token_info(token_t* t){
    if(t){
      return "\"" + std::string(token_text(t),token_len(t)) + "\"";  
    }else{
      return "Connector";
    }
  }

  void node_print(node_t* node, size_t depth){
    for(size_t i = 0; i < depth; i++){std::cerr << "   ";} 
    std::cerr<< RED << depth << RESET << YELLOW  << "|" RESET << "node: " YELLOW << node << RESET ", "; 
    if(node->token) std::cerr << "token: " << YELLOW << node->token << RESET ", ";  
    std::cerr <<  token_info(node->token) << " "  << ((node->token == nullptr)?MAGENTA: BLUE) << token_code_print(node->type) << RESET << std::endl;
  }

  //kinda trash but it's ok
  void node_traverse_rec(node_t* head, size_t depth, std::function<void(node_t*,size_t)> action){
    action(head,depth);
    for(auto& c: head->children){
      node_traverse_rec(&c,depth + 1, action); 
    }
  }
  void node_traverse(node_t* head,std::function<void(node_t *,size_t)> action){
    node_traverse_rec(head,0,action);
  }

  node_t create_node(token_t* token, int type){
    node_t node;
    node.token = token;
    node.type = type;
    return node;
  }
  node_t create_token_node(token_t* token){
    return create_node(token,token->type);
  }

  size_t node_token_count(node_t* node){
    size_t count = 0;
    node_traverse(node, [&count](node_t * head, size_t depth){ if(head->token) count += 1;} ); 
    return count;
  }

  inline node_t empty(){
    return {EMPTY};
  }

  //#define ONLY_FATAL_PRINT

  struct state{
    token_t* head;
    size_t token_count;
    size_t token_index;
    //std::vector<std::string> error_buffer;

    state(token_t* h, size_t tc): head{h},token_count{tc},token_index{0}{} 

    bool cons(){
      if (token_index > token_count) {
      //std::cout << "Out of tokens" << std::endl;
       return false;
      }
      token_index += 1;
      return true;
    }

    void reg(){
      if(token_index > 1)
        token_index--;
    }
    

    token_t* peek(int dst){
      if (token_index + dst > token_count) {
        return nullptr;
      }

      token_t* const t = head + (token_index + dst - 1);
      if(t->type == OTHER){
        this->frep("Found a Unknown token",__FUNCTION__);
        exit(EXIT_FAILURE);
      }
      return t;
    }

    size_t rec(){
      return token_index;
    }

    std::string msg(const std::string& message, const std::string& caller){
      token_t* t = peek((token_index > 0)?0 : 1);
      std::ostringstream oss;
      oss << t;
      return std::string(" [" YELLOW + caller + RESET "] " + "at token: " + "begin:" GREEN + std::to_string(t->loc.begin) +RESET " ptr:" + YELLOW + oss.str()  + RESET" " + token_info(t) + " " + message);   
    }

    void err_handler(std::string e){
      //error_buffer.push_back(e);
      std::cerr << e << std::endl;
    }
    
    void rep(const std::string& message, const std::string& caller) {
      #ifndef ONLY_FATAL_PRINT
      err_handler(RED "(error)" RESET + msg(message,caller));
      #endif
    }

    void srep(const std::string& message, const std::string& caller){
      #ifndef ONLY_FATAL_PRINT
      err_handler(GREEN "(success)" RESET + msg(message,caller));
      #endif
    }

    void frep(const std::string& message, const std::string& caller){
      err_handler(BG_RED"(KILL YOURSELF NOW)" RESET + msg(message,caller));
    }

    void re(size_t index){
      #ifndef ONLY_FATAL_PRINT
      err_handler( "["  YELLOW + std::string( __FUNCTION__) + RESET "] " GREEN "Recovered" RESET);
      #endif
      token_index = index;
    }
  };


  #define d_argsm state* s
  #define c_argsm s
  #define cons() s->cons()
  #define reg() s->reg()
  #define peek(i) s->peek(i)
  #define rec() s->rec()

  #define rep(message) s->rep(message,__FUNCTION__)
  #define srep(message) s->srep(message,__FUNCTION__)
  #define frep(message) s->frep(message,__FUNCTION__)

  #define re(i,...)  {s->re(i); rep(std::string(__VA_ARGS__));}

    
  #define err_msg(func) rep(std::string("Failed at line: " + std::to_string(__LINE__) + " Match Function: " + std::string(#func)));
  #define suc_msg(func) srep( std::string("Success at line: " + std::to_string(__LINE__) + " Match Function: " + std::string(#func)));
  
  #define orcase(node,r,func) node = func;if(node.type != ERROR){suc_msg(func);goto retpoint;}else{err_msg(func);}re(r);
  #define _case(node,func,err_action) node_t node = func;{size_t ___i = rec();if(node.type == ERROR){err_msg(func);re(___i);err_action;}else{suc_msg(func);}}
  #define andcase(node,func) node_t node = func;{size_t ___i = rec();if(node.type == ERROR){err_msg(func);re(___i);return {ERROR,{node}};}else{suc_msg(func);}}
  #define defcase(message,...) rep(message);return {ERROR,{__VA_ARGS__}};

  #define nodetp(node) //std::cout <<"[" << __FUNCTION__ <<"]"<< #node  << ":" << token_code_print(node.type) << //std::endl;
  node_t match(const size_t type, state* const s){
    token_t* const t = peek(1);
    if(t->type == type) { cons();return {.type = type,.token = t}; }
    else return {ERROR};
  }

  std::pair<std::vector<node_t>,size_t> match(std::vector<size_t> types, state* const s){
    std::vector<node_t> nodes;
    size_t type = 0;
    for(const auto& t: types){
      const node_t node = match(t,s);
      nodes.push_back(node);
      if(node.type == ERROR) {
        type = ERROR;
      }
    }

    return {nodes,type};
  }

  node_t wrap(size_t type, const std::vector<node_t>& nodes){
    for(const auto& n: nodes) 
      if(n.type == ERROR)
        {type = ERROR; break;}
    return {type, nodes};
  }
  node_t wrap(size_t type, node_t& node){
    if(node.type == ERROR)
      type = ERROR;
    return {type, {node}};
  }

  
  node_t list(state* s,int list_type,std::function<node_t(state*)> elmatch,int sep){
    std::vector<node_t> elms;

    while(true){
      size_t i = rec();
      node_t elm = elmatch(s);
      if(elm.type == ERROR){
        err_msg(elmsmatch);
        re(i);
        break;
      }else{
        suc_msg(elmsmatch); 
        elms.push_back(elm);
      }

      if(peek(1)->type != sep) 
        break;
      else 
        cons();
    }

    if(elms.size() == 1){return elms[0];}
    else if(elms.size() == 0){return {EMPTY_LIST};}
    else{return wrap(list_type,elms);}
  }

  node_t group(
    state* s,
    size_t prefix, 
    size_t suffix, 
    std::function<node_t(state*)> mid_match
  ){
    andcase(l,match(prefix,s));
    _case(mid,mid_match(s),{});
    andcase(r,match(suffix,s));

    if(mid.type == ERROR) return {EMPTY_GROUP};
    return mid;
  }

  

  node_t base_type(state* s){
    token_t* const t = peek(1);
    if(t->type >= BOOL && t->type <= F128){
      cons();
      return create_token_node(t); 
    }
    return {ERROR};
  }

  node_t typeof_builtin(state* s){
    andcase(tof, match(TYPEOF,s));
    andcase(l, match(LPAREN,s));
    andcase(mid,match(ID,s));
    andcase(r,match(RPAREN,s));

    return wrap(TYPEOF, mid);
  }

  node_t type(state* s){
    node_t t;
    size_t r = rec();
    orcase(t, r, base_type(s));
    orcase(t, r, typeof_builtin(s));
    orcase(t, r, match(ID,s));
    defcase("Unknown type of type",t);

    retpoint:
      return wrap(TYPE,t);
  }

  node_t id_(state* s){
    andcase(i,list(s,ID_LIST,[](state* s){return match(ID,s);},COMMA));
    return i;
  }

  node_t expr(state* s);  
  node_t var_decl(state* s){
    andcase(d, id_(s));
    andcase(c, match(COLON,s));
    andcase(t, type(s));

    size_t i = rec(); 
    if(match(ASIGN,s).type != ERROR){
      srep("There is a initialization on this declaration"); 
      node_t v;
      size_t i = rec();
      orcase(v,i,expr(s));
      retpoint:
        return wrap(VAR_DECL,{d,t,wrap(VALUE,v)});
    }else{rep("There is no initialization on this declaration");re(i);} 

    return wrap(VAR_DECL,{d,t});
  }



  node_t init_list(state* s);
  node_t if_builtin(state* s);
  node_t else_builtin(state* s);
  node_t elif_builtin(state* s);
  node_t func_call(state* s);
  node_t type(state* s);

  node_t as_builtin(state* s){

    andcase(a,match(AS,s));
    andcase(l,match(LPAREN,s));
    andcase(t,type(s));
    andcase(r,match(RPAREN,s));
    return wrap(AS,t);
  }
  
  node_t uop(state* s){
    node_t node;
    size_t i = rec();
      orcase(node,i,match(PLUS,s));
      orcase(node,i,match(MINUS,s));
      orcase(node,i,match(MULT,s));
      orcase(node,i,match(DIV,s));
      orcase(node,i,match(MOD,s));
      orcase(node,i,match(AND,s));
      orcase(node,i,match(OR,s));
      orcase(node,i,match(DOT,s));
        defcase("Unknown unary operator type",node);
    retpoint:
      return wrap(UN_OP,node);
  }

  node_t lop(state* s){
    node_t node;
    size_t i = rec();
      ////std::cout << token_info(peek(1)) << std::endl;
      orcase(node,i,match(NOT,s));
      orcase(node,i,match(POINTER,s));
      orcase(node,i,as_builtin(s));
        defcase("Unknown unary operator type",node);
    retpoint:
      return wrap(LEFT_OP,node);
  }

  node_t expr0(state* s){
    node_t node;
    size_t i = rec();
      orcase(node,i,func_call(s));
      orcase(node,i,match(ID,s));
      orcase(node,i,match(INT,s));
      orcase(node,i,match(FLOAT,s));
      orcase(node,i,init_list(s));
      defcase("Uknown expresion type",node);
    retpoint:
      return node;
  }

  #define expr_case(match_func,NEXT_STATE){\
      size_t __i = rec();\
      node = match_func;\
      if(node.type != ERROR){\
        suc_msg(match_func);\
        nodes_stack.push_back(node);\
        state = NEXT_STATE;\
        continue;\
      }else{err_msg(match_func);re(__i);}\
    }\

  #define expr_exit_case()\
  {if(node.type == ERROR){rep("Unknown state: " + std::to_string(state)); state = SEXIT; continue;}}

  node_t expr_paren_rec(const std::vector<node_t>& nodes,size_t& i){
    children_holder ch;

    for(; i < nodes.size(); i++){
      const auto n = nodes[i];
      ////std::cout << "index:" << i << " token:" << token_info(n.token) << std::endl;
      if(n.type == LPAREN){
        i++;
        ch.push_back(expr_paren_rec(nodes,i));
      }
      else if(n.type == RPAREN){
        break;
      }
      else{
        ch.push_back(n);
      }
    }

    return wrap(EXPR,ch);
  }

  node_t expr_paren(const std::vector<node_t>& nodes){
      size_t i = 0;
      node_t node = expr_paren_rec(nodes,i);
      return node;
  }

  node_t expr_postfix_eval(const std::vector<node_t> nodes){
    std::vector<node_t> stack;

    for(const auto& x: nodes){
      if(x.type == EXPR){
        stack.push_back(expr_postfix_eval(x.children));
      }else if(x.type == LEFT_OP){
        node_t tmp = x.children[0];

        node_t o = *(stack.end() - 1);

        tmp.children.push_back(o);
        stack.pop_back();
        stack.push_back(tmp);
      }else if(x.type == UN_OP){
        node_t tmp = x.children[0];

        node_t o1 = *(stack.end() - 1);
        node_t o2 = *(stack.end() - 2);

        tmp.children.push_back(o1);
        tmp.children.push_back(o2);

        stack.pop_back();
        stack.pop_back();

        stack.push_back(tmp);
      }else{
        stack.push_back(x);
      }
    }
    
    return wrap(EXPR,stack);
  }
  [[gnu::pure,gnu::hot]]
  std::vector<node_t> expr_shunting_yard(std::vector<node_t> nodes){
    std::vector<node_t> fixed;

    static const std::map<size_t,size_t> prec_map{
      {PLUS,5},{MINUS,5},
      {MULT,10},{DIV,10},{MOD,10},
      {AND,15},{OR,15},{XOR,15},
      {AS,98},{NOT,99},{POINTER,101},{DOT,101}
    };

    std::vector<node_t> op_stack;
    std::vector<node_t> out_queue;
   
    for(const auto& node: nodes){
      //std::cout << token_info(node.token) << std::endl;

      // Expresions
      if(node.type == FN_CALL || node.type == ID   ||
         node.type == INT     || node.type == FLOAT 
      ){out_queue.push_back(node);}
      else if(node.type == LPAREN){
         op_stack.push_back(node);
         out_queue.push_back(node);
      }
      else if(node.type == RPAREN){
        while(true){
          if(op_stack.size() == 0) break;
          node_t op_buffer = *(op_stack.end() - 1);
          if(op_buffer.type == LPAREN){
           op_stack.pop_back();
           break;
          }
          out_queue.push_back(op_buffer);
          op_stack.pop_back();
        }
        out_queue.push_back(node);
      }
      // Operators
      else if(node.type == UN_OP || node.type == LEFT_OP){
        if(op_stack.size() == 0){
          op_stack.push_back(node);
        }else if(op_stack.size() > 0){
          while(
            op_stack.size() != 0 &&
            (op_stack.end() -1)->type != LPAREN &&
            prec_map.at((op_stack.end() -1)->children[0].type) > prec_map.at(node.children[0].type))
          {
            node_t o = *(op_stack.end() - 1);
            op_stack.pop_back();
            out_queue.push_back(o);
          }
          op_stack.push_back(node);
        }
      }
    }

    for(size_t i = 0; i < op_stack.size(); i++)
      out_queue.push_back(op_stack[op_stack.size()- 1 - i]);

    auto transform = expr_paren(out_queue);
    transform = expr_postfix_eval(transform.children); 
    //Group the parenthesis
    //return nodes;
    //return out_queue;
    return {transform.children};
  }



  node_t expr_stackchk(state* s, std::vector<node_t> nodes){
      int64_t paren_balance;

      AGAIN:
      paren_balance = 0;
      for(size_t i = 0; i < nodes.size(); i++){
        const node_t& n = nodes[i]; 
        if(n.type == LPAREN) paren_balance++; 
        else if(n.type == RPAREN) paren_balance--;
      }
      // Recovery from rparen
      if(paren_balance < 0){
          nodes.pop_back();
          reg();
          goto AGAIN;
      }

      if(paren_balance != 0){
        std::cerr << RED "(KILL YOUR SELF RETARD)" RESET << "The parenthesis balance is: " << paren_balance;
        if(paren_balance > 0){
          std::cerr << " which means that you have " << paren_balance << " more left parenthesis than right ones" << std::endl; 
        }else if(paren_balance < 0){
          std::cerr << " which means that you have" << -1*paren_balance << " more right parenthesis than left ones" << std::endl; 
        }
        nodes.pop_back();
        exit(EXIT_FAILURE);
      }
      // maybe here make a thread
      // for all the checks that you need to do on the nodes
      // there is a small problem with left operators and dot operator
      // but other than that it is fine :)
      nodes = expr_shunting_yard(nodes);

      return  wrap(EXPR,nodes);
      //return expr_postfix_eval(nodes[0].children);
  }

  #define expr_false_case(match_func,err)\
      i = rec();\
      node = match_func;\
      if(node.type != ERROR){\
        frep(err);\
        exit(EXIT_FAILURE);\
      }else{re(i);}\


  node_t expr_entry(state* s){
    /*
     * fixed ??
     * 
     * this works great
     * but
     * there is a problem with detecting the end of a function
     * because a paren group ends with )
     * but so does a function call
     * which basicaly ends up with
     * the expr eating the ) and throwing a error becasue it can not find the matching (
     * BECAUSE IT BELONGS TO THE fucntion AND NOT THE expr
     * this would also happen with the ,
     * but it is not a symbol that can be used in a expr
     *
     * IT IS NOT A BUG IT IS A LANGUAGE FEATURE OK
     *
     */

    enum{SENTRY,SLPAREN,SRPAREN,SEXPR,SLOP,SUOP,SEXIT} state = SENTRY;
    std::vector<node_t> nodes_stack;
    bool bexit = false;
    while(!bexit){
      node_t node;
      node_t c;
      size_t i;
      switch(state){
        case SENTRY: 
            expr_case(match(LPAREN,s), SLPAREN);
            expr_case(expr0(s), SEXPR);
            expr_case(lop(s), SLOP);
            expr_exit_case();
          break;
        case SLPAREN:
            expr_case(match(LPAREN,s), SLPAREN);
            expr_case(expr0(s), SEXPR);
            expr_case(lop(s), SLOP);
            expr_exit_case();
          break;
        case SLOP:
            expr_case(match(LPAREN,s), SLPAREN);
            expr_case(lop(s), SLOP);
            expr_case(expr0(s), SEXPR);

            expr_false_case(uop(s), "a left operator can not precede a unary operator");
            //expr_false_case(lop(s), "a left operator can not precede a left operator");

            expr_exit_case();
        case SRPAREN: 
            expr_case(uop(s), SUOP);
            expr_case(match(RPAREN,s), SRPAREN);
            expr_exit_case();
          break;
        case SEXPR: 
            expr_case(uop(s), SUOP);
            expr_case(match(RPAREN,s), SRPAREN);
            expr_exit_case();
          break;
        case SUOP: 
            expr_case(match(LPAREN,s), SLPAREN);
            expr_case(expr0(s),SEXPR);
            expr_case(lop(s), SLOP);

            expr_false_case(uop(s), "a unary operator can not precede a unary operator");
            expr_false_case(match(RPAREN,s), "a unary operator can not precede a left parenthesis");

            expr_exit_case();
          break;
        case SEXIT: bexit = true; break;
      };
    }

    if(nodes_stack.size() == 0){
      rep("Expected to have a expresion");
      return {ERROR};
    }
    // One of the big problems here are that we basicaly we have no errors
    // so we end up looping forever
    // I love recursion
    // I do not know how to fix it
    // so make sure you type corectly
    // 
    // maybe define the the states that lead to an actual error like
      // UN_OP -> UN_OP


    //there is multy threading potential here
    //so we do not need to wait for the check to be done
    //we have shit error checking / none
    //so we can try it
    //I need to somehow get rid of the state
    //if not we can do it on the when the shunting yard runs

    const node_t* last_node = (nodes_stack.end() -1).base();
    if(last_node->type == UN_OP || 
       last_node->type == LPAREN || 
       last_node->type == LEFT_OP || 
       last_node->type == RIGHT_OP
    ){
      frep("the expresion cannot end with: " + token_code_print(last_node->type));
      exit(EXIT_FAILURE);
    }
    
    const auto& node = expr_stackchk(s,nodes_stack);

    return node;
  }

  node_t expr(state* s){
      const node_t node = expr_entry(s);
      return node;
  }

  /* Lvalues and Rvalues are validated later */
  node_t lvalue(state*s ){
    andcase(al , expr(s));
    return wrap(LVALUE,al);
  }
  node_t rvalue(state*s ){
    andcase(al , expr(s));
    return wrap(RVALUE,al);
  }

  node_t asign_stmt(state* s){
    andcase(al, lvalue(s));
    andcase(a , match(ASIGN,s));
    andcase(v , rvalue(s));

    return wrap(ASIGN_STMT,{al,wrap(VALUE,v)});
  }


  node_t stmt(state* s){
    node_t node;
    const size_t i = rec();
    //magic order do not touch
    orcase(node,i,var_decl(s));
    orcase(node,i,asign_stmt(s)); 
    orcase(node,i,expr(s));

    orcase(node,i,if_builtin(s));
    orcase(node,i,elif_builtin(s));
    orcase(node,i,else_builtin(s));

    retpoint:
      //andcase(semi,match(SEMICOLON,s));
      return wrap(STMT,node);
  }

  node_t init_list(state* s){
    // NOT REALLY WHAT I WAS GOING FOR
    andcase(node,group(s,LCBRACE,RCBRACE,[](state* s){return list(s,SEMI_LIST,expr,SEMICOLON);}));
    return wrap(INIT_LIST,node);
  }

  


  node_t comp_stmt(state* s){
    node_t g;
    size_t i = rec();

    orcase(g,i,group(s,LCBRACE,RCBRACE,[](state* s){return list(s,SEMI_LIST,stmt,SEMICOLON);}));
    orcase(g,i,group(s,LCBRACE,RCBRACE,[](state* s){return list(s,COMMA_LIST,stmt,COMMA);}));
      defcase("Uknown type of compound statement",g);

    retpoint:
      if(g.type != ERROR) 
        g = wrap(COMP_STMT,g);

      return g;
  }
  node_t elif_builtin(state* s){
    andcase(elif_, match(ELIF,s));
    andcase(l, match(LPAREN,s));
      andcase(cond, expr(s));
    andcase(r, match(RPAREN,s));
    andcase(body,comp_stmt(s));
    return wrap(ELIF_STMT,{wrap(IF_COND,cond),body});
  }
  node_t else_builtin(state* s){
    andcase(el, match(ELSE,s));
    andcase(body,comp_stmt(s));
    return wrap(ELSE_STMT,{body});
  }
  node_t if_builtin(state* s){
    andcase(i_, match(IF,s));
    andcase(l, match(LPAREN,s));
      andcase(cond, expr(s));
    andcase(r, match(RPAREN,s));
    andcase(body,comp_stmt(s));

    const node_t if_ = wrap(IF_STMT,{wrap(IF_COND,cond),body});
    
    std::vector<node_t> elifs;
    while(true){
      const size_t i = rec();
      node_t node = elif_builtin(s);
      //nodetp(node);
      if(node.type == ERROR){
        re(i);
        break;
      }else{
        elifs.push_back(node);
      }
    }
    node_t el = else_builtin(s);

    if(elifs.size() > 0 || el.type != ERROR){
      std::vector<node_t> chain;
      chain.push_back(if_);

      if(elifs.size() > 0){
        for(const auto& elif: elifs)
          chain.push_back(elif);
      }

      if(el.type != ERROR){
        chain.push_back(el);
      }

      return wrap(IF_CHAIN_STMT,chain);
    }
    
    return if_;
  }

  node_t func_args_rets(state* s){
    node_t args = wrap(ARGS,{group(s,LPAREN,RPAREN,[](state* s){return list(s,LIST,var_decl,COMMA);})});
    node_t rets = wrap(RETS,{group(s,LPAREN,RPAREN,[](state* s){return list(s,LIST,type,COMMA);})});
    return wrap(FN_ARGS_RETS,{args,rets});
  }
  node_t func_decl(state* s){
    andcase(i,match(ID,s));
    andcase(c,match(COLON,s));
    andcase(f,match(FN,s));
    andcase(ar,func_args_rets(s));

    if(match(ASIGN,s).type == ASIGN){
      node_t body = comp_stmt(s);
      if(body.type == COMP_STMT){
        if(body.children[0].type == COMMA_LIST)
          return {ERROR};

        return wrap(FN_DECL,{i,ar,body});
      }else return (node_t){ERROR};
    }

    return wrap(FN_PROTO,{i,ar});
  }

  node_t func_call(state* s){
    andcase(i, match(ID,s));
    andcase(l, match(LPAREN,s));
    andcase(cargs, list(s,LIST,expr,COMMA));
    andcase(r,match(RPAREN,s));

    return wrap(FN_CALL,{i,wrap(ARGS,cargs)});
  }

  node_t rec_decl(state* s){
    andcase(i,match(ID,s));
    andcase(c,match(COLON,s));
    andcase(r,match(REC,s));
    andcase(b,comp_stmt(s));

    return wrap(REC_DECL,{i,b});
  }
  
  node_t ext_decl(state* s){
    node_t node;
    size_t i = rec();

    orcase(node,i,func_decl(s));
    orcase(node,i,rec_decl(s));
    defcase("Unknown type of external declaration",node);

    retpoint:
      return wrap(EXT_DECL,node);
  }

  node_t unit(state* s){
    std::vector<node_t> decls;
    while(s->token_count > s->token_index){
      decls.push_back(ext_decl(s));
    }

    //for(const auto& e: s->error_buffer){
    //  std::cerr << e << std::endl;
    //}
   
    return wrap(UNIT,decls);
  }
  
  tree_t parse(std::vector<token_t>& tokens){ 
    tree_t tree;

    state s(tokens.data(),tokens.size());
    tree.head = unit(&s);

    return tree;
  }
  
}
