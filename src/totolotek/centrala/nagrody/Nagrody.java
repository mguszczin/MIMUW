package totolotek.centrala.nagrody;

import totolotek.centrala.losowanie.Losowanie;
import totolotek.udział_w_losowaniu.Zakład;

public class Nagrody {
    private final Losowanie losowanie;
    private final long[] nagrody = new long[4];
    private final int[] ileWygranych = new int[4];

    public Nagrody(Losowanie losowanie, long nagrodaIstopnia, long nagrodaIIstopnia, long nagrodaIIIstopnia, long nagrodaIVstopnia, int[] ileWygranych) {
        if(losowanie == null)
            throw new IllegalArgumentException("Losowanie nie może być null");
        this.losowanie = losowanie;

        this.nagrody[0] = nagrodaIstopnia;
        this.ileWygranych[0] = ileWygranych[0];

        this.nagrody[1] = nagrodaIIstopnia;
        this.ileWygranych[1] = ileWygranych[1];

        this.nagrody[2] = nagrodaIIIstopnia;
        this.ileWygranych[2] = ileWygranych[2];

        this.nagrody[3] = nagrodaIVstopnia;
        this.ileWygranych[3] = ileWygranych[3];
    }

    public long wyznaczNagrode(Zakład obecnyZakład) {
        if(obecnyZakład == null)
            throw new IllegalArgumentException("Zakład nie może być null");

        Zakład wygranyLos = losowanie.dajWygranyLos();

        int val = 6 - obecnyZakład.wspólneLiczby(wygranyLos);
        if (val < 0 || val > 3) {
            return 0;
        }
        int wygrani = ileWygranych[val];
        if(wygrani == 0) throw new IllegalArgumentException("Jest zero wygranych to nie pownwinno się zdarzyć");
        return this.nagrody[val] / wygrani;
    }

    public long[] dajPuleNagród() {
        return this.nagrody;
    }

    public int[] dajWygranych() {
        return this.ileWygranych;
    }

    private String formatujKwotę(long grosze) {
        long zł = grosze / 100;
        long gr = grosze % 100;
        return String.format("%dzł %02dgr", zł, gr);
    }

    @Override
    public String toString() {
        var sb = new StringBuilder();
        String[] stopnie = {"I", "II", "III", "IV"};
        sb.append(losowanie).append("\n");

        for (int i = 0; i < stopnie.length; i++) {
            int wygranych = ileWygranych[i];
            sb.append(String.format("  Nagrody %s stopnia:\n", stopnie[i]))
                    .append(String.format("    Pula: %s\n", formatujKwotę(nagrody[i])))
                    .append(String.format("    Zwycięskie kupony: %d\n", wygranych));

            if (wygranych > 0) {
                sb.append(String.format("    Kwota wygrana: %s\n",
                        formatujKwotę(nagrody[i] / wygranych)));
            }
        }
        return sb.toString();
    }


}
