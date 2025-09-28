package Lottery.player;

import Lottery.central_office.InfrastrukturaCentrali.LotteryOffice;
import Lottery.participation_in_lottery.Form;

import java.util.concurrent.ThreadLocalRandom;

public class fixedFormPlayer extends Player {
    private final Form favoriteForm;
    private int currentLotteryOffice = 0;

    public fixedFormPlayer(String name, String lastName, String pesel, LotteryOffice[] favoriteLotteryOffices, Form favoriteForm) {
        super(name, lastName, pesel, favoriteLotteryOffices, favoriteForm.getDraw());
        this.addFunds(ThreadLocalRandom.current().nextLong(0, 1000000));

        if(breakFromLottery <= 0) throw new IllegalArgumentException("Niepoprawna liczba Losowań");

        this.favoriteForm = favoriteForm;
    }

    public fixedFormPlayer(String name, String lastName, String pesel, LotteryOffice[] favoriteLotteryOffice, Form favoriteForm, long funds) {
        super(name, lastName, pesel, favoriteLotteryOffice, favoriteForm.getDraw(), funds);

        if(breakFromLottery <= 0) throw new IllegalArgumentException("Niepoprawna liczba Losowań");

        this.favoriteForm = favoriteForm;
    }


    @Override
    protected void buyLotteryTicket() {
        LotteryOffice lotteryOffice = this.ulubioneKolektury[currentLotteryOffice++ % ulubioneKolektury.length];
        lotteryOffice.sellLotteryTicketForm(this.favoriteForm, this);
    }
}
