/*
 * Copyright 2013-2020 Real Logic Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <iostream>
#include <string>

#include <stdio.h>
#include <iomanip>

#include "baseline/baseline_cpp.h"

using namespace std;
using namespace baseline;

#if defined(WIN32) || defined(_WIN32)
#    define snprintf _snprintf
#endif /* WIN32 */

char VEHICLE_CODE[] = {'a', 'b', 'c', 'd', 'e', 'f'};
char MANUFACTURER_CODE[] = {'1', '2', '3'};
const char *MANUFACTURER = "Honda";
const char *MODEL = "Civic VTi";
const int messageHeaderVersion = 0;

std::size_t encodeHdr(MessageHeader &hdr, Car &car, char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
{
    // encode the header
    hdr.wrap(buffer, offset, messageHeaderVersion, bufferLength)
       .blockLength(Car::sbeBlockLength())
       .templateId(Car::sbeTemplateId())
       .schemaId(Car::sbeSchemaId())
       .version(Car::sbeSchemaVersion());

    return static_cast<std::size_t>(hdr.encodedLength());
}

std::size_t decodeHdr(MessageHeader &hdr, char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
{
    hdr.wrap(buffer, offset, messageHeaderVersion, bufferLength);

    // decode the header
    cout << "messageHeader.blockLength=" << hdr.blockLength() << endl;
    cout << "messageHeader.templateId=" << hdr.templateId() << endl;
    cout << "messageHeader.schemaId=" << hdr.schemaId() << endl;
    cout << "messageHeader.schemaVersion=" << hdr.version() << endl;
    cout << "messageHeader.encodedLength=" << hdr.encodedLength() << endl;

    return static_cast<std::size_t>(hdr.encodedLength());
}

std::size_t encodeCar(Car &car, char *buffer, std::uint64_t offset, std::uint64_t bufferLength)
{
    car.wrapForEncode(buffer, offset, bufferLength)
       .serialNumber(1234)
       .modelYear(2013)
       .available(BooleanType::T)
       .code(Model::A)
       .putVehicleCode(VEHICLE_CODE);

    for (std::uint64_t i = 0, size = car.someNumbersLength(); i < size; i++)
    {
        car.someNumbers(i, static_cast<std::int32_t>(i));
    }

    car.extras().clear()
       .cruiseControl(true)
       .sportsPack(true)
       .sunRoof(false);

    car.engine()
       .capacity(2000)
       .numCylinders((short)4)
       .putManufacturerCode(MANUFACTURER_CODE)
       .efficiency((std::int8_t)35)
       .boosterEnabled(BooleanType::T)
       .booster().boostType(BoostType::NITROUS).horsePower(200);

    CarGroups::FuelFigures& fuelFigures = car.fuelFiguresCount(3);

    fuelFigures
        .next().speed(30).mpg(35.9f)
        .putUsageDescription("Urban Cycle", 11);

    fuelFigures
        .next().speed(55).mpg(49.0f)
        .putUsageDescription("Combined Cycle", 14);

    fuelFigures
        .next().speed(75).mpg(40.0f)
        .putUsageDescription("Highway Cycle", 13);

    CarGroups::PerformanceFigures &performanceFigures = car.performanceFiguresCount(2);

    performanceFigures.next()
        .octaneRating((short)95)
        .accelerationCount(3)
            .next().mph(30).seconds(4.0f)
            .next().mph(60).seconds(7.5f)
            .next().mph(100).seconds(12.2f);

    performanceFigures.next()
        .octaneRating((short)99)
        .accelerationCount(3)
            .next().mph(30).seconds(3.8f)
            .next().mph(60).seconds(7.1f)
            .next().mph(100).seconds(11.8f);

    car.putManufacturer(MANUFACTURER, 5)
        .putModel(MODEL, 9)
        .putActivationCode("deadbeef", 8);

    return static_cast<std::size_t>(car.encodedLength());
}

const char *format(BooleanType::Value value)
{
    if (value == BooleanType::T)
    {
        return "T";
    }
    else
    {
        return "F";
    }
}

const char *format(Model::Value value)
{
    switch (value)
    {
        case Model::A: return "A";
        case Model::B: return "B";
        case Model::C: return "C";
        case Model::NULL_VALUE: return "NULL";
        default: return "unknown";
    }
}

const char *format(BoostType::Value value)
{
    switch (value)
    {
        case BoostType::NITROUS: return "NITROUS";
        case BoostType::TURBO: return "TURBO";
        case BoostType::SUPERCHARGER: return "SUPERCHARGER";
        case BoostType::KERS: return "KERS";
        default: return "unknown";
    }
}

const char *format(bool value)
{
    if (value)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

std::size_t decodeCar(
    Car &car,
    char *buffer,
    std::uint64_t offset,
    std::uint64_t actingBlockLength,
    std::uint64_t actingVersion,
    std::uint64_t bufferLength)
{
    car.wrapForDecode(buffer, offset, actingBlockLength, actingVersion, bufferLength);

    std::cout.setf(std::ios::fixed);

    std::cout << "\ncar.serialNumberId=" << Car::serialNumberId();
    std::cout << "\ncar.modelYearId=" << Car::modelYearId();
    std::cout << "\ncar.availableId=" << Car::availableId();
    std::cout << "\ncar.codeId=" << Car::codeId();
    std::cout << "\ncar.someNumbersId=" << Car::someNumbersId();
    std::cout << "\ncar.vehicleCodeId=" << Car::vehicleCodeId();
    std::cout << "\ncar.extrasId=" << Car::extrasId();
    std::cout << "\ncar.engineId=" << Car::engineId();
    std::cout << "\ncar.fuelFiguresId=" << Car::fuelFiguresId();
    std::cout << "\ncar.fuelFigures.speedId=" << CarGroups::FuelFigures::speedId();
    std::cout << "\ncar.fuelFigures.mpgId=" << CarGroups::FuelFigures::mpgId();
    std::cout << "\ncar.fuelFigures.usageDescriptionId=" << CarGroups::FuelFigures::usageDescriptionId();
    std::cout << "\ncar.performanceFiguresId=" << Car::performanceFiguresId();
    std::cout << "\ncar.performanceFigures.octaneRatingId=" << CarGroups::PerformanceFigures::octaneRatingId();
    std::cout << "\ncar.performanceFigures.accelerationId=" << CarGroups::PerformanceFigures::accelerationId();
    std::cout << "\ncar.performanceFigures.acceleration.mphId=" << CarGroups::PerformanceFiguresGroups::Acceleration::mphId();
    std::cout << "\ncar.performanceFigures.acceleration.secondsId=" << CarGroups::PerformanceFiguresGroups::Acceleration::secondsId();
    std::cout << "\ncar.manufacturerId=" << Car::manufacturerId();
    std::cout << "\ncar.manufacturerCharacterEncoding=" << Car::manufacturerCharacterEncoding();
    std::cout << "\ncar.modelId=" << Car::modelId();
    std::cout << "\ncar.modelCharacterEncoding=" << Car::modelCharacterEncoding();
    std::cout << "\ncar.activationCodeId=" << Car::activationCodeId();
    std::cout << "\ncar.activationCodeCharacterEncoding=" << Car::activationCodeCharacterEncoding();

    std::cout << "\n";

    std::cout << "\ncar.serialNumber=" << car.serialNumber();
    std::cout << "\ncar.modelYear=" << car.modelYear();
    std::cout << "\ncar.available=" << format(car.available());
    std::cout << "\ncar.code=" << format(car.code());

    std::cout << "\ncar.someNumbers=";
    std::string separator("");
    for (std::uint64_t i = 0; i < Car::someNumbersLength(); i++)
    {
        std::cout << separator << car.someNumbers(i);
        separator = ", ";
    }

    std::cout << "\ncar.vehicleCode=";
    separator = "";
    for (std::uint64_t i = 0; i < Car::vehicleCodeLength(); i++)
    {
        std::cout << separator << car.vehicleCode(i);
        separator = ", ";
    }

    OptionalExtras &extras = car.extras();
    std::cout << "\ncar.extras.cruiseControl=" << format(extras.cruiseControl());
    std::cout << "\ncar.extras.sportsPack=" << format(extras.sportsPack());
    std::cout << "\ncar.extras.sunRoof=" << format(extras.sunRoof());

    std::cout << "\ncar.discountedModel=" << format(car.discountedModel());

    Engine &engine = car.engine();
    std::cout << "\ncar.engine.capacity=" << (int)engine.capacity();
    std::cout << "\ncar.engine.numCylinders=" << (int)engine.numCylinders();
    std::cout << "\ncar.engine.maxRpm=" << (int)engine.maxRpm();
    std::cout << "\ncar.engine.manufacturerCodeLength=" << (int)engine.manufacturerCodeLength();
    std::cout << "\ncar.engine.manufacturerCode=";
    separator = "";
    for (std::uint64_t i = 0; i < Engine::manufacturerCodeLength(); i++)
    {
        std::cout << separator << engine.manufacturerCode(i);
        separator = ", ";
    }

    char tmp[1024];
    std::uint64_t bytesCopied = engine.getFuel(tmp, sizeof(tmp));
    std::cout << "\ncar.engine.efficiency=" << (int)engine.efficiency();
    std::cout << "\ncar.engine.boosterEnabled=" << format(engine.boosterEnabled());
    std::cout << "\ncar.engine.fuelLength=" << bytesCopied;
    std::cout << "\ncar.engine.fuel=" << std::string(tmp, static_cast<std::size_t>(bytesCopied));
    std::cout << "\ncar.engine.booster.boostType=" << format(engine.booster().boostType());
    std::cout << "\ncar.engine.booster.horsePower=" << (int)engine.booster().horsePower();

    CarGroups::FuelFigures &fuelFigures = car.fuelFigures();
    while (fuelFigures.hasNext())
    {
        fuelFigures.next();
        std::cout << "\ncar.fuelFigures.speed=" << (int)fuelFigures.speed();
        std::cout << "\ncar.fuelFigures.mpg=" << std::setprecision(1) << (double)fuelFigures.mpg();

        std::string usageDesc = fuelFigures.getUsageDescriptionAsString();
        std::cout << "\ncar.fuelFigures.usageDescriptionLength=" << usageDesc.length();
        std::cout << "\ncar.fuelFigures.usageDescription=" << usageDesc;
    }

    CarGroups::PerformanceFigures &performanceFigures = car.performanceFigures();
    while (performanceFigures.hasNext())
    {
        performanceFigures.next();
        std::cout << "\ncar.performanceFigures.octaneRating=" << (std::uint64_t)performanceFigures.octaneRating();

        CarGroups::PerformanceFiguresGroups::Acceleration &acceleration = performanceFigures.acceleration();
        while (acceleration.hasNext())
        {
            acceleration.next();
            std::cout << "\ncar.performanceFigures.acceleration.mph=" << acceleration.mph();
            std::cout << "\ncar.performanceFigures.acceleration.seconds=" << std::setprecision(1) << acceleration.seconds();
        }
    }

    bytesCopied = car.getManufacturer(tmp, sizeof(tmp));
    std::cout << "\ncar.manufacturerLength=" << bytesCopied;
    std::cout << "\ncar.manufacturer=" << std::string(tmp, static_cast<std::size_t>(bytesCopied));

    bytesCopied = car.getModel(tmp, sizeof(tmp));
    std::cout << "\ncar.modelLength=" << bytesCopied;
    std::cout << "\ncar.model=" << std::string(tmp, static_cast<std::size_t>(bytesCopied));

    bytesCopied = car.getActivationCode(tmp, sizeof(tmp));
    std::cout << "\ncar.activationCodeLength=" << bytesCopied;
    std::cout << "\ncar.activationCode=" << std::string(tmp, static_cast<std::size_t>(bytesCopied));

    std::cout << "\ncar.encodedLength=" << (int)car.encodedLength() << "\n";

    return static_cast<std::size_t>(car.encodedLength());
}

int main(int argc, const char* argv[])
{
    char buffer[2048];
    MessageHeader hdr;
    Car car;

    std::size_t encodeHdrLength = encodeHdr(hdr, car, buffer, 0, sizeof(buffer));
    std::size_t encodeMsgLength = encodeCar(car, buffer, hdr.encodedLength(), sizeof(buffer));

    const CarGroups::FuelFiguresLengthParam fuelFigures[] = {{11}, {14}, {13}};
    const CarGroups::PerformanceFiguresLengthParam performanceFigures[] = {{3}, {3}};
    const CarLengthParam lengthInfo = {
        sbe_vector_view_make(fuelFigures),
        sbe_vector_view_make(performanceFigures),
        5,
        9,
        8
    };

    std::size_t predictedLength = Car::computeLength(lengthInfo);

    cout << "Encoded Lengths are " << encodeHdrLength << " + " << encodeMsgLength << " (" << predictedLength << ")" << endl;

    std::size_t decodeHdrLength = decodeHdr(hdr, buffer, 0, sizeof(buffer));
    std::size_t decodeMsgLength = decodeCar(car, buffer, hdr.encodedLength(), hdr.blockLength(), hdr.version(), sizeof(buffer));

    cout << "Decoded Lengths are " << decodeHdrLength << " + " << decodeMsgLength << endl;

    if (encodeHdrLength != decodeHdrLength)
    {
        cerr << "Encode/Decode header lengths do not match\n";
        return EXIT_FAILURE;
    }

    if (encodeMsgLength != decodeMsgLength)
    {
        cerr << "Encode/Decode message lengths do not match\n";
        return EXIT_FAILURE;
    }

    car.wrapForDecode(buffer, hdr.encodedLength(), hdr.blockLength(), hdr.version(), sizeof(buffer));
    cout << "Encoded json: '" << car << "'" << endl;

    return EXIT_SUCCESS;
}
