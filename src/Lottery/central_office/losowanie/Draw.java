package Lottery.central_office.losowanie;

import Lottery.participation_in_lottery.Bet;

public class Draw {

    int drawNumber;

    Bet winnerBet;

    public Draw(int i, Bet winner) {
        this.drawNumber = i;
        this.winnerBet = winner;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Draw nr ").append(drawNumber).append("\n");
        sb.append("Results: ");
        sb.append(winnerBet);
        return sb.toString();
    }


    public Bet getWinnerDraw() {
        return this.winnerBet;
    }

    public int getDrawNumber() { return this.drawNumber; }
}
