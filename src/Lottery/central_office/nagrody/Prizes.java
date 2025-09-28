package Lottery.central_office.nagrody;

import Lottery.central_office.losowanie.Draw;
import Lottery.participation_in_lottery.Bet;

public class Prizes {
    private final Draw draw;
    private final long[] prizes = new long[4];
    private final int[] prizesCount = new int[4];

    public Prizes(Draw draw, long stepIprizes, long stepIIprizes, long stepIIIprizes, long stepIVprizes, int[] prizesCount) {
        if(draw == null)
            throw new IllegalArgumentException("Draw nie może być null");
        this.draw = draw;

        this.prizes[0] = stepIprizes;
        this.prizesCount[0] = prizesCount[0];

        this.prizes[1] = stepIIprizes;
        this.prizesCount[1] = prizesCount[1];

        this.prizes[2] = stepIIIprizes;
        this.prizesCount[2] = prizesCount[2];

        this.prizes[3] = stepIVprizes;
        this.prizesCount[3] = prizesCount[3];
    }

    public long setPrizes(Bet currentBet) {
        if(currentBet == null)
            throw new IllegalArgumentException("Bet nie może być null");

        Bet winnerBet = draw.getWinnerDraw();

        int val = 6 - currentBet.numbersIntersection(winnerBet);
        if (val < 0 || val > 3) {
            return 0;
        }
        int winners = prizesCount[val];
        if(winners == 0) throw new IllegalArgumentException("Jest zero wygranych to nie pownwinno się zdarzyć");
        return this.prizes[val] / winners;
    }

    public long[] getPoolPrizes() {
        return this.prizes;
    }

    public int[] getWinners() {
        return this.prizesCount;
    }

    private String formatAmount(long grosze) {
        long zł = grosze / 100;
        long gr = grosze % 100;
        return String.format("%dzł %02dgr", zł, gr);
    }

    @Override
    public String toString() {
        var sb = new StringBuilder();
        String[] deg = {"I", "II", "III", "IV"};
        sb.append(draw).append("\n");

        for (int i = 0; i < deg.length; i++) {
            int wygranych = prizesCount[i];
            sb.append(String.format("  Prizes %s degree:\n", deg[i]))
                    .append(String.format("    Pool: %s\n", formatAmount(prizes[i])))
                    .append(String.format("    Winner lottery tickets: %d\n", wygranych));

            if (wygranych > 0) {
                sb.append(String.format("    Amount won: %s\n",
                        formatAmount(prizes[i] / wygranych)));
            }
        }
        return sb.toString();
    }


}
