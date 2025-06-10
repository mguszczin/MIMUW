package totolotek.gracz;

import totolotek.centrala.InfrastrukturaCentrali.Kolektura;
import totolotek.udział_w_losowaniu.Blankiet;

import java.util.concurrent.ThreadLocalRandom;

public class GraczStałoBlankietowy extends Gracz {
    private final Blankiet ulubionyBlankiet;
    private int obecnaKolektura = 0;

    public GraczStałoBlankietowy(String imie, String nazwisko, String pesel, Kolektura[] ulubioneKolektury, Blankiet ulubionyBlankiet) {
        super(imie, nazwisko, pesel, ulubioneKolektury, ulubionyBlankiet.dajLosowanie());
        this.dodajŚrodki(ThreadLocalRandom.current().nextLong(0, 1000000));

        if(coIleLosowanie <= 0) throw new IllegalArgumentException("Niepoprawna liczba Losowań");

        this.ulubionyBlankiet = ulubionyBlankiet;
    }

    public GraczStałoBlankietowy(String imie, String nazwisko, String pesel, Kolektura[] ulubioneKolektury, Blankiet ulubionyBlankiet, long środki) {
        super(imie, nazwisko, pesel, ulubioneKolektury, ulubionyBlankiet.dajLosowanie(), środki);

        if(coIleLosowanie <= 0) throw new IllegalArgumentException("Niepoprawna liczba Losowań");

        this.ulubionyBlankiet = ulubionyBlankiet;
    }


    @Override
    protected void kupujKupon() {
        Kolektura kolektura = this.ulubioneKolektury[obecnaKolektura++ % ulubioneKolektury.length];
        kolektura.sprzedajKuponNaBlankiet(this.ulubionyBlankiet, this);
    }
}
