package totolotek.udział_w_losowaniu;

import totolotek.centrala.InfrastrukturaCentrali.Kolektura;
import totolotek.udział_w_losowaniu.identyfikator_kuponu.IdentyfikatorKuponu;

import java.util.List;

public class Kupon {

    private final IdentyfikatorKuponu numerPorządkowy;

    private final List<Zakład> obstawioneZakłady;

    private final int liczbaLosowań, pierwszyNumerLosowania;

    private final long cena, podatek;

    private Kolektura kolekturaRealizacji;

    public Kupon(IdentyfikatorKuponu numberPorządkowy, List<Zakład> obstawioneZakłady,
                 int liczbaLosowań, int pierwszyNumerLosowania, long cena, long podatek, Kolektura kolekturaRealizacji) {
        this.numerPorządkowy = numberPorządkowy;
        this.obstawioneZakłady = obstawioneZakłady;
        this.liczbaLosowań = liczbaLosowań;
        this.pierwszyNumerLosowania = pierwszyNumerLosowania;
        this.cena = cena;
        this.podatek = podatek;
        this.kolekturaRealizacji = kolekturaRealizacji;
    }

    public IdentyfikatorKuponu getIdentyfikator() {
        return numerPorządkowy;
    }

    public long getCena() {
        return cena;
    }

    public Kolektura dajKolektura() {
        return kolekturaRealizacji;
    }

    public long getPodatek() {
        return podatek;
    }

    public int getLiczbaLosowań() {
        return this.liczbaLosowań;
    }

    public int getPierwszyNumerLosowania() {
        return this.pierwszyNumerLosowania;
    }

    public List<Zakład> dajObstawioneZakłady() {
        return this.obstawioneZakłady;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("KUPON NR ").append(numerPorządkowy).append("\n");

        int nrZakładu = 1;
        for (Zakład zakład : obstawioneZakłady) {
            sb.append(nrZakładu++).append(": ").append(zakład).append("\n");
        }

        sb.append("LICZBA LOSOWAŃ: ").append(liczbaLosowań).append("\n");

        sb.append("NUMERY LOSOWAŃ:\n");
        for (int i = 0; i < liczbaLosowań; i++) {
            sb.append(pierwszyNumerLosowania + i).append(" ");
        }
        sb.append("\n");

        long złote = cena / 100;
        long grosze = cena % 100;

        sb.append("CENA: ").append(złote).append(" zł ");
        if (grosze < 10) sb.append("0");
        sb.append(grosze).append(" gr");

        return sb.toString();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        Kupon kupon = (Kupon) o;
        return numerPorządkowy.equals(kupon.numerPorządkowy);
    }

    @Override
    public int hashCode() {
        return numerPorządkowy.hashCode();
    }

}
