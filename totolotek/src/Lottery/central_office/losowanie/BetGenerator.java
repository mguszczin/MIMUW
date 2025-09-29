package Lottery.central_office.losowanie;

import Lottery.participation_in_lottery.Bet;

import java.util.HashSet;
import java.util.Random;
import java.util.Set;

public class BetGenerator {

    private static final Random random = new Random();
    private static int nextDrawNumber = 1;

    public static Bet generateBet() {
        Set<Integer> numbers = new HashSet<>();
        while (numbers.size() < 6) {
            int number = 1 + random.nextInt(49);
            numbers.add(number);
        }
        return new Bet(numbers);
    }

    public int nextDrawNumber() {
        return nextDrawNumber;
    }

    public Draw generateDraw() {
        Bet winning = generateBet();
        Draw draw = new Draw(nextDrawNumber++, winning);
        return draw;
    }
}
