import totolotek.centrala.BudżetPanśtwa;
import totolotek.centrala.InfrastrukturaCentrali.CentralaTotolotka;
import totolotek.centrala.InfrastrukturaCentrali.Kolektura;
import totolotek.centrala.losowanie.GeneratorLosów;
import totolotek.gracz.GraczLosowy;
import totolotek.gracz.GraczMinimalista;
import totolotek.gracz.GraczStałoBlankietowy;
import totolotek.gracz.GraczStałoLiczbowy;
import totolotek.udział_w_losowaniu.Blankiet;
import totolotek.udział_w_losowaniu.Zakład;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

public class PokazKlas {

    public static String generujPesel() {
        Random random = new Random();
        // PESEL = 11 cyfr: YYMMDD + 5 losowych + suma kontrolna
        int rok = 1950 + random.nextInt(60); // 1950–2009
        int miesiac = 1 + random.nextInt(12);
        int dzien = 1 + random.nextInt(28);

        String yy = String.format("%02d", rok % 100);
        String mm = String.format("%02d", miesiac);
        String dd = String.format("%02d", dzien);

        String seria = String.format("%05d", random.nextInt(100000));
        String peselBezKontroli = yy + mm + dd + seria;


        return peselBezKontroli.substring(0, 11);
    }


    public static void main(String[] args) {
        BudżetPanśtwa budżet = new BudżetPanśtwa();
        CentralaTotolotka centrala = new CentralaTotolotka(budżet , 0);

        Kolektura[] kolektury = new Kolektura[10];

        for(int i = 0; i < 10; i++) {
            kolektury[i] = centrala.stwórzKolekture();
        }

        int liczbaGraczy = 200;

        String[] IMIONA = {
                "Jan", "Anna", "Piotr", "Katarzyna", "Tomasz", "Agnieszka", "Michał", "Ewa", "Paweł", "Magdalena"
        };
        String[] NAZWISKA = {
                "Kowalski", "Nowak", "Wiśniewski", "Wójcik", "Kowalczyk", "Kamiński", "Lewandowski", "Zieliński", "Szymański", "Woźniak"
        };

        GraczLosowy[] graczLosowy = new GraczLosowy[liczbaGraczy];
        for(int i = 0; i < liczbaGraczy; i++) {
            String imię = IMIONA[(int)(Math.random() * IMIONA.length)];
            String nazwisko = NAZWISKA[(int)(Math.random() * NAZWISKA.length)];
            String pesel = generujPesel();
            graczLosowy[i] = new GraczLosowy(imię, nazwisko, pesel, kolektury);
        }

        GraczMinimalista[] graczMinimalista = new GraczMinimalista[liczbaGraczy];
        for(int i = 0; i < liczbaGraczy; i++) {
            String imię = IMIONA[(int)(Math.random() * IMIONA.length)];
            String nazwisko = NAZWISKA[(int)(Math.random() * NAZWISKA.length)];
            String pesel = generujPesel();
            graczMinimalista[i] = new GraczMinimalista(imię, nazwisko, pesel, kolektury[(int)(Math.random() * kolektury.length)]);
        }

        GraczStałoBlankietowy[] graczStałoBlankietowy = new GraczStałoBlankietowy[liczbaGraczy];
        for(int i = 0; i < liczbaGraczy; i++) {
            String imię = IMIONA[(int)(Math.random() * IMIONA.length)];
            String nazwisko = NAZWISKA[(int)(Math.random() * NAZWISKA.length)];
            String pesel = generujPesel();

            // losowy blankiet
            int liczbaLosowań = ThreadLocalRandom.current().nextInt(1, 11);
            int liczbaZakładów = ThreadLocalRandom.current().nextInt(1, 9);
            List<Zakład> zakłady = new ArrayList<>();
            while (zakłady.size() < liczbaZakładów) {
                zakłady.add(GeneratorLosów.generujZakład());
            }

            Blankiet blankiet = new Blankiet(liczbaLosowań, zakłady.toArray(new Zakład[0]));

            graczStałoBlankietowy[i] = new GraczStałoBlankietowy(imię, nazwisko, pesel, kolektury, blankiet);
        }

        GraczStałoLiczbowy[] graczStałoLiczbowy = new GraczStałoLiczbowy[liczbaGraczy];
        for(int i = 0; i < liczbaGraczy; i++) {
            String imię = IMIONA[(int)(Math.random() * IMIONA.length)];
            String nazwisko = NAZWISKA[(int)(Math.random() * NAZWISKA.length)];
            String pesel = generujPesel();
            graczStałoLiczbowy[i] = new GraczStałoLiczbowy(imię, nazwisko, pesel, kolektury, GeneratorLosów.generujZakład());
        }

        // symulacja gry
        for(int j = 0; j < 20; j++) {
            int losowanie = centrala.dajNumerNastępnegoLosowania();

            for(int i = 0; i < liczbaGraczy; i++) {
                graczLosowy[i].czyChceKupicKupon(losowanie);
                graczStałoBlankietowy[i].czyChceKupicKupon(losowanie);
                graczMinimalista[i].czyChceKupicKupon(losowanie);
                graczStałoLiczbowy[i].czyChceKupicKupon(losowanie);
            }

            // generowanie losowania
            centrala.noweLosowanie();

            // spradzenie wyników , ewentualne wypłacenie nagród
            for(int i = 0; i < liczbaGraczy; i++) {
                graczLosowy[i].ZrealizujKupony(losowanie);
                graczStałoBlankietowy[i].ZrealizujKupony(losowanie);
                graczMinimalista[i].ZrealizujKupony(losowanie);
                graczStałoLiczbowy[i].ZrealizujKupony(losowanie);
            }
        }

        System.out.print(graczLosowy[0]);
        System.out.println(centrala);
        System.out.println(budżet);
    }
}