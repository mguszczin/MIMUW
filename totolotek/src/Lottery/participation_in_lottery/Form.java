package Lottery.participation_in_lottery;

public class Form {

    private final Bet[] placedBets = new Bet[8];
    private final boolean[] drawCount = new boolean[10];

    public Form(int drawCount, Bet...bets) {
        if(drawCount <= 0 || drawCount >= 11 || bets.length > 8) {
            throw new IllegalArgumentException();
        }
        this.drawCount[drawCount - 1] = true;
        System.arraycopy(bets, 0, placedBets, 0, bets.length);
    }

    public Form() {
        this.drawCount[0] = true;
    }

    public void setBet(int i, Bet Bet) {
        if(i < 0 || i >= 8) {
            throw new IllegalArgumentException();
        }
        placedBets[i] = Bet;
    }

    public void setDraw(int i) {
        if(i <= 0 || i >= 11) {
            throw new IllegalArgumentException();
        }
        drawCount[i - 1] = true;
    }

    // some bets might be null
    public Bet[] getPlacedBets() {
        return placedBets;
    }

    public int getDraw() {
        int c = 1;
        for(int i = 0; i < 10; i++) {
            if(drawCount[i]) c = i + 1;
        }
        return c;
    }

}
