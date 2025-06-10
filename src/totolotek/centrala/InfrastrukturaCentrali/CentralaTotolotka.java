package totolotek.centrala.InfrastrukturaCentrali;

import totolotek.centrala.BudżetPanśtwa;
import totolotek.centrala.losowanie.GeneratorLosów;
import totolotek.centrala.losowanie.Losowanie;
import totolotek.centrala.nagrody.Nagrody;
import totolotek.udział_w_losowaniu.Kupon;
import totolotek.udział_w_losowaniu.Zakład;
import totolotek.udział_w_losowaniu.identyfikator_kuponu.GeneratorIndentyfikatorów;
import totolotek.udział_w_losowaniu.identyfikator_kuponu.IdentyfikatorKuponu;

import java.util.*;

public class CentralaTotolotka {

    private final BudżetPanśtwa budżetPanśtwa;

    private final GeneratorIndentyfikatorów generatorIndentyfikatorów = new GeneratorIndentyfikatorów();

    private final GeneratorLosów generatorLosów = new GeneratorLosów();

    private int numerKolektury = 0;

    private long poprzedniaKumulacja = 0;

    private long budżetCentrali;

    private long[] wpłatyNaLoterie = new long[10];

    private Map<Integer, Nagrody> nagrody;

    private List<Kolektura> kolektury;

    private final long NAJMNIEJSZA_WYGRANA_I_STOPNIA = 2_000_000_00;

    private final long NAJMNIEJSZA_WYGRANA_III_STOPNIA = 36_00;

    private final long WYGRANA_IV_STOPNIA = 24_00;

    private final long PODATEK_OD_ZAKŁADU = 60;

    private final long BARIERA_PODATKOWA_WYGRANEJ = 2280_00;

    private final long PODATEK_OD_WYGRANEJ = 10;

    private Nagrody ostatnieNagrody;

    public CentralaTotolotka(BudżetPanśtwa budżetPanśtwa, long środki) {
        if(budżetPanśtwa == null)
            throw new IllegalArgumentException("Budżet państwa nie może być null");

        this.budżetPanśtwa = budżetPanśtwa;
        this.budżetCentrali = środki;
        this.nagrody = new HashMap<>();
        this.kolektury = new ArrayList<>();
    }

    public Kolektura stwórzKolekture() {
        Kolektura kolektura = new Kolektura(this, ++numerKolektury);
        this.kolektury.add(kolektura);
        return kolektura;
    }

    public int ileKolektur() {
        return numerKolektury;
    }

    public IdentyfikatorKuponu nowyIdentyfikator(int idKolektury) {
        return this.generatorIndentyfikatorów.generuj(idKolektury);
    }

    public void noweLosowanie() {
        Losowanie losowanie = generatorLosów.generujLosowanie();
        int obecneLosowanie = losowanie.dajNumberLosowania();

        int[] LiczIleNaNagrode = new int[4];
        for(int i = 0; i < this.ileKolektur(); i++) {
            this.kolektury.get(i).ileWygranych(LiczIleNaNagrode, losowanie);
        }

        long środki = 51 *  wpłatyNaLoterie[obecneLosowanie % 10] / 100;

        long nagrodaIstopnia = Math.max( ((środki * 44) / 100) + poprzedniaKumulacja,
                NAJMNIEJSZA_WYGRANA_I_STOPNIA);
        long nagrodaIIstopnia = Math.max(((środki * 8) / 100), 0);
        long nagrodaIVstopnia = WYGRANA_IV_STOPNIA * (long) LiczIleNaNagrode[3];
        long nagrodaIIIstopnia = Math.max(((środki * 48) / 100) - nagrodaIVstopnia,
                NAJMNIEJSZA_WYGRANA_III_STOPNIA * (long) LiczIleNaNagrode[2]);

        Nagrody nagrody = new Nagrody(losowanie, nagrodaIstopnia, nagrodaIIstopnia, nagrodaIIIstopnia, nagrodaIVstopnia, LiczIleNaNagrode);

        this.ostatnieNagrody = nagrody;
        int id = losowanie.dajNumberLosowania();
        this.nagrody.put(id, nagrody);

        if(LiczIleNaNagrode[0] == 0) poprzedniaKumulacja = nagrodaIstopnia;
        else poprzedniaKumulacja = 0;
        wpłatyNaLoterie[(obecneLosowanie % 10)] = 0;
    }

    private long podatekOdWygranej(long cur) {
        if(cur <= BARIERA_PODATKOWA_WYGRANEJ) return 0;

        long dif = cur - BARIERA_PODATKOWA_WYGRANEJ;
        long kwotapodatku = dif / PODATEK_OD_WYGRANEJ;

        this.budżetPanśtwa.wpłaćPodatek(kwotapodatku);
        return kwotapodatku;
    }

    private void zdobądźŚrodki(long cur) {
        if(cur <= this.budżetCentrali) return;

        this.budżetCentrali = cur - this.budżetCentrali;
        this.budżetPanśtwa.wniosekSubwencji((cur - this.budżetCentrali) * 2);
    }

    protected long odbierzNagrode(Kupon kupon) {
        if(kupon == null)
            throw new IllegalArgumentException("Kupon nie może być null");

        int początkoweLosowanie = kupon.getPierwszyNumerLosowania();
        List<Zakład> zakłady = kupon.dajObstawioneZakłady();
        long wygrana = 0;

        for(int i = początkoweLosowanie; i < początkoweLosowanie + kupon.getLiczbaLosowań(); i++) {
            if(!this.nagrody.containsKey(i)) break;
            Nagrody nagrody = this.nagrody.get(i);

            for(Zakład x : zakłady) {
                long cur = nagrody.wyznaczNagrode(x);

                zdobądźŚrodki(cur);
                this.budżetCentrali -= cur;

                cur -= podatekOdWygranej(cur);
                wygrana += cur;
            }
        }
        return wygrana;
    }

    protected long kuponSprzedany(long cena, int ile, int losowanie) {
        long podatek = PODATEK_OD_ZAKŁADU * ile;
        budżetPanśtwa.wpłaćPodatek(podatek);

        wpłatyNaLoterie[losowanie % 10] += cena * ile - podatek;
        this.budżetCentrali += cena * ile - podatek;

        return podatek;
    }

    public int dajNumerNastępnegoLosowania() {
        return this.generatorLosów.następnyNumerLosowania();
    }

    private String formatujKwotę(long grosze) {
        long zł = grosze / 100;
        long gr = grosze % 100;
        return String.format("%dzł %02dgr", zł, gr);
    }

    public Nagrody dajOstatniąNagrody() {
        return ostatnieNagrody;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Centrala Totolotka:\n")
                .append("  Budżet Centrali: ").append(formatujKwotę(budżetCentrali)).append("\n")
                .append("  Wpłaty na Loterie:\n");

        for (int i = 1; i < dajNumerNastępnegoLosowania(); i++) {
            sb.append(nagrody.get(i).toString().indent(2));
        }

        sb.append("  Kolektury: ").append(kolektury.size()).append("\n");
        return sb.toString();
    }


}
