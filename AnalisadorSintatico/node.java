import java.util.List;
import java.util.ArrayList;
import java.util.List;

class Node{

    String nome;
    List<Node> nodes;
    String enter;
    String exit;

    public Node(String nome){
        this.nome = nome;
        nodes = new ArrayList<>();
        this.enter = "";
        this.exit = "";
    }

    public void addNode(Node node){
        nodes.add(node);
    }

     public Node addNode(String nodeName){
        Node newNode = new Node(nodeName);
        nodes.add(newNode);
        return newNode;
     };

     public Node addNode(String enter, String nodeName, String exit){
        Node newNode = new Node(nodeName);
        newNode.enter = enter;
        newNode.exit = exit;
        nodes.add(newNode);
        return newNode;
     };

     @Override
     public String toString(){
        return this.enter + " " + this.nome + " " + this.exit;
     }
    
    public String getTree(){
        System.out.println("AST");
        StringBuilder buffer = new StringBuilder(50);
        getTreeHelper(buffer, "","");
        return buffer.toString();
    }

    private void getTreeHelper(StringBuilder buffer, String prefix, String childrenPrefix){
        buffer.append(prefix);
        buffer.append(this.nome);
        buffer.append("\n");
        for (int i = 0; i < nodes.size(); i++){
            Node node = nodes.get(i);
            if (i < nodes.size() - 1){
                node.getTreeHelper(buffer, childrenPrefix + "+-- ", childrenPrefix + "|   ");
            } else {
                node.getTreeHelper(buffer, childrenPrefix + "'-- ", childrenPrefix + "    ");
            }
        }
    }
}