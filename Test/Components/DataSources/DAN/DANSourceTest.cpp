/**
 * @file DANSourceTest.cpp
 * @brief Source file for class DANSourceTest
 * @date 18/04/2017
 * @author Andre Neto
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This source file contains the definition of all the methods for
 * the class DANSourceTest (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include "dan/dan_DataCore.h"
#include "dan/dan_Source.h"
#include "dan/reader/dan_stream_reader_cpp.h"
#include "tcn.h"

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "Directory.h"
#include "DirectoryScanner.h"
#include "GAM.h"
#include "DANSource.h"
#include "DANSourceTest.h"
#include "ObjectRegistryDatabase.h"
#include "RealTimeApplication.h"
#include "StandardParser.h"
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/**
 * @brief GAM which generates a given signal trigger, time and signal pattern which is then sinked to the DANSource
 */
class DANSourceGAMTriggerTestHelper: public MARTe::GAM {
public:
    CLASS_REGISTER_DECLARATION()DANSourceGAMTriggerTestHelper() {
        generateTrigger = false;
        useAbsoluteTime = false;
        counter = 0;
        period = 2;
        numberOfExecutes = 0;
        absTimeStamp = 0;
        signalToGenerate = NULL;
        triggerToGenerate = NULL;
        elements = NULL;
    }

    virtual ~DANSourceGAMTriggerTestHelper() {
        using namespace MARTe;
        if (signalToGenerate != NULL) {
            delete [] signalToGenerate;
        }
        if (triggerToGenerate != NULL) {
            delete [] triggerToGenerate;
        }
        if (elements != NULL) {
            delete [] elements;
        }
    }

    virtual bool Initialise(MARTe::StructuredDataI & data) {
        using namespace MARTe;
        bool ok = GAM::Initialise(data);
        AnyType signalAT = data.GetType("Signal");
        numberOfExecutes = signalAT.GetNumberOfElements(0);
        signalToGenerate = new uint32[numberOfExecutes];
        triggerToGenerate = new uint8[numberOfExecutes];
        Vector<uint32> signalV(signalToGenerate, numberOfExecutes);
        data.Read("Signal", signalV);
        Vector<uint8> triggerV(triggerToGenerate, numberOfExecutes);
        generateTrigger = data.Read("Trigger", triggerV);
        data.Read("Period", period);

        uint32 absoluteTime = 0;
        data.Read("AbsoluteTime", absoluteTime);
        useAbsoluteTime = (absoluteTime == 1);
        return ok;
    }

    virtual bool Setup() {
        using namespace MARTe;
        elements = new uint32[GetNumberOfOutputSignals()];
        uint32 n;
        for (n=0; n<GetNumberOfOutputSignals(); n++) {
            uint32 el;
            GetSignalNumberOfElements(OutputSignals, n, el);
            elements[n] = el;
        }
        (void) (tcn_get_time(&absTimeStamp) == TCN_SUCCESS);

        return true;
    }

    virtual bool Execute() {
        using namespace MARTe;
        if (counter < numberOfExecutes) {
            uint32 n = 0;
            uint8 *trigger = NULL;
            uint32 *timeSignal = NULL;
            uint64 *absTimeSignal = NULL;
            if (generateTrigger) {
                trigger = reinterpret_cast<uint8 *>(GetOutputSignalMemory(0));
                *trigger = triggerToGenerate[counter];
                if (useAbsoluteTime) {
                    absTimeSignal = reinterpret_cast<uint64 *>(GetOutputSignalMemory(1));
                    *absTimeSignal = absTimeStamp + (*timeSignal * 1000);
                }
                else {
                    timeSignal = reinterpret_cast<uint32 *>(GetOutputSignalMemory(1));
                    *timeSignal = (counter * (period * 1e6) * elements[2]);
                }
                n = 2;
            }
            for (; n<GetNumberOfOutputSignals(); n++) {
                TypeDescriptor signalType = GetSignalType(OutputSignals, n);
                if (signalType == UnsignedInteger16Bit) {
                    uint16 *signalOut = reinterpret_cast<uint16 *>(GetOutputSignalMemory(n));
                    uint32 e;
                    for (e=0; e<elements[n]; e++) {
                        signalOut[e] = static_cast<uint16>(signalToGenerate[counter]);
                    }
                }
                else if (signalType == UnsignedInteger32Bit) {
                    uint32 *signalOut = reinterpret_cast<uint32 *>(GetOutputSignalMemory(n));
                    uint32 e;
                    for (e=0; e<elements[n]; e++) {
                        signalOut[e] = static_cast<uint32>(signalToGenerate[counter]);
                    }
                }
                else if (signalType == UnsignedInteger64Bit) {
                    uint64 *signalOut = reinterpret_cast<uint64 *>(GetOutputSignalMemory(n));
                    uint64 e;
                    for (e=0; e<elements[n]; e++) {
                        signalOut[e] = static_cast<uint64>(signalToGenerate[counter]);
                    }
                }
                else if (signalType == SignedInteger16Bit) {
                    int16 *signalOut = reinterpret_cast<int16 *>(GetOutputSignalMemory(n));
                    uint32 e;
                    for (e=0; e<elements[n]; e++) {
                        signalOut[e] = static_cast<int16>(signalToGenerate[counter]);
                    }
                }
                else if (signalType == SignedInteger32Bit) {
                    int32 *signalOut = reinterpret_cast<int32 *>(GetOutputSignalMemory(n));
                    uint32 e;
                    for (e=0; e<elements[n]; e++) {
                        signalOut[e] = static_cast<int32>(signalToGenerate[counter]);
                    }
                }
                else if (signalType == SignedInteger64Bit) {
                    int64 *signalOut = reinterpret_cast<int64 *>(GetOutputSignalMemory(n));
                    uint32 e;
                    for (e=0; e<elements[n]; e++) {
                        signalOut[e] = static_cast<int64>(signalToGenerate[counter]);
                    }
                }
                else if (signalType == Float32Bit) {
                    float32 *signalOut = reinterpret_cast<float32 *>(GetOutputSignalMemory(n));
                    uint32 e;
                    for (e=0; e<elements[n]; e++) {
                        signalOut[e] = static_cast<float32>(signalToGenerate[counter]);
                    }
                }
                else if (signalType == Float64Bit) {
                    float64 *signalOut = reinterpret_cast<float64 *>(GetOutputSignalMemory(n));
                    uint32 e;
                    for (e=0; e<elements[n]; e++) {
                        signalOut[e] = static_cast<float64>(signalToGenerate[counter]);
                    }
                }
            }

        }
        counter++;
        return true;
    }

    bool generateTrigger;
    bool useAbsoluteTime;
    MARTe::uint8 *triggerToGenerate;
    MARTe::uint32 *signalToGenerate;
    MARTe::uint32 *elements;
    MARTe::uint32 counter;
    MARTe::uint32 numberOfExecutes;
    MARTe::float32 period;
    hpn_timestamp_t absTimeStamp;
};
CLASS_REGISTER(DANSourceGAMTriggerTestHelper, "1.0")

/**
 * @brief Manual scheduler to test the correct interface between the DANSource and the GAMs
 */
class DANSourceSchedulerTestHelper: public MARTe::GAMSchedulerI {
public:

    CLASS_REGISTER_DECLARATION()

DANSourceSchedulerTestHelper    () : MARTe::GAMSchedulerI() {
        scheduledStates = NULL;
    }

    virtual MARTe::ErrorManagement::ErrorType StartNextStateExecution() {
        return MARTe::ErrorManagement::NoError;
    }

    virtual MARTe::ErrorManagement::ErrorType StopCurrentStateExecution() {
        return MARTe::ErrorManagement::NoError;
    }

    void ExecuteThreadCycle(MARTe::uint32 threadId) {
        using namespace MARTe;
        ExecuteSingleCycle(scheduledStates[RealTimeApplication::GetIndex()]->threads[threadId].executables,
                scheduledStates[RealTimeApplication::GetIndex()]->threads[threadId].numberOfExecutables);
    }

    virtual bool ConfigureScheduler() {

        bool ret = GAMSchedulerI::ConfigureScheduler();
        if (ret) {
            scheduledStates = GetSchedulableStates();
        }
        return ret;
    }

    virtual void CustomPrepareNextState() {
    }

private:

    MARTe::ScheduledState * const * scheduledStates;
};

CLASS_REGISTER(DANSourceSchedulerTestHelper, "1.0")

static bool TestIntegratedInApplication(const MARTe::char8 * const config, bool destroy) {
    using namespace MARTe;

    ConfigurationDatabase cdb;
    StreamString configStream = config;
    configStream.Seek(0);
    StreamString err;
    StandardParser parser(configStream, cdb, &err);

    bool ok = parser.Parse();
    if (!ok) {
        REPORT_ERROR_STATIC(ErrorManagement::FatalError, "%s", err.Buffer());
    }

    ObjectRegistryDatabase *god = ObjectRegistryDatabase::Instance();

    if (ok) {
        god->Purge();
        ok = god->Initialise(cdb);
    }
    ReferenceT<RealTimeApplication> application;
    if (ok) {
        application = god->Find("Test");
        ok = application.IsValid();
    }
    if (ok) {
        ok = application->ConfigureApplication();
    }

    if (destroy) {
        god->Purge();
    }
    return ok;
}

template<typename typeToCheck> static bool VerifyData(const hpn_timestamp_t hpnTimeStamp, const MARTe::uint64 periodNanos, MARTe::uint32 *signalToVerify, MARTe::uint32 toVerifyNumberOfElements) {
    using namespace MARTe;
    //To discover the type
    typeToCheck tDiscover;
    AnyType typeDiscover(tDiscover);

    //Wait for the file to be created...
    bool found = false;
    uint32 timeoutCounter = 0u;
    const uint32 maxTimeoutCounter = 30u;
    DirectoryScanner hd5FilesToTest;
    while ((!found) && (timeoutCounter < maxTimeoutCounter)) {
        StreamString filterName;
        filterName.Printf("*DANStreamTest_%s*", TypeDescriptor::GetTypeNameFromTypeDescriptor(typeDiscover.GetTypeDescriptor()));
        hd5FilesToTest.Scan("/tmp/data/", filterName.Buffer());
        found = (static_cast<Directory *>(hd5FilesToTest.ListPeek(0u)) != NULL);
        if (!found) {
            Sleep::Sec(0.1);
        }
        timeoutCounter++;
    }
    bool ok = found;
    DanStreamReaderCpp danStreamReader;
    if (ok) {
        found = false;
        timeoutCounter = 0;
        while ((ok) && (!found) && (timeoutCounter < maxTimeoutCounter)) {
            StreamString hd5FileName = static_cast<Directory *>(hd5FilesToTest.ListPeek(0u))->GetName();
            ok = (danStreamReader.openFile(hd5FileName.Buffer()) == 0);
            found = (danStreamReader.getCurSamples() == toVerifyNumberOfElements);
            if (!found) {
                danStreamReader.closeFile();
                Sleep::Sec(0.1);
            }
            timeoutCounter++;
        }
        if (ok) {
            ok = found;
        }
    }

    const char8 *channelNames[] = { "Signal1", "Signal2", "Signal3" };
    const uint32 numberOfChannels = 3;
    uint32 c;
    for (c = 0; (c < numberOfChannels) && (ok); c++) {
        DanDataHolder *pDataChannel = NULL;
        if (ok) {
            DataInterval interval = danStreamReader.getIntervalWhole();
            danStreamReader.setChannel(channelNames[c]);
            pDataChannel = danStreamReader.getRawValuesNative(&interval, -1);
            ok = (pDataChannel != NULL);
            if (ok) {
                ok = (interval.getTimeFrom() == (long) hpnTimeStamp);
            }
            if (ok) {
                ok = (interval.getTimeTo() == (long) (hpnTimeStamp + (toVerifyNumberOfElements - 1) * periodNanos));
                if (!ok) {
                    //Try again and check if this is because of DAN bug 9699
                    ok = (interval.getTimeTo() == (long) (hpnTimeStamp + (toVerifyNumberOfElements + c - 1) * periodNanos));
                }
            }
        }
        if (ok) {
            const typeToCheck *channelDataStored = NULL;
            if (typeDiscover.GetTypeDescriptor() == UnsignedInteger16Bit) {
                channelDataStored = (typeToCheck *) pDataChannel->asUInt16();
            }
            else if (typeDiscover.GetTypeDescriptor() == SignedInteger16Bit) {
                channelDataStored = (typeToCheck *) pDataChannel->asInt16();
            }
            else if (typeDiscover.GetTypeDescriptor() == UnsignedInteger32Bit) {
                channelDataStored = (typeToCheck *) pDataChannel->asUInt32();
            }
            else if (typeDiscover.GetTypeDescriptor() == SignedInteger32Bit) {
                channelDataStored = (typeToCheck *) pDataChannel->asInt32();
            }
            else if (typeDiscover.GetTypeDescriptor() == UnsignedInteger64Bit) {
                channelDataStored = (typeToCheck *) pDataChannel->asUInt64();
            }
            else if (typeDiscover.GetTypeDescriptor() == SignedInteger64Bit) {
                channelDataStored = (typeToCheck *) pDataChannel->asInt64();
            }
            else if (typeDiscover.GetTypeDescriptor() == Float32Bit) {
                channelDataStored = (typeToCheck *) pDataChannel->asFloat();
            }
            else if (typeDiscover.GetTypeDescriptor() == Float64Bit) {
                channelDataStored = (typeToCheck *) pDataChannel->asDouble();
            }
            uint32 k;
            //Remove the following line when DAN bug 9699 is solved.
            toVerifyNumberOfElements--;
            for (k = 0; (k < toVerifyNumberOfElements) && (ok); k++) {
                ok = (((uint32) channelDataStored[k]) == signalToVerify[k]);
                if (!ok) {
                    REPORT_ERROR_STATIC(ErrorManagement::FatalError, "[%d] %d != %d", k, (uint32) channelDataStored[k], signalToVerify[k]);
                }

            }

            delete pDataChannel;
        }
    }
    if (ok) {
        danStreamReader.closeFile();
    }
    return ok;
}

static bool TestIntegratedExecution(const MARTe::char8 * const config, MARTe::uint32 *signalToGenerate, MARTe::uint32 toGenerateNumberOfElements, MARTe::uint8 *triggerToGenerate, MARTe::uint32 *signalToVerify, MARTe::uint32 *timeToVerify,
                                    MARTe::uint32 toVerifyNumberOfElements, MARTe::uint32 numberOfBuffers, MARTe::uint32 numberOfPreTriggers, MARTe::uint32 numberOfPostTriggers, MARTe::float32 period, MARTe::uint32 sleepMSec = 10) {
    using namespace MARTe;
    //Delete any previous DAN test files
    DirectoryScanner hd5FilesToTest;
    hd5FilesToTest.Scan("/tmp/data/", "*DANStreamTest*");
    uint32 i = 0u;
    while (hd5FilesToTest.ListPeek(i)) {
        Directory *toDelete = static_cast<Directory *>(hd5FilesToTest.ListPeek(i));
        toDelete->Delete();
        i++;
    }

    ConfigurationDatabase cdb;
    StreamString configStream = config;
    configStream.Seek(0);
    StandardParser parser(configStream, cdb);

    bool ok = parser.Parse();

    cdb.MoveAbsolute("$Test.+Functions.+GAM1");
    cdb.Delete("Signal");
    Vector<uint32> signalV(signalToGenerate, toGenerateNumberOfElements);
    cdb.Write("Signal", signalV);
    cdb.Delete("Period");
    cdb.Write("Period", period);
    if (triggerToGenerate != NULL) {
        cdb.Delete("Trigger");
        Vector<uint8> triggerV(triggerToGenerate, toGenerateNumberOfElements);
        cdb.Write("Trigger", triggerV);
    }

    cdb.MoveAbsolute("$Test.+Data.+DANStreamTest");
    cdb.Delete("NumberOfBuffers");
    cdb.Write("NumberOfBuffers", numberOfBuffers);
    cdb.Delete("NumberOfPreTriggers");
    cdb.Write("NumberOfPreTriggers", numberOfPreTriggers);
    cdb.Delete("NumberOfPostTriggers");
    cdb.Write("NumberOfPostTriggers", numberOfPostTriggers);
    cdb.Delete("StoreOnTrigger");
    uint32 storeOnTrigger = (triggerToGenerate != NULL) ? 1 : 0;
    cdb.Write("StoreOnTrigger", storeOnTrigger);
    cdb.MoveRelative("Signals");
    uint32 nSignals = cdb.GetNumberOfChildren();
    uint32 n = 0;
    //Do not patch the trigger and the timing signal
    if (triggerToGenerate != NULL) {
        n = 2;
    }
    for (; n < nSignals; n++) {
        cdb.MoveRelative(cdb.GetChildName(n));
        cdb.Delete("Period");
        cdb.Write("Period", period);
        cdb.MoveToAncestor(1u);
    }

    cdb.MoveToRoot();
    ObjectRegistryDatabase *god = ObjectRegistryDatabase::Instance();

    if (ok) {
        god->Purge();
        ok = god->Initialise(cdb);
    }
    ReferenceT<RealTimeApplication> application;
    ReferenceT<DANSourceSchedulerTestHelper> scheduler;
    ReferenceT<DANSourceGAMTriggerTestHelper> gam;
    ObjectRegistryDatabase *godb = ObjectRegistryDatabase::Instance();

    if (ok) {
        application = god->Find("Test");
        ok = application.IsValid();
    }
    if (ok) {
        ok = application->ConfigureApplication();
    }
    if (ok) {
        scheduler = application->Find("Scheduler");
        ok = scheduler.IsValid();
    }
    if (ok) {
        gam = application->Find("Functions.GAM1");
        ok = gam.IsValid();
    }
    ReferenceT<DANSource> danSource;
    if (ok) {
        danSource = godb->Find("Test.Data.DANStreamTest");
        ok = danSource.IsValid();
    }
    if (ok) {
        ok = danSource->OpenStream();
    }
    if (ok) {
        ok = application->PrepareNextState("State1");
    }
    if (ok) {
        ok = application->StartNextStateExecution();
    }

    if (ok) {
        for (i = 0; (i < gam->numberOfExecutes); i++) {
            scheduler->ExecuteThreadCycle(0);
            Sleep::MSec(sleepMSec);
        }
    }
    if (ok) {
        ok = application->StopCurrentStateExecution();
    }
    if (ok) {
        ok = danSource->CloseStream();
    }
    godb->Purge();

    if (ok) {
        ok = VerifyData<uint16>(danSource->GetAbsoluteStartTime(), period * 1e9, signalToVerify, toVerifyNumberOfElements);
    }

    return ok;
}

//Standard configuration with no trigger source
static const MARTe::char8 * const config1 = ""
        "$Test = {"
        "    Class = RealTimeApplication"
        "    +Functions = {"
        "        Class = ReferenceContainer"
        "        +GAM1 = {"
        "            Class = DANSourceGAMTriggerTestHelper"
        "            Signal =  {0 1 2 3 4 5 6 7 8 9 8 7 6 5}"
        "            OutputSignals = {"
        "                SignalUInt16F_1 = {"
        "                    Type = uint16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt16F_2 = {"
        "                    Type = uint16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt32F_1 = {"
        "                    Type = uint32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt32F_2 = {"
        "                    Type = uint32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt64F_1 = {"
        "                    Type = uint64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt64F_2 = {"
        "                    Type = uint64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt16F_1 = {"
        "                    Type = int16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt16F_2 = {"
        "                    Type = int16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt32F_1 = {"
        "                    Type = int32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt32F_2 = {"
        "                    Type = int32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt64F_1 = {"
        "                    Type = int64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt64F_2 = {"
        "                    Type = int64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat32F_1 = {"
        "                    Type = float32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat32F_2 = {"
        "                    Type = float32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat64F_1 = {"
        "                    Type = float64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat64F_2 = {"
        "                    Type = float64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat64F_3 = {"
        "                    Type = float64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat32F_3 = {"
        "                    Type = float32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt64F_3 = {"
        "                    Type = int64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt32F_3 = {"
        "                    Type = int32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt16F_3 = {"
        "                    Type = int16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt64F_3 = {"
        "                    Type = uint64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt32F_3 = {"
        "                    Type = uint32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt16F_3 = {"
        "                    Type = uint16"
        "                    DataSource = DANStreamTest"
        "                }"
        "            }"
        "        }"
        "    }"
        "    +Data = {"
        "        Class = ReferenceContainer"
        "        DefaultDataSource = DDB1"
        "        +Timings = {"
        "            Class = TimingDataSource"
        "        }"
        "        +DANStreamTest = {"
        "            Class = DANSource"
        "            NumberOfBuffers = 10"
        "            CPUMask = 15"
        "            StackSize = 10000000"
        "            StoreOnTrigger = 0"
        "            DanBufferMultiplier = 4"
        "            Signals = {"
        "                SignalUInt16F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt16F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt32F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt32F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt64F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt64F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt16F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt16F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt32F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt32F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt64F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt64F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat32F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat32F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat64F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat64F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt16F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt32F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt64F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt16F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt32F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt64F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat32F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat64F_3 = {"
        "                    Period = 2"
        "                }"
        "            }"
        "        }"
        "    }"
        "    +States = {"
        "        Class = ReferenceContainer"
        "        +State1 = {"
        "            Class = RealTimeState"
        "            +Threads = {"
        "                Class = ReferenceContainer"
        "                +Thread1 = {"
        "                    Class = RealTimeThread"
        "                    Functions = {GAM1}"
        "                }"
        "            }"
        "        }"
        "    }"
        "    +Scheduler = {"
        "        Class = DANSourceSchedulerTestHelper"
        "        TimingDataSource = Timings"
        "    }"
        "}";

//Configuration with a trigger source and relative time
static const MARTe::char8 * const config2 = ""
        "$Test = {"
        "    Class = RealTimeApplication"
        "    +Functions = {"
        "        Class = ReferenceContainer"
        "        +GAM1 = {"
        "            Class = DANSourceGAMTriggerTestHelper"
        "            Signal =  {0 1 2 3 4 5 6 7 8 9 8 7 6 5}"
        "            OutputSignals = {"
        "                Trigger = {"
        "                    Type = uint8"
        "                    DataSource = DANStreamTest"
        "                }"
        "                Time = {"
        "                    Type = int32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt16F_1 = {"
        "                    Type = uint16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt16F_2 = {"
        "                    Type = uint16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt32F_1 = {"
        "                    Type = uint32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt32F_2 = {"
        "                    Type = uint32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt64F_1 = {"
        "                    Type = uint64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt64F_2 = {"
        "                    Type = uint64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt16F_1 = {"
        "                    Type = int16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt16F_2 = {"
        "                    Type = int16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt32F_1 = {"
        "                    Type = int32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt32F_2 = {"
        "                    Type = int32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt64F_1 = {"
        "                    Type = int64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt64F_2 = {"
        "                    Type = int64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat32F_1 = {"
        "                    Type = float32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat32F_2 = {"
        "                    Type = float32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat64F_1 = {"
        "                    Type = float64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat64F_2 = {"
        "                    Type = float64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat64F_3 = {"
        "                    Type = float64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalFloat32F_3 = {"
        "                    Type = float32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt64F_3 = {"
        "                    Type = int64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt32F_3 = {"
        "                    Type = int32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalInt16F_3 = {"
        "                    Type = int16"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt64F_3 = {"
        "                    Type = uint64"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt32F_3 = {"
        "                    Type = uint32"
        "                    DataSource = DANStreamTest"
        "                }"
        "                SignalUInt16F_3 = {"
        "                    Type = uint16"
        "                    DataSource = DANStreamTest"
        "                }"
        "            }"
        "        }"
        "    }"
        "    +Data = {"
        "        Class = ReferenceContainer"
        "        DefaultDataSource = DDB1"
        "        +Timings = {"
        "            Class = TimingDataSource"
        "        }"
        "        +DANStreamTest = {"
        "            Class = DANSource"
        "            NumberOfBuffers = 10"
        "            CPUMask = 15"
        "            StackSize = 10000000"
        "            StoreOnTrigger = 1"
        "            DanBufferMultiplier = 4"
        "            NumberOfPreTriggers = 2"
        "            NumberOfPostTriggers = 1"
        "            Signals = {"
        "                Trigger = {"
        "                    Type = uint8"
        "                }"
        "                Time = {"
        "                    Type = int32"
        "                    TimeSignal = 1"
        "                }"
        "                SignalUInt16F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt16F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt32F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt32F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt64F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt64F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt16F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt16F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt32F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt32F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt64F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt64F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat32F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat32F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat64F_1 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat64F_2 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt16F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt32F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalUInt64F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt16F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt32F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalInt64F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat32F_3 = {"
        "                    Period = 2"
        "                }"
        "                SignalFloat64F_3 = {"
        "                    Period = 2"
        "                }"
        "            }"
        "        }"
        "    }"
        "    +States = {"
        "        Class = ReferenceContainer"
        "        +State1 = {"
        "            Class = RealTimeState"
        "            +Threads = {"
        "                Class = ReferenceContainer"
        "                +Thread1 = {"
        "                    Class = RealTimeThread"
        "                    Functions = {GAM1}"
        "                }"
        "            }"
        "        }"
        "    }"
        "    +Scheduler = {"
        "        Class = DANSourceSchedulerTestHelper"
        "        TimingDataSource = Timings"
        "    }"
        "}";

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
bool DANSourceTest::TestConstructor() {
    using namespace MARTe;
    DANSource ds;
    bool ok = (ds.GetCPUMask() == 0xf);
    ok &= (ds.GetNumberOfBuffers() == 0u);
    ok &= (ds.GetNumberOfPostTriggers() == 0u);
    ok &= (ds.GetNumberOfPreTriggers() == 0u);
    ok &= (ds.GetStackSize() == 0u);
    ok &= (ds.IsAbsoluteTime() == false);
    ok &= (ds.IsStoreOnTrigger() == false);
    ok &= (ds.GetTimeSignalIdx() == -1);
    ok &= (DANSource::GetDANDataCore() == NULL);
    return ok;
}

bool DANSourceTest::TestAllocateMemory() {
    using namespace MARTe;
    DANSource ds;
    return ds.AllocateMemory();
}

bool DANSourceTest::TestGetNumberOfMemoryBuffers() {
    using namespace MARTe;
    DANSource ds;
    return (ds.GetNumberOfMemoryBuffers() == 1);
}

bool DANSourceTest::TestGetSignalMemoryBuffer() {
    using namespace MARTe;
    DANSource test;
    void *ptr = NULL;
    bool ok = !test.GetSignalMemoryBuffer(0, 0, ptr);
    if (ok) {
        ok = TestIntegratedInApplication(config1, true);
    }
    return ok;
}

bool DANSourceTest::TestGetBrokerName_InputSignals() {
    using namespace MARTe;
    DANSource test;
    ConfigurationDatabase cdb;
    bool ok = (StringHelper::Compare(test.GetBrokerName(cdb, InputSignals), "") == 0);

    return ok;
}

bool DANSourceTest::TestGetBrokerName_MemoryMapAsyncOutputBroker() {
    using namespace MARTe;
    DANSource test;
    ConfigurationDatabase cdb;
    cdb.Write("NumberOfBuffers", 10);
    cdb.Write("CPUMask", 15);
    cdb.Write("StackSize", 10000000);
    cdb.Write("StoreOnTrigger", 0);
    cdb.Write("NumberOfPreTriggers", 2);
    cdb.Write("NumberOfPostTriggers", 1);
    cdb.Write("DanBufferMultiplier", 4);
    cdb.CreateRelative("Signals");
    cdb.MoveToRoot();
    bool ok = test.Initialise(cdb);
    if (ok) {
        ok = (StringHelper::Compare(test.GetBrokerName(cdb, OutputSignals), "MemoryMapAsyncOutputBroker") == 0);
    }

    return ok;
}

bool DANSourceTest::TestGetBrokerName_MemoryMapAsyncTriggerOutputBroker() {
    using namespace MARTe;
    DANSource test;
    ConfigurationDatabase cdb;
    cdb.Write("NumberOfBuffers", 10);
    cdb.Write("CPUMask", 15);
    cdb.Write("StackSize", 10000000);
    cdb.Write("StoreOnTrigger", 1);
    cdb.Write("NumberOfPreTriggers", 2);
    cdb.Write("NumberOfPostTriggers", 1);
    cdb.Write("DanBufferMultiplier", 4);
    cdb.CreateRelative("Signals");
    cdb.MoveToRoot();
    bool ok = test.Initialise(cdb);
    if (ok) {
        ok = (StringHelper::Compare(test.GetBrokerName(cdb, OutputSignals), "MemoryMapAsyncTriggerOutputBroker") == 0);
    }

    return ok;
}

bool DANSourceTest::TestGetInputBrokers() {
    using namespace MARTe;
    DANSource test;
    ReferenceContainer rc;
    return !test.GetInputBrokers(rc, "", NULL);
}

bool DANSourceTest::TestGetOutputBrokers() {
    bool ok = TestIntegratedInApplication_NoTrigger();
    if (ok) {
        ok = TestIntegratedInApplication_Trigger();
    }
    return ok;
}

bool DANSourceTest::TestIntegratedInApplication_NoTrigger() {
    using namespace MARTe;
    uint32 signalToGenerate[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    uint32 timeToVerify[] = { 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22 };
    uint32 numberOfElements = sizeof(signalToGenerate) / sizeof(uint32);
    const uint32 numberOfBuffers = 16;
    const float32 period = 1e-3;
    return TestIntegratedExecution(config1, signalToGenerate, numberOfElements, NULL, signalToGenerate, timeToVerify, numberOfElements, numberOfBuffers, 0, 0, period);
}

bool DANSourceTest::TestIntegratedInApplication_Trigger() {
    using namespace MARTe;
    uint32 signalToGenerate[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
    uint32 signalToVerify[] = { 1, 2, 3, 4, 6, 7, 8, 9 };
    uint32 timeToVerify[] = { 0, 2, 4, 6, 10, 12, 14, 16 };
    uint8 triggerToGenerate[] = { 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 };
    uint32 numberOfElements = sizeof(signalToGenerate) / sizeof(uint32);
    uint32 numberOfElementsToVerify = sizeof(signalToVerify) / sizeof(uint32);
    const uint32 numberOfBuffers = 16;
    const uint32 numberOfPreTriggers = 2;
    const uint32 numberOfPostTriggers = 1;
    const float32 period = 1e-3;
    return TestIntegratedExecution(config2, signalToGenerate, numberOfElements, triggerToGenerate, signalToVerify, timeToVerify, numberOfElementsToVerify, numberOfBuffers, numberOfPreTriggers, numberOfPostTriggers, period);
}
