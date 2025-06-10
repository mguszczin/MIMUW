package totolotek.udział_w_losowaniu.identyfikator_kuponu;

public class IdentyfikatorKuponu {

    private final int numerKuponu;
    private final int numerKolektury;
    private final String znacznik;
    private final String sumaKontrolna;

    public IdentyfikatorKuponu(int numerKuponu, int numerKolektury, String znacznik, String sumaKontrolna) {
        this.numerKuponu = numerKuponu;
        this.numerKolektury = numerKolektury;
        this.znacznik = znacznik;
        this.sumaKontrolna = sumaKontrolna;
    }

    @Override
    public String toString() {
        return numerKuponu + "-" + numerKolektury + "-" + znacznik + "-" + sumaKontrolna;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        IdentyfikatorKuponu that = (IdentyfikatorKuponu) o;
        return numerKuponu == that.numerKuponu &&
                numerKolektury == that.numerKolektury &&
                znacznik.equals(that.znacznik) &&
                sumaKontrolna.equals(that.sumaKontrolna);
    }

    @Override
    public int hashCode() {
        int result = Integer.hashCode(numerKuponu);
        result = 31 * result + Integer.hashCode(numerKolektury);
        result = 31 * result + znacznik.hashCode();
        result = 31 * result + sumaKontrolna.hashCode();
        return result;
    }


    public int getNumerKuponu() {
        return numerKuponu;
    }

    public int getNumerKolektury() {
        return numerKolektury;
    }
}
