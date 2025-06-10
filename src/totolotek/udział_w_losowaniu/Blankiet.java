package totolotek.udział_w_losowaniu;

public class Blankiet {

    private final Zakład[] obstawioneZakłady = new Zakład[8];
    private final boolean[] ileLosowań = new boolean[10];

    public Blankiet(int ileLosowań, Zakład ...zakłady) {
        if(ileLosowań <= 0 || ileLosowań >= 11 || zakłady.length > 8) {
            throw new IllegalArgumentException();
        }
        this.ileLosowań[ileLosowań - 1] = true;
        System.arraycopy(zakłady, 0, obstawioneZakłady, 0, zakłady.length);
    }

    public Blankiet() {
        this.ileLosowań[0] = true;
    }

    public void ustawZakład(int i, Zakład Zakład) {
        if(i < 0 || i >= 8) {
            throw new IllegalArgumentException();
        }
        obstawioneZakłady[i] = Zakład;
    }

    public void ustawLosowanie(int i) {
        if(i <= 0 || i >= 11) {
            throw new IllegalArgumentException();
        }
        ileLosowań[i - 1] = true;
    }

    // niektóre zakłady mogą być null
    public Zakład[] dajObstawioneZakłady() {
        return obstawioneZakłady;
    }

    public int dajLosowanie() {
        int c = 1;
        for(int i = 0; i < 10; i++) {
            if(ileLosowań[i]) c = i + 1;
        }
        return c;
    }

}
