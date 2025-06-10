package totolotek.udział_w_losowaniu;

import java.util.HashSet;
import java.util.Objects;
import java.util.Set;

public class Zakład {

    private final Set<Integer> obstawioneLiczby;

    public Zakład (Set<Integer> obstawioneLiczby) {
        if(!obstawioneLiczby.stream().allMatch(x -> x >= 1 && x <= 49)) {
            throw new IllegalArgumentException("Zły zakres liczb");
        }
        this.obstawioneLiczby = obstawioneLiczby;
    }

    public Set<Integer> dajLiczby() {
        return obstawioneLiczby;
    }

    public boolean czyPoprawny() {
        return (obstawioneLiczby.size() == 6);
    }

    public int wspólneLiczby(Zakład innyZakład) {
        if(innyZakład == null) throw new NullPointerException();

        Set<Integer> inneLiczby = innyZakład.dajLiczby();
        Set<Integer> teLiczby = new HashSet<>(this.dajLiczby());

        teLiczby.retainAll(inneLiczby);
        return teLiczby.size();
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (!(o instanceof Zakład)) return false;
        Zakład that = (Zakład) o;
        return Objects.equals(this.dajLiczby(), that.dajLiczby()); // Set equality = content-based
    }

    @Override
    public int hashCode() {
        return Objects.hash(obstawioneLiczby); // consistent with equals
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for(int x : obstawioneLiczby) {
            sb.append(String.format("%2d", x)).append(" ");
        }
        return sb.toString();
    }

}
