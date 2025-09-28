package Lottery.player;

import Lottery.central_office.InfrastrukturaCentrali.LotteryOffice;
import Lottery.participation_in_lottery.Form;
import Lottery.participation_in_lottery.Bet;

public class fixedNumberPlayer extends fixedFormPlayer {

    public fixedNumberPlayer(String name, String lastName, String pesel, LotteryOffice[] favoriteLotteryOffice, Bet favoriteNumbers) {
        super(name,lastName,pesel, favoriteLotteryOffice, new Form(10, favoriteNumbers));
    }

}
