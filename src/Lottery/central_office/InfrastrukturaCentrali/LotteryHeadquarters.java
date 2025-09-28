package Lottery.central_office.InfrastrukturaCentrali;

import Lottery.central_office.GovernmentBudget;
import Lottery.central_office.losowanie.BetGenerator;
import Lottery.central_office.losowanie.Draw;
import Lottery.central_office.nagrody.Prizes;
import Lottery.participation_in_lottery.Bet;
import Lottery.participation_in_lottery.LotteryTicket;
import Lottery.participation_in_lottery.lotteryTicketID.GenerateID;
import Lottery.participation_in_lottery.lotteryTicketID.LotteryTicketID;

import java.util.*;

public class LotteryHeadquarters {

    private final GovernmentBudget governmentBudget;

    private final GenerateID generateID = new GenerateID();

    private final BetGenerator betGenerator = new BetGenerator();

    private int lotteryOfficeLastId = 0;

    private long lastPool = 0;

    private long headquartersBudget;

    private long[] lotteryPools = new long[10];

    private Map<Integer, Prizes> prizes;

    private List<LotteryOffice> lotteryOffices;

    private final long I_DEG_LOWER_BOUND_PRIZE = 2_000_000_00;

    private final long III_DEG_LOWER_BOUND_PRIZE = 36_00;

    private final long IV_DEG_LOWER_BOUND_PRIZE = 24_00;

    private final long TAX_BET = 60;

    private final long WINNING_TAX_THRESHOLD = 2280_00;

    private final long WINNING_TAX = 10;

    private Prizes lastPrizes;

    public LotteryHeadquarters(GovernmentBudget governmentBudget, long funds) {
        if(governmentBudget == null)
            throw new IllegalArgumentException("Government budget cannot be null");

        this.governmentBudget = governmentBudget;
        this.headquartersBudget = funds;
        this.prizes = new HashMap<>();
        this.lotteryOffices = new ArrayList<>();
    }

    public LotteryOffice createLotteryOffice() {
        LotteryOffice lotteryOffice = new LotteryOffice(this, ++lotteryOfficeLastId);
        this.lotteryOffices.add(lotteryOffice);
        return lotteryOffice;
    }

    public int getLotteryOfficeCount() {
        return lotteryOfficeLastId;
    }

    public LotteryTicketID newId(int lotteryOfficeId) {
        return this.generateID.generate(lotteryOfficeId);
    }

    public void newDraw() {
        Draw draw = betGenerator.generateDraw();
        int currentDraw = draw.getDrawNumber();

        int[] prizesTresholds = new int[4];
        for(int i = 0; i < this.getLotteryOfficeCount(); i++) {
            this.lotteryOffices.get(i).howManyPrizes(prizesTresholds, draw);
        }

        long środki = 51 *  lotteryPools[currentDraw % 10] / 100;

        long Iprize = Math.max( ((środki * 44) / 100) + lastPool,
                I_DEG_LOWER_BOUND_PRIZE);
        long IIprize = Math.max(((środki * 8) / 100), 0);
        long IVprize = IV_DEG_LOWER_BOUND_PRIZE * (long) prizesTresholds[3];
        long IIIprize = Math.max(((środki * 48) / 100) - IVprize,
                III_DEG_LOWER_BOUND_PRIZE * (long) prizesTresholds[2]);

        Prizes prizes = new Prizes(draw, Iprize, IIprize, IIIprize, IVprize, prizesTresholds);

        this.lastPrizes = prizes;
        int id = draw.getDrawNumber();
        this.prizes.put(id, prizes);

        if(prizesTresholds[0] == 0) lastPool = Iprize;
        else lastPool = 0;
        lotteryPools[(currentDraw % 10)] = 0;
    }

    private long winningTax(long cur) {
        if(cur <= WINNING_TAX_THRESHOLD) return 0;

        long dif = cur - WINNING_TAX_THRESHOLD;
        long taxAmount = dif / WINNING_TAX;

        this.governmentBudget.payTax(taxAmount);
        return taxAmount;
    }

    private void getFunds(long cur) {
        if(cur <= this.headquartersBudget) return;

        this.headquartersBudget = cur - this.headquartersBudget;
        this.governmentBudget.subsidesApplication((cur - this.headquartersBudget) * 2);
    }

    protected long getFunds(LotteryTicket lotteryTicket) {
        if(lotteryTicket == null)
            throw new IllegalArgumentException("LotteryTicket can't be null");

        int initialDraw = lotteryTicket.getFirstDrawNumber();
        List<Bet> bets = lotteryTicket.getPlacedBets();
        long won = 0;

        for(int i = initialDraw; i < initialDraw + lotteryTicket.getDrawNumber(); i++) {
            if(!this.prizes.containsKey(i)) break;
            Prizes prizes = this.prizes.get(i);

            for(Bet x : bets) {
                long cur = prizes.setPrizes(x);

                getFunds(cur);
                this.headquartersBudget -= cur;

                cur -= winningTax(cur);
                won += cur;
            }
        }
        return won;
    }

    protected long lotteryTicketSold(long prize, int amount, int draw) {
        long tax = TAX_BET * amount;
        governmentBudget.payTax(tax);

        lotteryPools[draw % 10] += prize * amount - tax;
        this.headquartersBudget += prize * amount - tax;

        return tax;
    }

    public int getNextDrawNumber() {
        return this.betGenerator.nextDrawNumber();
    }

    private String formatAmount(long grosze) {
        long zł = grosze / 100;
        long gr = grosze % 100;
        return String.format("%dzł %02dgr", zł, gr);
    }

    public Prizes getLastPrize() {
        return lastPrizes;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append("Lottery Headquarter:\n")
                .append("  Headquarter Budget: ").append(formatAmount(headquartersBudget)).append("\n")
                .append("  Lottery payments:\n");

        for (int i = 1; i < getNextDrawNumber(); i++) {
            sb.append(prizes.get(i).toString().indent(2));
        }

        sb.append("  Lottery Offices: ").append(lotteryOffices.size()).append("\n");
        return sb.toString();
    }


}
