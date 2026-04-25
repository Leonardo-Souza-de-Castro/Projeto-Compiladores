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

public void erro(){
    System.out.println("Syntaxe incorrecte: token inattendu " + token.lexema);
}

private boolean ifelse(){
    if (matchL("IF") && condicao() && bloco() && matchL("ELSE") && bloco()){
        return true;
    }
}

private boolean if(){
    if (matchL("IF") && condicao() && bloco()){
        return true;
    }
}

private boolean bloco(){

}
