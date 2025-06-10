package totolotek.centrala.InfrastrukturaCentrali;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import totolotek.centrala.BudżetPanśtwa;
import totolotek.gracz.Gracz;
import totolotek.gracz.GraczLosowy;
import totolotek.gracz.GraczStałoBlankietowy;
import totolotek.udział_w_losowaniu.Blankiet;
import totolotek.udział_w_losowaniu.Zakład;

import java.util.NoSuchElementException;
import java.util.Set;

import static org.junit.jupiter.api.Assertions.*;

class KolekturaTest {

    private CentralaTotolotka centrala;
    private Kolektura kolektura;
    private BudżetPanśtwa budżet;

    @BeforeEach
    void setUp() {
        budżet = new BudżetPanśtwa();
        centrala = new CentralaTotolotka(budżet, 0);
        kolektura = centrala.stwórzKolekture();
    }

    @Test
    void testSprzedajKuponNaBlankiet_Prawidlowy() {
        // given
        Zakład[] zakłady = {new Zakład(Set.of(1,2,3,4,5,6))};
        Blankiet blankiet = new Blankiet(1, zakłady);
        Gracz gracz = new GraczStałoBlankietowy("Jan", "Kowalski", "12345678901", new Kolektura[]{kolektura}, blankiet, 10_00); // 10 zł


        kolektura.sprzedajKuponNaBlankiet(blankiet, gracz);


        assertEquals(7_00, gracz.ileŚrodków()); // Initial 10 zł - 3 zł = should remain 7 zł (the method returns 0, but środki are updated)
    }

    @Test
    void testSprzedajKuponNaBlankiet_BrakSrodkow() {
        // given
        Zakład[] zakłady = {new Zakład(Set.of(1, 2, 3, 4, 5, 6))};
        Blankiet blankiet = new Blankiet(1, zakłady);
        Gracz gracz = new GraczStałoBlankietowy("Anna", "Nowak", "98765432109", new Kolektura[]{kolektura}, blankiet, 2_00); // 2 zł, less than cost

        // when
        kolektura.sprzedajKuponNaBlankiet(blankiet, gracz);

        // then
        // środki should not go negative
        assertEquals(2_00, gracz.ileŚrodków()); // Should remain unchanged
    }

    @Test
    void testSprzedajKuponNaBlankiet_NieprawidlowyBlankiet() {
        // given
        Zakład[] zakłady = {null}; // Invalid zakład
        Blankiet blankiet = new Blankiet(1, zakłady);
        Gracz gracz = new GraczStałoBlankietowy("Piotr", "Zieliński", "55555555555", new Kolektura[]{kolektura}, blankiet);

        // then
        assertThrows(NoSuchElementException.class, () -> kolektura.sprzedajKuponNaBlankiet(blankiet, gracz));
    }

    @Test
    void testSprzedajKuponNaBlankiet_WieleLosowan() {
        // given
        Zakład[] zakłady = {new Zakład(Set.of(1, 2, 3, 4, 5, 6))};
        Blankiet blankiet = new Blankiet(3, zakłady); // 3 draws
        Gracz gracz = new GraczStałoBlankietowy("Kasia", "Wiśniewska", "44444444444", new Kolektura[]{kolektura}, blankiet, 10_00);

        // when
        kolektura.sprzedajKuponNaBlankiet(blankiet, gracz);

        // then
        assertEquals(1_00, gracz.ileŚrodków()); // Cost: 3 draws * 3 zł = 9 zł -> remaining: 1 zł
    }

    @Test
    void testSprzedajKuponNaBlankiet_WieleZakladow() {
        // given
        Zakład[] zakłady = {
                new Zakład(Set.of(1, 2, 3, 4, 5, 6)),
                new Zakład(Set.of(7, 8, 9, 10, 11, 12))
        };
        Blankiet blankiet = new Blankiet(1, zakłady);
        Gracz gracz = new GraczStałoBlankietowy("Marek", "Lewandowski", "66666666666", new Kolektura[]{kolektura}, blankiet, 10_00);

        // when
        kolektura.sprzedajKuponNaBlankiet(blankiet, gracz);

        // then
        assertEquals(4_00, gracz.ileŚrodków()); // Cost: 2 zakłady * 3 zł = 6 zł -> remaining: 4 zł
    }

    @Test
    void testSprzedajKuponNaBlankiet_DokladnaKwota() {
        // given
        Zakład[] zakłady = {new Zakład(Set.of(1, 2, 3, 4, 5, 6))};
        Blankiet blankiet = new Blankiet(1, zakłady);
        Gracz gracz = new GraczStałoBlankietowy("Ewa", "Dąbrowska", "33333333333", new Kolektura[]{kolektura}, blankiet, 3_00); // exact funds

        // when
        kolektura.sprzedajKuponNaBlankiet(blankiet, gracz);

        // then
        assertEquals(0, gracz.ileŚrodków()); // All money spent
    }

    @Test
    void testSprzedajLosowyKupon_ZaDużoZakładów() {
        Gracz gracz = new GraczLosowy("Ola", "Kaczmarek", "77777777777", new Kolektura[]{kolektura});

        assertThrows(IllegalArgumentException.class, () -> kolektura.sprzedajLosowyKupon(1, 9, gracz)); // max 8 zakładów allowed
    }

    @Test
    void testSprzedajLosowyKupon_ZaMaloLosowan() {
        Gracz gracz = new GraczLosowy("Tomek", "Wójcik", "11111111111", new Kolektura[]{kolektura});

        assertThrows(IllegalArgumentException.class, () -> kolektura.sprzedajLosowyKupon(0, 1, gracz)); // min 1 losowanie required
    }
}
