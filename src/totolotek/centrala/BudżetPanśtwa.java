package totolotek.centrala;

public class BudżetPanśtwa {
    private long podatki = 0, subwencje = 0;

    public void wpłaćPodatek(long cnt) {
        podatki += cnt;
    }

    public long wniosekSubwencji(long cnt) {
        subwencje += cnt;
        return cnt;
    }

    public long getPodatki() {
        return podatki;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        long podatkiZl = podatki / 100;
        long podatkiGr = podatki % 100;

        long subwencjeZl = subwencje / 100;
        long subwencjeGr = subwencje % 100;

        sb.append("Podatki: ")
                .append(podatkiZl)
                .append(',')
                .append(String.format("%02d", podatkiGr))
                .append(" zł\n");

        sb.append("Subwencje: ")
                .append(subwencjeZl)
                .append(',')
                .append(String.format("%02d", subwencjeGr))
                .append(" zł");

        return sb.toString();
    }

}
