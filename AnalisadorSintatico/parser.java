import java.util.ArrayList;
import java.util.List;

// =============================================
// Mapeamento do analisador_lexico.py:
//
// LEXEMA     -> TIPO (token.tipo)
// -------       ----
// si         -> IF
// sinon      -> ELSE
// sinon_si   -> ELIF
// alors_que  -> WHILE
// pour       -> FOR
// ET         -> AND
// OU         -> OR
// NON        -> NOT
// FIN        -> EOF
// afficher   -> PRINT
// saisir     -> INPUT
// ouvrir     -> OPEN
// fermer     -> CLOSE
// entier     -> INT
// flotter    -> FLOAT
// doubler    -> DOUBLE
// chaine     -> STRING
// personnage -> CHAR
// logique    -> BOOL
//
// (          -> ouvrirPAREN
// )          -> fermerPAREN
// {          -> MAIOR  (operador >)
// }          -> MENOR  (operador <)
// {__        -> IGUAL_MAIOR (operador >=)
// }__        -> IGUAL_MENOR (operador <=)
// __         -> IGUALDADE   (operador ==)
// =          -> IGUAL       (atribuicao)
// PLUSPLUS   -> PLUSPLUS
// MOINSMOINS -> MOINSMOINS
// +          -> ADD
// -          -> SUB
// *          -> MULT
// /          -> DIV
// %          -> PCM
// [a-z]+     -> ID
// [0-9]+     -> NUMBER
// =============================================

class Parser {

    private List<Token> tokens;
    private Token token;
    private Tree tree;

    public Parser(List<Token> tokens) {
        this.tokens = new ArrayList<>(tokens);
        this.tree = new Tree();
    }

    public Tree parse() {
        token = getNextToken();
        Node root = new Node("main");
        tree.setRoot(root);

        boolean ok = programa(root);
        if (ok && token != null && token.tipo.equals("EOF")) {
            matchT("EOF", root);
            System.out.println("Sintaticamente correto");
        } else {
            System.out.println("Parse error");
            erro();
        }
        return tree;
    }

    private Token getNextToken() {
        if (tokens.size() > 0) {
            return tokens.remove(0);
        }
        return null;
    }

    public void erro() {
        if (token != null) {
            System.out.println("Syntaxe incorrecte: token inattendu " + token.lexema + " (tipo: " + token.tipo + ")");
        } else {
            System.out.println("Syntaxe incorrecte: fim inesperado");
        }
    }

    private boolean programa(Node node) {
        Node programa = node.addNode("programa");
        while (token != null && !token.tipo.equals("EOF")) {
            if (!comando(programa)) return false;
        }
        return true;
    }

    private boolean comando(Node node) {
        if (token == null) return false;
        Node cmd = node.addNode("comando");
        switch (token.tipo) {
            case "IF":    return IF(cmd);
            case "WHILE": return WHILE(cmd);
            case "FOR":   return FOR(cmd);
            case "PRINT": return print(cmd);
            case "INPUT": return input(cmd);
            case "OPEN":  return bloco(cmd);
            case "ID":    return atribuicao(cmd);
            case "INT":
            case "FLOAT":
            case "DOUBLE":
            case "STRING":
            case "CHAR":
            case "BOOL": return declaracao(cmd);
            default:      return false;
        }
    }

    private boolean ifelse(Node node) {
        Node n = node.addNode("ifelse");
        if (matchT("ELIF", n) && matchT("ouvrirPAREN", n) && condicao(n) && matchT("fermerPAREN", n) && bloco(n)) {
            return true;
        }
        return false;
    }

    private boolean declaracao(Node node) {
        Node n = node.addNode("declaracao");

        if(!tipoVariavel(n)) return false;

        if(!id(n)) return false;

        if(token != null && token.tipo.equals("IGUAL")) {
            if(!operadorAtribuicao(n)) return false;
            if(!idOuNumero(n)) return false;
        }

        return true;
    }

    private boolean id(Node node) {
        Node n = node.addNode("id");
        if (matchT("ID", n)) return true;
        return false;
    }

    private boolean numero(Node node) {
        Node n = node.addNode("numero");
        if (matchT("NUMBER", n)) return true;
        return false;
    }

    private boolean IF(Node node) {
        Node n = node.addNode("IF");
        if (matchT("IF", n) && matchT("ouvrirPAREN", n) && condicoes(n) && matchT("fermerPAREN", n) && bloco(n)) {
            
            while(token != null && token.tipo.equals("ELIF")) {
                if(!ifelse(n)) return false;
            }

            if (token != null && token.tipo.equals("ELSE")) {
                if(!ELSE(n)) return false;
            }
            
            return true;
        }
        return false;
    }

    private boolean condicao(Node node) {
        Node n = node.addNode("condicao");
        if (idOuNumero(n) && operador(n) && idOuNumero(n)) {
            return true;
        }
        return false;
    }

    private boolean idOuNumero(Node node) {
        if (token == null) return false;
        if (token.tipo.equals("ID")) return id(node);
        if (token.tipo.equals("NUMBER")) return numero(node);
        return false;
    }

    private boolean condicoes(Node node) {
        Node n = node.addNode("condicoes");
        if (token == null) return false;

        if (token.tipo.equals("NOT")) {
            if (!matchT("NOT", n)) return false;
            if (!condicoes(n)) return false;
        } else if (token.tipo.equals("ouvrirPAREN")) {
            if (!matchT("ouvrirPAREN", n)) return false;
            if (!condicoes(n)) return false;
            if (!matchT("fermerPAREN", n)) return false;
        } else {
            if (!condicao(n)) return false;
        }

        while (token != null && (token.tipo.equals("AND") || token.tipo.equals("OR"))) {
            if (!agregador_logico(n)) return false;
            if (!condicoes(n)) return false;
        }
        return true;
    }

    private boolean operador(Node node) {
        Node n = node.addNode("operador");
        if (matchT("IGUAL_MAIOR", n) || matchT("IGUAL_MENOR", n) || matchT("IGUALDADE", n)
                || matchT("MAIOR", n) || matchT("MENOR", n)) {
            return true;
        }
        return false;
    }

    private boolean agregador_logico(Node node) {
        Node n = node.addNode("agregador_logico");
        if (matchT("AND", n) || matchT("OR", n) || matchT("NOT", n)) {
            return true;
        }
        return false;
    }

    private boolean ELSE(Node node) {
        Node n = node.addNode("ELSE");
        if (matchT("ELSE", n) && bloco(n)) {
            return true;
        }
        return false;
    }

    private boolean WHILE(Node node) {
        Node n = node.addNode("WHILE");
        if (matchT("WHILE", n) && matchT("ouvrirPAREN", n) && condicoes(n) && matchT("fermerPAREN", n) && bloco(n)) {
            return true;
        }
        return false;
    }

    private boolean incrimento(Node node) {
        Node n = node.addNode("incrimento");
        if (token == null) return false;
        if (token.tipo.equals("ID")) {
            if (!id(n)) return false;
            if (matchT("PLUSPLUS", n) || matchT("MOINSMOINS", n)) return true;
            return false;
        }
        if (token.tipo.equals("PLUSPLUS") || token.tipo.equals("MOINSMOINS")) {
            if (!(matchT("PLUSPLUS", n) || matchT("MOINSMOINS", n))) return false;
            return id(n);
        }
        return false;
    }

    private boolean tipoVariavel(Node node) {
        Node n = node.addNode("tipoVariavel");
        if (matchT("INT", n) || matchT("FLOAT", n) || matchT("DOUBLE", n)
                || matchT("STRING", n) || matchT("CHAR", n) || matchT("BOOL", n)) {
            return true;
        }
        return false;
    }

    private boolean operadorMatematico(Node node) {
        Node n = node.addNode("operadorMatematico");
        if (matchT("ADD", n) || matchT("SUB", n) || matchT("MULT", n)
                || matchT("DIV", n) || matchT("PCM", n)) {
                    System.out.println("Entrei aqui o que rolou??");
            return true;
        }
        return false;
    }

    private boolean FOR(Node node) {
        Node n = node.addNode("FOR");
        if (matchT("FOR", n)
                && matchT("ouvrirPAREN", n)
                && tipoVariavel(n)
                && id(n)
                && operadorAtribuicao(n)
                && numero(n)
                && matchT("AND", n)
                && condicao(n)
                && matchT("AND", n)
                && incrimento(n)
                && matchT("fermerPAREN", n)
                && bloco(n)) {
            return true;
        }
        return false;
    }

    private boolean print(Node node) {
        Node n = node.addNode("print");
        if (matchT("PRINT", n) && matchT("ouvrirPAREN", n) && idOuNumero(n) && matchT("fermerPAREN", n)) {
            return true;
        }
        return false;
    }

    private boolean input(Node node) {
        Node n = node.addNode("input");
        if (matchT("INPUT", n) && matchT("ouvrirPAREN", n) && id(n) && matchT("fermerPAREN", n)) {
            return true;
        }
        return false;
    }

    private boolean atribuicao(Node node) {
        Node n = node.addNode("atribuicao");
        if (!id(n)) return false;
        if (token != null && token.tipo.equals("IGUAL")) {
            return funcao(n);
        }
        if (token != null && (token.tipo.equals("PLUSPLUS") || token.tipo.equals("MOINSMOINS"))) {
            return matchT(token.tipo, n);
        }
        return true; 
    }

    private boolean funcao(Node node){
        Node n = node.addNode("funcao");
        if (!id(n)) return false;
        if (token != null && token.tipo.equals("IGUAL")) {
            if((idOuNumero(n) && operadorMatematico(n) && idOuNumero(n)|| idOuNumero(n)) );
        }
        if (token != null && (token.tipo.equals("PLUSPLUS") || token.tipo.equals("MOINSMOINS"))) {
            return matchT(token.tipo, n);
        }
        return true; 
    }

    private boolean bloco(Node node) {
        Node n = node.addNode("bloco");
        if (!matchT("OPEN", n)) return false;
        while (token != null && !token.tipo.equals("CLOSE") && !token.tipo.equals("EOF")) {
            if (!comando(n)) return false;
        }
        if (matchT("CLOSE", n)) return true;
        return false;
    }

    private boolean operadorAtribuicao(Node node) {
        Node n = node.addNode("operadorAtribuicao");
        if (matchT("IGUAL", n)) return true;
        return false;
    }

    private boolean matchL(String palavra, Node node) {
        if (token != null && token.lexema.equals(palavra)) {
            node.addNode(token.lexema);
            token = getNextToken();
            return true;
        }
        return false;
    }

    private boolean matchT(String tipo, Node node) {
        if (token != null && token.tipo.equals(tipo)) {
            node.addNode(token.lexema);
            token = getNextToken();
            return true;
        }
        return false;
    }
}