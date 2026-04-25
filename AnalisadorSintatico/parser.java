import java.util.List;

public class Parser(List<Token> tokens) {
    this.tokens = tokens;
}


public void main(){
    token = getNextToken();
    if (ifelse()){
        if (token.tipo.equals("EOF")){
            System.out.println("Syntaxe incorrecte: EOF attendu après la structure if-else")
            return;
        } else {
            erro();
        }
    }
    erro();
}
public Token getNextToken(){
    if (tokens.size() > 0){
        return tokens.remove(0);
    } else {
        return null;
    }
}

public void Traduz(String code){
    System.out.println(code);
}


public void erro(){
    System.out.println("Syntaxe incorrecte: token inattendu " + token.lexema);
}

private boolean ifelse(){
    if (matchL("ELIF") && matchT("ouvrirPAREN") && condicao() && matchT("fermerPAREN") && bloco()){
        return true;
    }
}

private boolean id(){
    if (matchT("ID")){
        return true;
    }
}

private boolean numero(){
    if (matchT("INT") || matchT("FLOAT") || matchT("DOUBLE")){
        return true;
    }
}

if(condicao and (condicao))

private boolean IF(){
    if (matchL("IF") && matchT("ouvrirPAREN") && condicoes() && matchT("fermerPAREN") && bloco()){
        return true;
    }
}

private boolean condicao(){
    if((id() || numero()) && operador() && (id() || numero()) ){
        return true;
    }
    return false;
}

private boolean condicoes(){
    if ((matchT("ouvrirPAREN") && condicoes() && (agregador_logico() && condicoes() && matchT("fermerPAREN"))) || (condicoes() && (agregador_logico() && condicoes())) || condicao()){
        return true;
    }
    return false;
}

private boolean operador(){
    if (matchL("IGUAL_MAIOR") || matchL("IGUAL_MENOR") || matchL("IGUALDADE") || matchL("MAIOR") || matchL("MENOR")){
        return true;
    }
    return false;
}

private boolean agregador_logico(){
    if (matchL("ET") || matchL("OU") || matchL("NON")){
        return true;
    }
    return false;
}


private boolean ELSE(){
    if (matchL("ELSE") && bloco()){
        return true;
    }
}

private boolean WHILE(){
    if (matchL("WHILE") && matchT("ouvrirPAREN") && condicoes() && matchT("fermerPAREN") && bloco()){
        return true;
    }
}

private boolean incrimento(){
    if (matchL("ID") && (matchT("PLUSPLUS") || matchT("MOINSMOINS"))){
        return true;
    }
}

private boolean FOR(){
    if(matchL("FOR") && matchL("ouvrirPAREN") && matchT("INT") && matchL("ID") && operadorAtribuição() && numero() 
     && matchL("ET") 
     && condicoes() 
     && matchL("ET") 
     && matchL("ID") && ((incrimento() && (matchL("ID") || numero())) || incrimento())
     && matchL("fermerPAREN")
     && bloco()){
        return true;
    }
}

private boolean print(){
    if (matchL("AFFICHER") && matchL("ouvrirPAREN") && (id() || numero()) && matchL("fermerPAREN")){
        return true;
    }
}

private boolean bloco(){
    if(matchL("ouvrir") 
    && ((
    id() && operadorAtribuicao() && num())
    || (IF() && ifelse() && ELSE())
    || (IF())
    || (IF() && ELSE())
    || WHILE() 
    || FOR() 
    || print())
    && matchL("fermer")){
        return true;
    }

}
private boolean operadorAtribuição(){
    if (matchL("IGUAL")){
        return true;
    }
    return false;
}

private boolean input(){
    if (matchL("saisir") && matchL("ouvrirPAREN") && id() && matchL("fermerPAREN")){
        return true;
    }
}

private boolean matchL(String palavra){
    if (token != null && token.lexema.equals(palavra)){
        token = getNextToken();
        return true;
    } else {
        return false;
    }
}

private boolean matchT(String palavra){
    if (token != null && token.tipo.equals(palavra)){
        token = getNextToken();
        return true;
    } else {
        return false;
    }
}

