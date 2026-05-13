import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;

class Tree {
    Node root;

    public Tree(){
    }

    public Tree(Node root){
        this.root = root;
    }

     public void setRoot(Node root){
        this.root = root;
     }

     public void preOrder(){
        preOrder(root);
        System.out.println("");
     }

     public void printCode(){
        printCode(root);
        System.out.println("");
     }

     public void preOrder(Node node){
        System.out.print(node);
        for (Node child : node.nodes){
            preOrder(child);
        }
     }

    public void printCode(Node node){
        System.out.print(node.enter);
        if (node.nodes.isEmpty()){
            System.out.print(node.nome);
        }
        for(Node child : node.nodes){
            printCode(child);
        }
        System.out.print(node.exit);
    }

    public void printTree(){
        if (root == null){
            System.out.println("(arvore vazia)");
            return;
        }
        System.out.println(root.getTree());
    }

    /**
     * Exports the AST to a file in the format "depth:nodeName",
     * one node per line, so the C semantic analyzer can read it.
     */
    public void exportToFile(String path) {
        try (PrintWriter pw = new PrintWriter(new FileWriter(path))) {
            exportNode(root, 0, pw);
        } catch (IOException e) {
            System.out.println("Erro ao exportar AST: " + e.getMessage());
        }
    }

    private void exportNode(Node node, int depth, PrintWriter pw) {
        if (node == null) return;
        pw.println(depth + ":" + node.nome);
        for (Node child : node.nodes) {
            exportNode(child, depth + 1, pw);
        }
    }

}
