package totolotek.centrala.losowanie;

import totolotek.udział_w_losowaniu.Zakład;

public class Losowanie {

    int numberLosowania;

    Zakład wygranyLos;

    public Losowanie(int i, Zakład wygrany) {
        this.numberLosowania = i;
        this.wygranyLos = wygrany;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Losowanie nr ").append(numberLosowania).append("\n");
        sb.append("Wyniki: ");
        sb.append(wygranyLos);
        return sb.toString();
    }


    public Zakład dajWygranyLos() {
        return this.wygranyLos;
    }

    public int dajNumberLosowania() { return this.numberLosowania; }
}
