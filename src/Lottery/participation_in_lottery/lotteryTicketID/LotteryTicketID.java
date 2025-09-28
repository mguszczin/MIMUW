package Lottery.participation_in_lottery.lotteryTicketID;

public class LotteryTicketID {

    private final int lotteryTicketNumber;
    private final int loteryOfficeID;
    private final String marker;
    private final String checkSum;

    public LotteryTicketID(int loteryTicketNumber, int numerKolektury, String marker, String checkSum) {
        this.lotteryTicketNumber = loteryTicketNumber;
        this.loteryOfficeID = numerKolektury;
        this.marker = marker;
        this.checkSum = checkSum;
    }

    @Override
    public String toString() {
        return lotteryTicketNumber + "-" + loteryOfficeID + "-" + marker + "-" + checkSum;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;
        LotteryTicketID that = (LotteryTicketID) o;
        return lotteryTicketNumber == that.lotteryTicketNumber &&
                loteryOfficeID == that.loteryOfficeID &&
                marker.equals(that.marker) &&
                checkSum.equals(that.checkSum);
    }

    @Override
    public int hashCode() {
        int result = Integer.hashCode(lotteryTicketNumber);
        result = 31 * result + Integer.hashCode(loteryOfficeID);
        result = 31 * result + marker.hashCode();
        result = 31 * result + checkSum.hashCode();
        return result;
    }


    public int getLotteryTicketNumber() {
        return lotteryTicketNumber;
    }

    public int getLoteryOfficeID() {
        return loteryOfficeID;
    }
}
