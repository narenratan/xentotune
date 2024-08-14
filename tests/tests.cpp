#include <iostream>

#include "MTS-ESP/Client/libMTSClient.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest/doctest.h"

#include "plugin.cpp"

#define SR 48000.0

TEST_CASE("Test 440Hz")
{
    MTSClient *c = MTS_RegisterClient();
    float f = 440.0 / SR;
    float g = mapFreq(f, SR, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(f));
}

TEST_CASE("Test 440Hz different sample rate")
{
    MTSClient *c = MTS_RegisterClient();
    float sr = 44100.0;
    float f = 440.0 / sr;
    float g = mapFreq(f, sr, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(f));
}

TEST_CASE("Test 880Hz")
{
    MTSClient *c = MTS_RegisterClient();
    float sr = 48000.0;
    float f = 880.0 / SR;
    float g = mapFreq(f, SR, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(f));
}

TEST_CASE("Test middle C")
{
    MTSClient *c = MTS_RegisterClient();
    float sr = 48000.0;
    float f = 440.0 * pow(2, -9 / 12) / SR;
    float g = mapFreq(f, SR, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(f));
}

TEST_CASE("Test eighth tone up")
{
    MTSClient *c = MTS_RegisterClient();
    float sr = 48000.0;
    float f = 440.0 * pow(2.0, 1.0 / 48.0) / SR;
    float g = mapFreq(f, SR, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(440.0 / SR));
}

TEST_CASE("Test eighth tone down")
{
    MTSClient *c = MTS_RegisterClient();
    float sr = 48000.0;
    float f = 440.0 * pow(2.0, -1.0 / 48.0) / SR;
    float g = mapFreq(f, SR, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(440.0 / SR));
}

TEST_CASE("Test 3/8 tone up")
{
    MTSClient *c = MTS_RegisterClient();
    float sr = 48000.0;
    float f = 440.0 * pow(2.0, 3.0 / 48.0) / SR;
    float g = mapFreq(f, SR, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(440.0 * pow(2.0, 1.0 / 12.0) / SR));
}

TEST_CASE("Test very large freq")
{
    MTSClient *c = MTS_RegisterClient();
    float sr = 48000.0;
    float f = 1e12 / 48.0;
    float g = mapFreq(f, SR, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(f));
}

TEST_CASE("Test very small freq")
{
    MTSClient *c = MTS_RegisterClient();
    float sr = 48000.0;
    float f = 1e-12 / 48.0;
    float g = mapFreq(f, SR, c);
    MTS_DeregisterClient(c);
    CHECK(g == doctest::Approx(f));
}
