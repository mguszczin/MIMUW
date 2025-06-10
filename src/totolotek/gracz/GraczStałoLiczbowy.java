package totolotek.gracz;

import totolotek.centrala.InfrastrukturaCentrali.Kolektura;
import totolotek.udział_w_losowaniu.Blankiet;
import totolotek.udział_w_losowaniu.Zakład;

public class GraczStałoLiczbowy extends GraczStałoBlankietowy {

    public GraczStałoLiczbowy(String imie, String nazwisko, String pesel, Kolektura[] ulubioneKolektury, Zakład ulubioneLiczby) {
        super(imie,nazwisko,pesel,ulubioneKolektury, new Blankiet(10, ulubioneLiczby));
    }

}
