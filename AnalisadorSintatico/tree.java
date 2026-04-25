public class Tree {
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
        if (node.nodes.IsEmpty()){
            System.out.print(node);
        }
        for(Node child : node.nodes){
            printCode(child);
        }        
        System.out.print(node.exit);
    }

    public void printTree(){
        System.out.println(root.getTree());
    }

}
