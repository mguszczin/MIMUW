# Totolotek Lottery System
## Overview

This project implements a game inspired by polish Lottery, a simulation where players participate by purchasing tickets and attempting to match 6 numbers drawn from a pool of 49. The system is designed to model the core functionality of Totolotek, including ticket sales, draw management, prize calculation, and prize distribution.

## Structure 
The system is structured around several key components:
- Central Lottery Office
- Draws
- Players
- Lottery Offices
- Tickets and Entry Forms
- Government Budget

## System Components

### Central Totolotek Office

- Manages funds, including revenues from ticket sales and government subsidies
- Conducts each draw of 6 winning numbers
- Calculates prize pools and individual prize amounts for each tier:
  - I tier – 6 correct numbers
  - II tier – 5 correct numbers
  - III tier – 4 correct numbers
  - IV tier – 3 correct numbers
- Supports cumulative prizes: if no one wins a top-tier prize, it rolls over to the next draw
- Provides public access to:
  - Winning numbers
  - Total prize pools per tier
  - Actual payouts per winning ticket
- Keeps track of its financial balance

### Draws

- Each draw has a unique sequential number
- Stores 6 winning numbers in ascending order
- Can produce a textual representation of its results including draw number and winning numbers
- Prize allocation:
  - 51% of ticket revenue (after tax) goes to prizes, the remainder is profit
  - Prize distribution:
    - I tier: 44% of prize funds
    - II tier: 8%
    - IV tier: fixed at 24 PLN per win
    - III tier: remainder
  - Minimum guaranteed I-tier prize: 2,000,000 PLN
  - III-tier prizes cannot fall below 15x the ticket stake
  - Any fractional cents are rounded down

### Collection Points (Kolektura)

- Each collection point has a unique ID
- Sells tickets and records sales
- Transfers revenue (minus tax) to the central office
- Pays winnings to players from central office funds
- Offers tickets via:
  - Manual entry forms (Blankiet)
  - “Quick pick” (random) tickets

### Entry Forms (Blankiet)

- Each form has 8 numbered fields, representing potential bets
- Each field has 49 numbered squares plus an “annul” option
- A valid bet consists of marking exactly 6 numbers
- Forms also specify number of draws for each ticket (1–10)
- Excessively marked fields or fields with fewer than 6 numbers are ignored

### Tickets (Kupon)

- Issued after payment at a collection point
- Each ticket has:
  - A unique identifier combining ticket number, collection point ID, random digits, and a checksum
  - 1–8 bets
  - Participation in up to 10 sequential draws
  - Price including tax
- Tickets are the only proof of winnings

### Players

- Players can purchase tickets and claim winnings
- Each player has:
  - Name, surname, and PESEL
  - Available funds
  - List of purchased tickets
- Different types of players are supported:
  - Minimalist: always buys a single “quick pick” ticket in a favorite collection point for the next draw
  - Random: buys a random number of tickets in random collection points with random bets and draws
  - Fixed-number player: plays their favorite 6 numbers for 10 draws, waiting for all draws before purchasing another ticket
  - Fixed-form player: plays a pre-defined blank form for a fixed number of draws, alternating among favorite collection points
- The system allows easy addition of other player types

### Prize Collection

- Players claim prizes at the collection point where the ticket was purchased
- The system verifies ticket validity and prevents multiple redemptions
- High-tier prizes (>2280 PLN per bet) are taxed at 10%
- Prizes can be collected at any time. Claims for draws not yet conducted are forfeited
- If the central office lacks funds, it automatically requests the remaining amount from the state budget

### Government Budget

- Collects taxes from ticket sales and prize payouts
- Provides subsidies to the central office if needed
- Tracks total taxes and subsidies

---

## Prize Rules

- Single ticket cost: 3 PLN, including 0.60 PLN tax
- Prize breakdown:
  - I-tier: 44% of prize pool (minimum 2,000,000 PLN)
  - II-tier: 8% of prize pool
  - III-tier: remainder (≥36 PLN per win)
  - IV-tier: 24 PLN fixed
- Prizes are distributed equally among all winning bets in each tier
- Cumulative rules apply if no winners in a tier

---

## How It Works

1. Players purchase tickets in collection points
2. Each draw generates 6 winning numbers
3. The central office calculates prize pools and divides winnings according to the results
4. Players claim prizes using their tickets
5. All financial transactions and prize distributions are logged for public access

---

## Notes

- Players, tickets, and draws are completely anonymous. No personal data is stored beyond basic identifiers
- The system supports multiple draws, multiple players, and multiple collection points simultaneously
- Rounding rules: all fractional cents are rounded down
