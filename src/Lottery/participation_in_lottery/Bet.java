package Lottery.participation_in_lottery;

import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

public class Bet {

    private final Set<Integer> markedNumbers;

    public Bet(Set<Integer> markedNumbers) {
        if(!markedNumbers.stream().allMatch(x -> x >= 1 && x <= 49)) {
            throw new IllegalArgumentException("Wrong range of numbers");
        }
        this.markedNumbers = markedNumbers;
    }

    public Set<Integer> getNumbers() {
        return markedNumbers;
    }

    public boolean isCorrect() {
        return (markedNumbers.size() == 6);
    }

    public int numbersIntersection(Bet otherBet) {
        if(otherBet == null) throw new NullPointerException();

        Set<Integer> otherBetNumbers = otherBet.getNumbers();
        Set<Integer> thisBetNumbers = new HashSet<>(this.getNumbers());

        thisBetNumbers.retainAll(otherBetNumbers);
        return thisBetNumbers.size();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Bet)) return false;
        Bet that = (Bet) o;
        return Objects.equals(this.getNumbers(), that.getNumbers()); // Set equality = content-based
    }

    @Override
    public int hashCode() {
        return Objects.hash(markedNumbers); // consistent with equals
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for(int x : markedNumbers) {
            sb.append(String.format("%2d", x)).append(" ");
        }
        return sb.toString();
    }

}
