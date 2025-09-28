package Lottery.central_office;

public class GovernmentBudget {
    private long taxes = 0, subsides = 0;

    public void payTax(long cnt) {
        taxes += cnt;
    }

    public long subsidesApplication(long cnt) {
        subsides += cnt;
        return cnt;
    }

    public long getTaxes() {
        return taxes;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        long taxesZł = taxes / 100;
        long taxesGr = taxes % 100;

        long subsidesZł = subsides / 100;
        long subsidesGr = subsides % 100;

        sb.append("Taxes: ")
                .append(taxesZł)
                .append(',')
                .append(String.format("%02d", taxesGr))
                .append(" zł\n");

        sb.append("Subsides: ")
                .append(subsidesZł)
                .append(',')
                .append(String.format("%02d", subsidesGr))
                .append(" zł");

        return sb.toString();
    }

}
