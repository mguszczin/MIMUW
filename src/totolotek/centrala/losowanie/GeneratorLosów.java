package totolotek.centrala.losowanie;

import totolotek.udział_w_losowaniu.Zakład;

import java.util.HashSet;
import java.util.Random;
import java.util.Set;

public class GeneratorLosów {

    private static final Random random = new Random();
    private static int następnyNumberLosowania = 1;

    public static Zakład generujZakład() {
        Set<Integer> liczby = new HashSet<>();
        while (liczby.size() < 6) {
            int liczba = 1 + random.nextInt(49);
            liczby.add(liczba);
        }
        return new Zakład(liczby);
    }

    public int następnyNumerLosowania() {
        return następnyNumberLosowania;
    }

    public Losowanie generujLosowanie() {
        Zakład wygrany = generujZakład();
        Losowanie los = new Losowanie(następnyNumberLosowania++, wygrany);
        return los;
    }
}
