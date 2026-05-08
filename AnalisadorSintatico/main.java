import java.util.ArrayList;
import java.util.List;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;

public class main {

    public static void main(String[] args) {
        List<Token> tokens = new ArrayList<>();
        
        try {
            File arquivo = new File("resultado_lexico.txt");
            Scanner leitor = new Scanner(arquivo);
            
            while (leitor.hasNextLine()) {
                String linha = leitor.nextLine().trim();
                
                if (linha.isEmpty()) {
                    continue;
                }
                
                linha = linha.replace("<", "").replace(">", ""); 
                
                String[] partes = linha.split(", ");
                
                if (partes.length == 2) {
                    String valor = partes[0];
                    String tipo = partes[1];
                    tokens.add(new Token(valor, tipo));
                }
            }
            leitor.close();
            
        } catch (FileNotFoundException e) {
            System.out.println("Erro: Arquivo 'resultado_lexico.txt' não encontrado.");
            System.out.println("Certifique-se de executar o analisador léxico em Python primeiro!");
            return;
        }

        System.out.println("Tokens lidos do arquivo:");
        for (Token t : tokens) {
            System.out.println("  " + t);
        }
        System.out.println();

        Parser parser = new Parser(tokens);
        Tree tree = parser.parse();

        System.out.println();
        tree.printTree();

        // Export the AST so the C semantic analyzer can consume it
        tree.exportToFile("resultado_sintatico.txt");
        System.out.println("AST exportada para 'resultado_sintatico.txt'");
    }
}