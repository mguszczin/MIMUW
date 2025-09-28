package Lottery.participation_in_lottery.lotteryTicketID;

import java.util.Random;

public class GenerateID {

    private final Random random = new Random();
    private int currentLotteryTicketNumber = 1;

    public LotteryTicketID generate(int lotteryOfficeId) {
        int lotteryTicketId = currentLotteryTicketNumber++;
        String marker = generateMarker();
        String checkSum = calculateCheckSum(lotteryTicketId, lotteryOfficeId, marker);
        return new LotteryTicketID(lotteryTicketId, lotteryOfficeId, marker, checkSum);
    }

    private String generateMarker() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 9; i++) {
            sb.append(random.nextInt(10));
        }
        return sb.toString();
    }

    private String calculateCheckSum(int lotteryTicketId, int lotteryOfficeId, String marker) {
        int sum = digitSum(lotteryTicketId) + digitSum(lotteryOfficeId) + digitSum(marker);
        int modulo = sum % 100;
        return String.format("%02d", modulo);
    }

    private int digitSum(int cur) {
        int sum = 0;
        while (cur > 0) {
            sum += cur % 10;
            cur /= 10;
        }
        return sum;
    }

    private int digitSum(String text) {
        int sum = 0;
        for (char c : text.toCharArray()) {
            if (Character.isDigit(c)) {
                sum += Character.getNumericValue(c);
            }
        }
        return sum;
    }

}
