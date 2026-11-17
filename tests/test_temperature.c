#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "I2C.h"
#include "temperature.h"

#define CHECK(condition) do {                                      \
    if (!(condition)) {                                             \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(EXIT_FAILURE);                                         \
    }                                                               \
} while (0)

enum operation { START, WRITE, READ, STOP };

struct step {
    enum operation operation;
    unsigned char argument;
    unsigned char response;
};

static const struct step *next_step;
static size_t steps_left;

static unsigned char expect_step(enum operation operation, unsigned char argument)
{
    CHECK(steps_left > 0);
    CHECK(next_step->operation == operation);
    CHECK(next_step->argument == argument);

    steps_left--;
    return (next_step++)->response;
}

void start_i2c(void)
{
    expect_step(START, 0);
}

void stop_i2c(void)
{
    expect_step(STOP, 0);
}

char write_i2c(unsigned char byte)
{
    return (char)expect_step(WRITE, byte);
}

unsigned char read_i2c(char nack)
{
    return expect_step(READ, (unsigned char)nack);
}

static void check_read(unsigned int raw, int expected)
{
    int hundredths = INT_MAX;
    const struct step transaction[] = {
        {START, 0, 0}, {WRITE, 0x30, 0}, {WRITE, 0x05, 0},
        {START, 0, 0}, {WRITE, 0x31, 0},
        {READ, 0, (unsigned char)(raw >> 8)}, {READ, 1, (unsigned char)raw},
        {STOP, 0, 0}
    };

    next_step = transaction;
    steps_left = sizeof transaction / sizeof transaction[0];
    CHECK(read_temperature(&hundredths) == 0);
    CHECK(steps_left == 0);
    CHECK(hundredths == expected);
}

static void check_nack(size_t write_step)
{
    int hundredths = 2375;
    struct step transaction[] = {
        {START, 0, 0}, {WRITE, 0x30, 0}, {WRITE, 0x05, 0},
        {START, 0, 0}, {WRITE, 0x31, 0}, {STOP, 0, 0}
    };

    transaction[write_step].response = 1;
    transaction[write_step + 1] = (struct step){STOP, 0, 0};
    next_step = transaction;
    steps_left = write_step + 2;
    CHECK(read_temperature(&hundredths) == -1);
    CHECK(steps_left == 0);
    CHECK(hundredths == 2375);
}

int main(void)
{
    const struct { unsigned int raw; int hundredths; } temperatures[] = {
        {0x0000, 0}, {0x0001, 6}, {0x017C, 2375}, {0x0FFF, 25593},
        {0x1000, -25600}, {0x1E64, -2575}, {0x1FFF, -7}
    };

    for (size_t i = 0; i < sizeof temperatures / sizeof temperatures[0]; i++) {
        for (unsigned int alerts = 0; alerts < 8; alerts++) {
            check_read(temperatures[i].raw | (alerts << 13),
                       temperatures[i].hundredths);
        }
    }
    puts("PASS: 56 signed register and alert-bit cases");

    CHECK(temperature_display_bcd(INT_MIN) == 0);
    CHECK(temperature_display_bcd(-1) == 0);
    CHECK(temperature_display_bcd(0) == 0);
    CHECK(temperature_display_bcd(1) == 0x0001);
    CHECK(temperature_display_bcd(100) == 0x0100);
    CHECK(temperature_display_bcd(2375) == 0x2375);
    CHECK(temperature_display_bcd(9999) == 0x9999);
    CHECK(temperature_display_bcd(10000) == 0);
    CHECK(temperature_display_bcd(INT_MAX) == 0);
    puts("PASS: 9 four-digit BCD cases");

    check_nack(1);
    check_nack(2);
    check_nack(4);
    puts("PASS: 3 I2C NACK paths issue STOP without updating the temperature");

    return EXIT_SUCCESS;
}
