package Lottery.player;

import Lottery.central_office.InfrastrukturaCentrali.LotteryOffice;

import java.util.concurrent.ThreadLocalRandom;

public class MinimalistPlayer extends Player {
    public MinimalistPlayer(String name, String lastName, String pesel, LotteryOffice ulubionaLotteryOffice) {
        super(name, lastName, pesel, new LotteryOffice[]{ulubionaLotteryOffice}, 1);
        this.addFunds(ThreadLocalRandom.current().nextLong(0, 10000000));
    }

    @Override
    protected void buyLotteryTicket() {
        LotteryOffice favoriteLotteryOffice = this.ulubioneKolektury[0];
        favoriteLotteryOffice.sellRandomLotteryTicket(1, 1, this);
    }
}
