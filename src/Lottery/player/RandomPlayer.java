package Lottery.player;

import Lottery.central_office.InfrastrukturaCentrali.LotteryOffice;

import java.util.concurrent.ThreadLocalRandom;

public class RandomPlayer extends Player {

    public RandomPlayer(String name, String lastName, String pesel, LotteryOffice[] lotteryOffice) {
        super(name, lastName, pesel, lotteryOffice, 1);
        this.addFunds(ThreadLocalRandom.current().nextLong(0, 100000000));
    }

    @Override
    protected void buyLotteryTicket() {
        int index = ThreadLocalRandom.current().nextInt(0, this.ulubioneKolektury.length);
        LotteryOffice drawnLotteryOffice = this.ulubioneKolektury[index];

        int lotteryTicketsCount = ThreadLocalRandom.current().nextInt(0, 100);
        for(int i = 0; i < lotteryTicketsCount; i++) {
            int draws = ThreadLocalRandom.current().nextInt(1, 11);
            int bets = ThreadLocalRandom.current().nextInt(1, 9);
            drawnLotteryOffice.sellRandomLotteryTicket(draws, bets, this);
        }
    }

}
