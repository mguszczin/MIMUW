package Lottery.participation_in_lottery;

import Lottery.central_office.InfrastrukturaCentrali.LotteryOffice;
import Lottery.participation_in_lottery.lotteryTicketID.LotteryTicketID;

import java.util.List;

public class LotteryTicket {

    private final LotteryTicketID ordinalNumber;

    private final List<Bet> placedEntries;

    private final int drawNumber, firstDrawNumber;

    private final long price, tax;

    private final LotteryOffice lotteryOffice;

    public LotteryTicket(LotteryTicketID ordinalNumber, List<Bet> placedEntries,
                         int drawNumber, int firstDrawNumber, long price, long tax, LotteryOffice lotteryOffice) {
        this.ordinalNumber = ordinalNumber;
        this.placedEntries = placedEntries;
        this.drawNumber = drawNumber;
        this.firstDrawNumber = firstDrawNumber;
        this.price = price;
        this.tax = tax;
        this.lotteryOffice = lotteryOffice;
    }

    public LotteryTicketID getId() {
        return ordinalNumber;
    }

    public long getPrice() {
        return price;
    }

    public LotteryOffice getLotteryOffice() {
        return lotteryOffice;
    }

    public long getTax() {
        return tax;
    }

    public int getDrawNumber() {
        return this.drawNumber;
    }

    public int getFirstDrawNumber() {
        return this.firstDrawNumber;
    }

    public List<Bet> getPlacedBets() {
        return this.placedEntries;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("LOTERY TICKET NUMBER ").append(ordinalNumber).append("\n");

        int betCnt = 1;
        for (Bet bet : placedEntries) {
            sb.append(betCnt++).append(": ").append(bet).append("\n");
        }

        sb.append("DRAW COUNT: ").append(drawNumber).append("\n");

        sb.append("DRAW NUMBER:\n");
        for (int i = 0; i < drawNumber; i++) {
            sb.append(firstDrawNumber + i).append(" ");
        }
        sb.append("\n");

        long złote = price / 100;
        long grosze = price % 100;

        sb.append("PRICE: ").append(złote).append(" zł ");
        if (grosze < 10) sb.append("0");
        sb.append(grosze).append(" gr");

        return sb.toString();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        LotteryTicket lotteryTicket = (LotteryTicket) o;
        return ordinalNumber.equals(lotteryTicket.ordinalNumber);
    }

    @Override
    public int hashCode() {
        return ordinalNumber.hashCode();
    }

}
