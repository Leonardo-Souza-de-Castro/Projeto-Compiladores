import java.util.ArrayList;
import java.util.List;

public class main {

    public static void main(String[] args) {
        List<Token> tokens = new ArrayList<>();
        tokens.add(new Token("pour", "FOR"));
        tokens.add(new Token("(", "ouvrirPAREN"));
        tokens.add(new Token("entier", "INT"));
        tokens.add(new Token("i", "ID"));
        tokens.add(new Token("=", "IGUAL"));
        tokens.add(new Token("0", "NUMBER"));
        tokens.add(new Token("ET", "AND"));
        tokens.add(new Token("i", "ID"));
        tokens.add(new Token("}", "MENOR"));
        tokens.add(new Token("10", "NUMBER"));
        tokens.add(new Token("ET", "AND"));
        tokens.add(new Token("PLUSPLUS", "PLUSPLUS"));
        tokens.add(new Token("i", "ID"));
        tokens.add(new Token(")", "fermerPAREN"));
        tokens.add(new Token("ouvrir", "OPEN"));
        tokens.add(new Token("afficher", "PRINT"));
        tokens.add(new Token("(", "ouvrirPAREN"));
        tokens.add(new Token("i", "ID"));
        tokens.add(new Token(")", "fermerPAREN"));
        tokens.add(new Token("saisir", "INPUT"));
        tokens.add(new Token("(", "ouvrirPAREN"));
        tokens.add(new Token("i", "ID"));
        tokens.add(new Token(")", "fermerPAREN"));
        tokens.add(new Token("fermer", "CLOSE"));
        tokens.add(new Token("FIN", "EOF"));

        System.out.println("Tokens:");
        for (Token t : tokens) System.out.println("  " + t);
        System.out.println();

        Parser parser = new Parser(tokens);
        Tree tree = parser.parse();

        System.out.println();
        tree.printTree();
    }
}
