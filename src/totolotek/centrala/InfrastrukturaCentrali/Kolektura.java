package totolotek.centrala.InfrastrukturaCentrali;

import totolotek.centrala.losowanie.GeneratorLosów;
import totolotek.centrala.losowanie.Losowanie;
import totolotek.gracz.Gracz;
import totolotek.udział_w_losowaniu.Blankiet;
import totolotek.udział_w_losowaniu.Zakład;
import totolotek.udział_w_losowaniu.Kupon;
import totolotek.udział_w_losowaniu.identyfikator_kuponu.IdentyfikatorKuponu;

import java.util.*;

public class Kolektura {

    private final int numberKolektury;

    private final CentralaTotolotka centralaTotolotka;

    private final Map<Kupon, Boolean> zakupioneKupony = new HashMap<>();

    private final long CENA_ZAKŁADU = 3_00;

    protected Kolektura(CentralaTotolotka centralaTotolotka, int numerKolektury) {
        if(centralaTotolotka == null) {
            throw new IllegalArgumentException("CentralaTotolotka nie może być null");
        }
        this.centralaTotolotka = centralaTotolotka;
        this.numberKolektury = numerKolektury;
    }

    private void sprawdźZakłady(List<Zakład> zakłady, Zakład wygranyLos, int nagrody[]) {
        for(Zakład x : zakłady) {
            int val = 6 - x.wspólneLiczby(wygranyLos);
            if(val >= 0 && val < 4) nagrody[val]++;
        }
    }

    public void ileWygranych(int nagrody[], Losowanie los) {
        if(los == null) {
            throw new IllegalArgumentException("Losowanie nie może być null");
        }
        if(nagrody.length != 4) {
            throw new IllegalArgumentException("nagrody.length != 4");
        }
        Zakład wygranyLos = los.dajWygranyLos();
        int obecneLosowanie = los.dajNumberLosowania();

        for (Map.Entry<Kupon, Boolean> entry : zakupioneKupony.entrySet()) {
            Kupon kupon = entry.getKey();
            if(kupon.getPierwszyNumerLosowania() <= obecneLosowanie &&
            kupon.getPierwszyNumerLosowania() + kupon.getLiczbaLosowań() > obecneLosowanie) {
                List<Zakład> zakłady = kupon.dajObstawioneZakłady();
                sprawdźZakłady(zakłady,wygranyLos, nagrody);
            }
        }
    }

    public long odbierzKupon(Kupon kupon) {
        if(kupon == null) {
            throw new IllegalArgumentException("Kupon nie może być null");
        }
        if(!zakupioneKupony.containsKey(kupon) || !zakupioneKupony.get(kupon)) {
            return 0;
        }
        zakupioneKupony.put(kupon, false);
        long wygrana = this.centralaTotolotka.odbierzNagrode(kupon);
        return wygrana;
    }

    // zweryfikuj czy zakłady są ok
    public void sprzedajKuponNaBlankiet(Blankiet blankiet, Gracz gracz) {
        if(gracz == null)
            throw new IllegalArgumentException("Gracz nie może być null");
        if(blankiet == null)
            throw new IllegalArgumentException("Blankiet nie może być null");

        List<Zakład> dobreZakłady = new ArrayList<>(Arrays.asList(blankiet.dajObstawioneZakłady()));
        dobreZakłady.removeIf(z -> z == null || !z.czyPoprawny());

        if(dobreZakłady.isEmpty()) throw new NoSuchElementException("Podano niepoprawny blankiet");

        long cena = CENA_ZAKŁADU * dobreZakłady.size() * blankiet.dajLosowanie();
        if(gracz.dodajŚrodki(-cena) == -1) return;

        IdentyfikatorKuponu id = this.centralaTotolotka.nowyIdentyfikator(numberKolektury);
        long podatek = 0;
        for(int i = 0; i < blankiet.dajLosowanie(); i++) {
            podatek += centralaTotolotka.kuponSprzedany(CENA_ZAKŁADU, dobreZakłady.size(),
                    centralaTotolotka.dajNumerNastępnegoLosowania() + i);
        }
        Kupon nowyKupon = new Kupon(id, dobreZakłady, blankiet.dajLosowanie(), centralaTotolotka.dajNumerNastępnegoLosowania(),
                cena, podatek, this);
        zakupioneKupony.put(nowyKupon, true);
        gracz.dajKupon(nowyKupon);
    }

    public void sprzedajLosowyKupon(int liczbaLosowań, int liczbaZakładów, Gracz gracz) {
        if (liczbaZakładów > 8 || liczbaZakładów <= 0)
            throw new IllegalArgumentException("Zła liczba zakładów");
        if (liczbaLosowań < 1 || liczbaLosowań > 10)
            throw new IllegalArgumentException("Zła liczba losowań");

        List<Zakład> zakłady = new ArrayList<>();
        while (zakłady.size() < liczbaZakładów) {
            zakłady.add(GeneratorLosów.generujZakład());
        }

        Blankiet blankiet = new Blankiet(liczbaLosowań, zakłady.toArray(new Zakład[0]));
        sprzedajKuponNaBlankiet(blankiet, gracz);
    }



}
