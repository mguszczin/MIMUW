package totolotek.gracz;

import totolotek.centrala.InfrastrukturaCentrali.Kolektura;
import totolotek.udział_w_losowaniu.Kupon;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public abstract class Gracz {
    private final String pesel, imie, nazwisko;

    private long środki = 0;

    protected final int coIleLosowanie;

    protected final Kolektura[] ulubioneKolektury;

    private final List<Kupon> posiadaneKupony = new ArrayList<>();

    public Gracz (String imie, String nazwisko, String pesel, Kolektura[] ulubioneKolektury, int coIleLosowanie) {
        if(imie == null || nazwisko == null || pesel == null || ulubioneKolektury == null || ulubioneKolektury.length == 0 || coIleLosowanie <= 0)
            throw new IllegalArgumentException("Niepoprawne dane gracza");
        this.imie = imie;
        this.nazwisko = nazwisko;
        this.pesel = pesel;
        this.ulubioneKolektury = ulubioneKolektury;
        this.coIleLosowanie = coIleLosowanie;
    }

    public Gracz(String imie, String nazwisko, String pesel, Kolektura[] ulubioneKolektury, int coIleLosowanie, long środki) {
        if(imie == null || nazwisko == null || pesel == null || ulubioneKolektury == null || ulubioneKolektury.length == 0 || coIleLosowanie <= 0)
            throw new IllegalArgumentException("Niepoprawne dane gracza");
        this.imie = imie;
        this.nazwisko = nazwisko;
        this.pesel = pesel;
        this.ulubioneKolektury = ulubioneKolektury;
        this.środki = środki;
        this.coIleLosowanie = coIleLosowanie;
    }

    protected abstract void kupujKupon();

    public void czyChceKupicKupon(int losowanie) {
        if((losowanie - 1 + coIleLosowanie) % coIleLosowanie == 0) kupujKupon();
    }

    public void ZrealizujKupony(int ostatnieLosowanie) {
        Iterator<Kupon> iterator = this.posiadaneKupony.iterator();
        while (iterator.hasNext()) {
            Kupon kupon = iterator.next();
            if(kupon.getPierwszyNumerLosowania() + kupon.getLiczbaLosowań() - 1 > ostatnieLosowanie) continue;
            iterator.remove();

            Kolektura kolektura = kupon.dajKolektura();
            dodajŚrodki(kolektura.odbierzKupon(kupon));
        }
    }

    public void dajKupon(Kupon kupon) {
        this.posiadaneKupony.add(kupon);
    }

    public long ileŚrodków() {
        return this.środki;
    }

    public int dodajŚrodki(long a) {
        if (this.środki + a < 0) return -1;
        this.środki += a;
        return 0;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(imie).append(" ").append(nazwisko).append("\n");
        sb.append("Pesel: ").append(pesel).append("\n");
        sb.append("Środki :").append(this.środki/100).append("zł").append(" ")
                .append(this.środki % 100).append("gr").append("\n");
        if(posiadaneKupony.size() > 0) {
            sb.append("Identyfikatory Kuponów:\n");
            for(Kupon kupon : posiadaneKupony) {
                sb.append(kupon.getIdentyfikator()).append("\n");
            }
        }else sb.append("Brak posiadanych kuponów.\n");
        return sb.toString();
    }
}
