package totolotek.gracz;

import totolotek.centrala.InfrastrukturaCentrali.Kolektura;

import java.util.concurrent.ThreadLocalRandom;

public class GraczLosowy extends Gracz {

    public GraczLosowy(String imie, String nazwisko, String pesel, Kolektura[] kolektury) {
        super(imie, nazwisko, pesel, kolektury, 1);
        this.dodajŚrodki(ThreadLocalRandom.current().nextLong(0, 100000000));
    }

    @Override
    protected void kupujKupon() {
        int index = ThreadLocalRandom.current().nextInt(0, this.ulubioneKolektury.length);
        Kolektura wylosowanaKolektura = this.ulubioneKolektury[index];

        int ileKuponów = ThreadLocalRandom.current().nextInt(0, 100);
        for(int i = 0; i < ileKuponów; i++) {
            int liczbaLosowań = ThreadLocalRandom.current().nextInt(1, 11);
            int liczbaZakładów = ThreadLocalRandom.current().nextInt(1, 9);
            wylosowanaKolektura.sprzedajLosowyKupon(liczbaLosowań, liczbaZakładów, this);
        }
    }

}
