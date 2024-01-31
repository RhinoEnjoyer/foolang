  #define ONLY_FATAL_PRINT

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

