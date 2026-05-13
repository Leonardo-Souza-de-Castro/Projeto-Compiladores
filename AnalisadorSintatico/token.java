class Token{

public String lexema;
public String tipo;

public Token(String lexema, String tipo){
    this.lexema = lexema;
    this.tipo = tipo;
}

@Override
public String toString(){
    return "<" + lexema + ", " + tipo + ">";
}

}