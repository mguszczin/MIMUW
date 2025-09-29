package Lottery.player;

import Lottery.central_office.InfrastrukturaCentrali.LotteryOffice;
import Lottery.participation_in_lottery.LotteryTicket;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public abstract class Player {
    private final String pesel, name, lastName;

    private long funds = 0;

    protected final int breakFromLottery;

    protected final LotteryOffice[] ulubioneKolektury;

    private final List<LotteryTicket> posiadaneKupony = new ArrayList<>();

    public Player(String name, String lastName, String pesel, LotteryOffice[] ulubioneKolektury, int breakFromLottery) {
        if(name == null || lastName == null || pesel == null || ulubioneKolektury == null || ulubioneKolektury.length == 0 || breakFromLottery <= 0)
            throw new IllegalArgumentException("Niepoprawne dane gracza");
        this.name = name;
        this.lastName = lastName;
        this.pesel = pesel;
        this.ulubioneKolektury = ulubioneKolektury;
        this.breakFromLottery = breakFromLottery;
    }

    public Player(String name, String lastName, String pesel, LotteryOffice[] ulubioneKolektury, int breakFromLottery, long funds) {
        if(name == null || lastName == null || pesel == null || ulubioneKolektury == null || ulubioneKolektury.length == 0 || breakFromLottery <= 0)
            throw new IllegalArgumentException("Niepoprawne dane gracza");
        this.name = name;
        this.lastName = lastName;
        this.pesel = pesel;
        this.ulubioneKolektury = ulubioneKolektury;
        this.funds = funds;
        this.breakFromLottery = breakFromLottery;
    }

    protected abstract void buyLotteryTicket();

    public void wantToBuyLotteryTicket (int draw) {
        if((draw - 1 + breakFromLottery) % breakFromLottery == 0) buyLotteryTicket();
    }

    public void encashLotteryTickets(int lastDraw) {
        Iterator<LotteryTicket> iterator = this.posiadaneKupony.iterator();
        while (iterator.hasNext()) {
            LotteryTicket lotteryTicket = iterator.next();
            if(lotteryTicket.getFirstDrawNumber() + lotteryTicket.getDrawNumber() - 1 > lastDraw) continue;
            iterator.remove();

            LotteryOffice lotteryOffice = lotteryTicket.getLotteryOffice();
            addFunds(lotteryOffice.getLotteryTickets(lotteryTicket));
        }
    }

    public void giveLotteryTicket(LotteryTicket lotteryTicket) {
        this.posiadaneKupony.add(lotteryTicket);
    }

    public long howManyFunds() {
        return this.funds;
    }

    public int addFunds(long a) {
        if (this.funds + a < 0) return -1;
        this.funds += a;
        return 0;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(name).append(" ").append(lastName).append("\n");
        sb.append("Pesel: ").append(pesel).append("\n");
        sb.append("Funds :").append(this.funds /100).append("zł").append(" ")
                .append(this.funds % 100).append("gr").append("\n");
        if(posiadaneKupony.size() > 0) {
            sb.append("Lottery ticket ID:\n");
            for(LotteryTicket lotteryTicket : posiadaneKupony) {
                sb.append(lotteryTicket.getId()).append("\n");
            }
        }else sb.append("No lottery tickets in holding.\n");
        return sb.toString();
    }
}
