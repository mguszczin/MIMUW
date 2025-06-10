package totolotek.gracz;

import totolotek.centrala.InfrastrukturaCentrali.Kolektura;

import java.util.concurrent.ThreadLocalRandom;

public class GraczMinimalista extends Gracz {
    public GraczMinimalista(String imie, String nazwisko, String pesel, Kolektura ulubionaKolektura) {
        super(imie, nazwisko, pesel, new Kolektura[]{ulubionaKolektura}, 1);
        this.dodajŚrodki(ThreadLocalRandom.current().nextLong(0, 10000000));
    }

    @Override
    protected void kupujKupon() {
        Kolektura ulubionaKolektura = this.ulubioneKolektury[0];
        ulubionaKolektura.sprzedajLosowyKupon(1, 1, this);
    }
}
