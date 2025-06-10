package totolotek.udział_w_losowaniu.identyfikator_kuponu;

import java.util.Random;

public class GeneratorIndentyfikatorów {

    private final Random random = new Random();
    private int aktualnyNumerKuponu = 1;

    public IdentyfikatorKuponu generuj(int numerKolektury) {
        int numerKuponu = aktualnyNumerKuponu++;
        String znacznik = generujZnacznik();
        String sumaKontrolna = obliczSumeKontrolna(numerKuponu, numerKolektury, znacznik);
        return new IdentyfikatorKuponu(numerKuponu, numerKolektury, znacznik, sumaKontrolna);
    }

    private String generujZnacznik() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 9; i++) {
            sb.append(random.nextInt(10));
        }
        return sb.toString();
    }

    private String obliczSumeKontrolna(int numerKuponu, int numerKolektury, String znacznik) {
        int suma = sumaCyfr(numerKuponu) + sumaCyfr(numerKolektury) + sumaCyfr(znacznik);
        int modulo = suma % 100;
        return String.format("%02d", modulo);
    }

    private int sumaCyfr(int liczba) {
        int suma = 0;
        while (liczba > 0) {
            suma += liczba % 10;
            liczba /= 10;
        }
        return suma;
    }

    private int sumaCyfr(String tekst) {
        int suma = 0;
        for (char c : tekst.toCharArray()) {
            if (Character.isDigit(c)) {
                suma += Character.getNumericValue(c);
            }
        }
        return suma;
    }

}
