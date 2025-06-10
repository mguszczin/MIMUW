package totolotek.centrala.InfrastrukturaCentrali;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import totolotek.centrala.BudżetPanśtwa;
import totolotek.centrala.losowanie.GeneratorLosów;
import totolotek.centrala.losowanie.Losowanie;
import totolotek.centrala.nagrody.Nagrody;
import totolotek.gracz.Gracz;
import totolotek.gracz.GraczStałoLiczbowy;
import totolotek.udział_w_losowaniu.Zakład;

import java.lang.reflect.Field;
import java.util.Set;

import static org.junit.jupiter.api.Assertions.*;

class CentralaTotolotkaTest {

    private BudżetPanśtwa budżet;
    private CentralaTotolotka centrala;
    private GeneratorLosów generator;
    private Kolektura kolektura1, kolektura2;

    // Prosty Stały GeneratorLosów
    static class StałyGeneratorLosów extends GeneratorLosów {
        private final Losowanie losowanie;

        StałyGeneratorLosów(Losowanie losowanie) {
            this.losowanie = losowanie;
        }

        @Override
        public Losowanie generujLosowanie() {
            return losowanie;
        }
    }

    @BeforeEach
    void setup() throws Exception {
        budżet = new BudżetPanśtwa();
        centrala = new CentralaTotolotka(budżet, 0);


        // Ustaw stałe losowanie 1-6
        Zakład zakład = new Zakład(Set.of(1, 2, 3, 4, 5, 6));
        Losowanie losowanie = new Losowanie(1, zakład);
        generator = new StałyGeneratorLosów(losowanie);

        // Podmień generator losów na stały
        Field field = CentralaTotolotka.class.getDeclaredField("generatorLosów");
        field.setAccessible(true);
        field.set(centrala, generator);

        // Ustaw wpłaty na loterię na 1 002 zł (100 200 groszy) dla losowania 1
        centrala.kuponSprzedany(300, 34, 1);
        kolektura1 = centrala.stwórzKolekture();
        kolektura2 = centrala.stwórzKolekture();
    }


    @Test
    void testNoweLosowanie_BrakWygranych_Kumulacja() throws Exception {
        // Kolektura bez wygranych
        Kolektura kolektura = centrala.stwórzKolekture();

        centrala.noweLosowanie();

        Nagrody nagroda = centrala.dajOstatniąNagrody();
        assertNotNull(nagroda);

        Field nagrodyField = Nagrody.class.getDeclaredField("nagrody");
        nagrodyField.setAccessible(true);
        long[] nagrody = (long[]) nagrodyField.get(nagroda);

        // Sprawdź minimalną nagrodę I stopnia 2 mln zł (200 000 000 groszy)
        assertEquals(200_000_000, nagrody[0]);
    }

    @Test
    void testPodatekOdWygranej_PoniżejProgu() {
        long podatek = budżet.getPodatki();
        assertEquals(2040, podatek);
    }

    @Test
    void testOdbierzNagrode_ZPodatkiem() {

        Gracz gracz = new GraczStałoLiczbowy("Marek", "Nowak", "02302323", new Kolektura[]{kolektura1, kolektura2},new Zakład(Set.of(1,2,3,4,5,6)));
        gracz.czyChceKupicKupon(1);
        long prev = gracz.ileŚrodków();

        centrala.noweLosowanie();
        gracz.ZrealizujKupony(10);

        // Wypłata po podatku:
        long expected = 1_800_228_00L;                     // 2 miliony - (2miliony - 2280) / 10

        assertEquals(expected, gracz.ileŚrodków() - prev);
    }
}