import Lottery.central_office.GovernmentBudget;
import Lottery.central_office.InfrastrukturaCentrali.LotteryHeadquarters;
import Lottery.central_office.InfrastrukturaCentrali.LotteryOffice;
import Lottery.central_office.losowanie.BetGenerator;
import Lottery.participation_in_lottery.Bet;
import Lottery.player.RandomPlayer;
import Lottery.player.MinimalistPlayer;
import Lottery.player.fixedFormPlayer;
import Lottery.player.fixedNumberPlayer;
import Lottery.participation_in_lottery.Form;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import java.util.concurrent.ThreadLocalRandom;

public class ClassPresentation {

    public static String generatePESEL() {
        Random random = new Random();
        // PESEL = 11 digits: YYMMDD + 5 random + contrlol sum
        int year = 1950 + random.nextInt(60); // 1950–2009
        int month = 1 + random.nextInt(12);
        int day = 1 + random.nextInt(28);

        String yy = String.format("%02d", year % 100);
        String mm = String.format("%02d", month);
        String dd = String.format("%02d", day);

        String serial = String.format("%05d", random.nextInt(100000));
        String pesel = yy + mm + dd + serial;


        return pesel.substring(0, 11);
    }


    public static void main(String[] args) {
        GovernmentBudget budget = new GovernmentBudget();
        LotteryHeadquarters centralOffice = new LotteryHeadquarters(budget , 0);

        LotteryOffice[] lotteryOffices = new LotteryOffice[10];

        for(int i = 0; i < 10; i++) {
            lotteryOffices[i] = centralOffice.createLotteryOffice();
        }

        int playerCount = 200;

        String[] IMIONA = {
                "Jan", "Anna", "Piotr", "Katarzyna", "Tomasz", "Agnieszka", "Michał", "Ewa", "Paweł", "Magdalena"
        };
        String[] NAZWISKA = {
                "Kowalski", "Nowak", "Wiśniewski", "Wójcik", "Kowalczyk", "Kamiński", "Lewandowski", "Zieliński", "Szymański", "Woźniak"
        };

        RandomPlayer[] randomPlayer = new RandomPlayer[playerCount];
        for(int i = 0; i < playerCount; i++) {
            String name = IMIONA[(int)(Math.random() * IMIONA.length)];
            String lastName = NAZWISKA[(int)(Math.random() * NAZWISKA.length)];
            String pesel = generatePESEL();
            randomPlayer[i] = new RandomPlayer(name, lastName, pesel, lotteryOffices);
        }

        MinimalistPlayer[] minimalistPlayer = new MinimalistPlayer[playerCount];
        for(int i = 0; i < playerCount; i++) {
            String name = IMIONA[(int)(Math.random() * IMIONA.length)];
            String lastName = NAZWISKA[(int)(Math.random() * NAZWISKA.length)];
            String pesel = generatePESEL();
            minimalistPlayer[i] = new MinimalistPlayer(name, lastName, pesel, lotteryOffices[(int)(Math.random() * lotteryOffices.length)]);
        }

        fixedFormPlayer[] fixedTicketPlayer = new fixedFormPlayer[playerCount];
        for(int i = 0; i < playerCount; i++) {
            String name = IMIONA[(int)(Math.random() * IMIONA.length)];
            String lastName = NAZWISKA[(int)(Math.random() * NAZWISKA.length)];
            String pesel = generatePESEL();

            // losowy ticket
            int numberOfDraws = ThreadLocalRandom.current().nextInt(1, 11);
            int numberOfBets = ThreadLocalRandom.current().nextInt(1, 9);
            List<Bet> bet = new ArrayList<>();
            while (bet.size() < numberOfBets) {
                bet.add(BetGenerator.generateBet());
            }

            Form ticket = new Form(numberOfDraws, bet.toArray(new Bet[0]));

            fixedTicketPlayer[i] = new fixedFormPlayer(name, lastName, pesel, lotteryOffices, ticket);
        }

        fixedNumberPlayer[] fixedNumberPlayer = new fixedNumberPlayer[playerCount];
        for(int i = 0; i < playerCount; i++) {
            String name = IMIONA[(int)(Math.random() * IMIONA.length)];
            String lastName = NAZWISKA[(int)(Math.random() * NAZWISKA.length)];
            String pesel = generatePESEL();
            fixedNumberPlayer[i] = new fixedNumberPlayer(name, lastName, pesel, lotteryOffices, BetGenerator.generateBet());
        }

        // symulacja gry
        for(int j = 0; j < 20; j++) {
            int draw = centralOffice.getNextDrawNumber();

            for(int i = 0; i < playerCount; i++) {
                randomPlayer[i].wantToBuyLotteryTicket(draw);
                fixedTicketPlayer[i].wantToBuyLotteryTicket(draw);
                minimalistPlayer[i].wantToBuyLotteryTicket(draw);
                fixedNumberPlayer[i].wantToBuyLotteryTicket(draw);
            }

            // generate new draw
            centralOffice.newDraw();

            // check the winners, maybe pay out the prizes
            for(int i = 0; i < playerCount; i++) {
                randomPlayer[i].encashLotteryTickets(draw);
                fixedTicketPlayer[i].encashLotteryTickets(draw);
                minimalistPlayer[i].encashLotteryTickets(draw);
                fixedNumberPlayer[i].encashLotteryTickets(draw);
            }
        }

        System.out.print(randomPlayer[0]);
        System.out.println(centralOffice);
        System.out.println(budget);
    }
}