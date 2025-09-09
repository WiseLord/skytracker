# Skytracker STM32F103/F303 version

## Principles

Equatorial mount is based on 3D-printing project from
https://www.thingiverse.com/thing:4593838 and has the following configuration:

```
┏━━━━━━━┓
┃       ┃ ┏━━┓
┃ Motor ┣━┫  ┣  Small pinion with 10 teath
┃       ┃ ┗┳┳┛
┗━━━━━━━┛  ┃┃
           ┃┃ 1/6
        ┏━━┫┃
        ┫  ┃┣   Double pinion with 60/15 teath
        ┗┳┳┫┃
         ┃┃┃┃
   ┏━━━━━┫┃┃┃ 1/4
   ┃     ┃┃┗┛
   ┃Mount┃┣     Large pinion with 60 teath
   ┃     ┃┃
   ┗━━━━━┫┃
         ┃┃
         ┗┛
```

# Stars

Star day lasts  23h 56m 04s = 86164s. This is a time period while:

- Large pinion does 1 evolution;
- Double pinion does 4 evolutions;
- Small pinion does 24 evolutions.

Used motor 17HS4401 has 200 steps per evolution.
Used driver TMC2209 is configured for 64 microsteps.

So, we have: 24 * 200 * 64 = 307200 microsteps per 86164 seconds.

MCU is clocked at 72 MHz, i.e.:

307200 microsteps <=> 86164 * 72000000 clocks;
1 microstep <=> 20194687.5 clocks.

Number 20194687 has two good divisors 46639 * 433 good enough to configure
hardware timer (they both belong to 1..65536 range).

# Moon

Moon returns to the same star after 27.322 days or 2360621 seconds. Roughly,
it "moves" in the opposite direction than sky. So, we have a period

1/x = 1/86164 - 1/2360621, from where x = 89428.2 seconds. So,

307200 microsteps <=> 89428.2 * 72000000 clocks;
1 microstep <=> 20959729 steps.

Good good divisors are e.g. 8197 * 2557.

