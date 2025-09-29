package Lottery.central_office.InfrastrukturaCentrali;

import Lottery.central_office.losowanie.BetGenerator;
import Lottery.central_office.losowanie.Draw;
import Lottery.participation_in_lottery.Bet;
import Lottery.participation_in_lottery.Form;
import Lottery.participation_in_lottery.LotteryTicket;
import Lottery.player.Player;
import Lottery.participation_in_lottery.lotteryTicketID.LotteryTicketID;

import java.util.*;

public class LotteryOffice {

    private final int lotteryOfficeId;

    private final LotteryHeadquarters lotteryHeadquarters;

    private final Map<LotteryTicket, Boolean> boughtLotteryTickets = new HashMap<>();

    private final long BET_PRICE = 3_00;

    protected LotteryOffice(LotteryHeadquarters lotteryHeadquarters, int lotteryOfficeId) {
        if(lotteryHeadquarters == null) {
            throw new IllegalArgumentException("Lottery headquarters cannot be null");
        }
        this.lotteryHeadquarters = lotteryHeadquarters;
        this.lotteryOfficeId = lotteryOfficeId;
    }

    private void checkBets(List<Bet> bets, Bet getWinnersDraw, int prizes[]) {
        for(Bet x : bets) {
            int val = 6 - x.numbersIntersection(getWinnersDraw);
            if(val >= 0 && val < 4) prizes[val]++;
        }
    }

    public void howManyPrizes(int prizes[], Draw draw) {
        if(draw == null) {
            throw new IllegalArgumentException("Draw nie może być null");
        }
        if(prizes.length != 4) {
            throw new IllegalArgumentException("prizes.length != 4");
        }
        Bet winnerDraw = draw.getWinnerDraw();
        int currentDraw = draw.getDrawNumber();

        for (Map.Entry<LotteryTicket, Boolean> entry : boughtLotteryTickets.entrySet()) {
            LotteryTicket lotteryTicket = entry.getKey();
            if(lotteryTicket.getFirstDrawNumber() <= currentDraw &&
            lotteryTicket.getFirstDrawNumber() + lotteryTicket.getDrawNumber() > currentDraw) {
                List<Bet> zakłady = lotteryTicket.getPlacedBets();
                checkBets(zakłady,winnerDraw, prizes);
            }
        }
    }

    public long getLotteryTickets(LotteryTicket lotteryTicket) {
        if(lotteryTicket == null) {
            throw new IllegalArgumentException("LotteryTicket nie może być null");
        }
        if(!boughtLotteryTickets.containsKey(lotteryTicket) || !boughtLotteryTickets.get(lotteryTicket)) {
            return 0;
        }
        boughtLotteryTickets.put(lotteryTicket, false);
        long won = this.lotteryHeadquarters.getFunds(lotteryTicket);
        return won;
    }

    public void sellLotteryTicketForm(Form form, Player player) {
        if(player == null)
            throw new IllegalArgumentException("Player cannot be null");
        if(form == null)
            throw new IllegalArgumentException("Form cannot be null");

        List<Bet> goodBets = new ArrayList<>(Arrays.asList(form.getPlacedBets()));
        goodBets.removeIf(z -> z == null || !z.isCorrect());

        if(goodBets.isEmpty()) throw new NoSuchElementException("Invalid form");

        long price = BET_PRICE * goodBets.size() * form.getDraw();
        if(player.addFunds(-price) == -1) return;

        LotteryTicketID id = this.lotteryHeadquarters.newId(lotteryOfficeId);
        long tax = 0;
        for(int i = 0; i < form.getDraw(); i++) {
            tax += lotteryHeadquarters.lotteryTicketSold(BET_PRICE, goodBets.size(),
                    lotteryHeadquarters.getNextDrawNumber() + i);
        }
        LotteryTicket nowyLotteryTicket = new LotteryTicket(id, goodBets, form.getDraw(), lotteryHeadquarters.getNextDrawNumber(),
                price, tax, this);
        boughtLotteryTickets.put(nowyLotteryTicket, true);
        player.giveLotteryTicket(nowyLotteryTicket);
    }

    public void sellRandomLotteryTicket(int liczbaLosowań, int betCount, Player player) {
        if (betCount > 8 || betCount <= 0)
            throw new IllegalArgumentException("wrong bet count");
        if (liczbaLosowań < 1 || liczbaLosowań > 10)
            throw new IllegalArgumentException("wrong draw count");

        List<Bet> bets = new ArrayList<>();
        while (bets.size() < betCount) {
            bets.add(BetGenerator.generateBet());
        }

        Form form = new Form(liczbaLosowań, bets.toArray(new Bet[0]));
        sellLotteryTicketForm(form, player);
    }



}
