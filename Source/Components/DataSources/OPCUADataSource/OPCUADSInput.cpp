/**
 * @file OPCUADSInput.cpp
 * @brief Source file for class OPCUADSInput
 * @date 12/03/2019
 * @author Luca Porzio
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
 * the class OPCUADSInput (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "OPCUADSInput.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

OPCUADSInput::OPCUADSInput() :
        DataSourceI(),
        EmbeddedServiceMethodBinderI(),
        executor(*this) {
    masterClient = NULL_PTR(OPCUAClientWrapper *);
    numberOfSignals = 0u;
    numberOfNodes = 0u;
    paths = NULL_PTR(StreamString *);
    namespaceIndexes = NULL_PTR(uint32 *);
    tempPaths = NULL_PTR(StreamString *);
    tempNamespaceIndexes = NULL_PTR(uint32 *);
    serverAddress = "";
    samplingTime = 0;
    nElements = NULL_PTR(uint32 *);
    types = NULL_PTR(TypeDescriptor *);
}

OPCUADSInput::~OPCUADSInput() {
    if (!executor.Stop()) {
        REPORT_ERROR(ErrorManagement::FatalError, "Could not stop SingleThreadService.");
    }
    if (nElements != NULL_PTR(uint32 *)) {
        delete[] nElements;
    }
    if (types != NULL_PTR(TypeDescriptor *)) {
        delete[] types;
    }
    if (namespaceIndexes != NULL_PTR(uint32 *)) {
        delete[] namespaceIndexes;
    }
    if (tempPaths != NULL_PTR(StreamString *)) {
        delete[] tempPaths;
    }
    if (tempNamespaceIndexes != NULL_PTR(uint32 *)) {
        delete[] tempNamespaceIndexes;
    }
    if(masterClient != NULL_PTR(OPCUAClientWrapper *)){
        delete masterClient;
    }

}

bool OPCUADSInput::Initialise(StructuredDataI & data) {
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        if (ok) {
            ok = data.Read("Address", serverAddress);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read the Address attribute");
            }
        }
        if (ok) {
            ok = data.Read("Synchronise", sync);
            if (!ok) {
                sync = "no";
                REPORT_ERROR(ErrorManagement::ParametersError, "Synchronise option is not enabled. Using SingleThreadService.");
                ok = true;
            }
        }
        if (ok) {
            ok = data.Read("SamplingTime", samplingTime);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::Information, "Sampling Time not set. Using default value: 250ms");
                samplingTime = 250;
                ok = true;
            }
        }
        if (ok) {
            ok = data.MoveRelative("Signals");
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Could not move to the Signals section");
            }
        }
        if (ok) {
            ok = signalsDatabase.MoveRelative("Signals");
        }
        /* Do not allow to add signals in run-time */
        if (ok) {
            ok = signalsDatabase.Write("Locked", 1u);
            numberOfSignals = (signalsDatabase.GetNumberOfChildren() - 1u);
            tempPaths = new StreamString[numberOfSignals];
            tempNamespaceIndexes = new uint32[numberOfSignals];
            for (uint32 i = 0u; (i < numberOfSignals) && (ok); i++) {
                ok = signalsDatabase.MoveRelative(signalsDatabase.GetChildName(i));
                if (ok) {
                    ok = signalsDatabase.Read("Path", tempPaths[i]);
                    if (!ok) {
                        REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read the Path attribute from signal %d", i);
                    }
                }
                if (ok) {
                    ok = signalsDatabase.Read("NamespaceIndex", tempNamespaceIndexes[i]);
                    if (!ok) {
                        REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read the NamespaceIndex attribute from signal %d", i);
                    }
                }
                if (ok) {
                    ok = signalsDatabase.MoveToAncestor(1u);
                }
            }
        }
        if (ok) {
            ok = signalsDatabase.MoveToAncestor(1u);
        }
        if (ok) {
            ok = data.MoveToAncestor(1u);
        }
    }
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "Error during Initialise!");
    }
    return ok;
}

bool OPCUADSInput::SetConfiguredDatabase(StructuredDataI & data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    numberOfNodes = GetNumberOfSignals();
    uint8 nDimensions = 0u;
    nElements = new uint32[numberOfNodes];
    types = new TypeDescriptor[numberOfNodes];
    StreamString signalName;
    /* Check for signals with number of dimensions > 1. Multidimensional arrays not supported yet */
    for (uint32 k = 0u; k < numberOfNodes; k++) {
        ok = GetSignalNumberOfDimensions(k, nDimensions);
        if (nDimensions > 1 && ok) {
            GetSignalName(k, signalName);
            REPORT_ERROR(ErrorManagement::ParametersError, "Signal %s has Number Of Dimensions = %d. Multidimensional arrays not supported yet.",
                         signalName.Buffer(), nDimensions);
            ok = false;
        }
        if (ok) {
            ok = GetSignalNumberOfElements(k, nElements[k]);
        }
        if (ok) {
            types[k] = GetSignalType(k);
        }
    }
    /* Setting path and namespace for each signal */
    if (ok) {
        paths = new StreamString[numberOfNodes];
        namespaceIndexes = new uint32[numberOfNodes];
        StreamString sigName, pathToken, sigToken;
        char8 ignore;
        for (uint32 i = 0u; i < numberOfNodes; i++) {
            sigName = "";
            GetSignalName(i, sigName);
            /* Getting the first name from the signal path */
            sigName.Seek(0LLU);
            ok = sigName.GetToken(sigToken, ".", ignore);

            for (uint32 j = 0u; j < numberOfSignals; j++) {
                /* This cycle will save the last token found */
                tempPaths[j].Seek(0LLU);
                do {
                    pathToken = "";
                    ok = tempPaths[j].GetToken(pathToken, ".", ignore);
                    if (pathToken == sigToken) {
                        paths[i] = tempPaths[j];
                        namespaceIndexes[i] = tempNamespaceIndexes[j];
                        ok = false; /* Exit from the cycle */
                    }
                }
                while (ok);
            }

            /* Then we add to the path the remaining node names */
            StreamString dotToken = ".";
            do {
                sigToken = "";
                ok = sigName.GetToken(sigToken, ".", ignore);
                if (ok) {
                    paths[i] += dotToken;
                    paths[i] += sigToken;
                }
            }
            while (ok);
            ok = true;
        }
    }
    if (ok) {
        /* Setting up the master Client who will perform the operations */
        char* s = new char[strlen(reinterpret_cast<const char*>(serverAddress.Buffer()))];
        strcpy(s, reinterpret_cast<const char*>(serverAddress.Buffer()));
        masterClient = new OPCUAClientWrapper("Read");
        masterClient->SetServerAddress(s);
        masterClient->SetSamplingTime(samplingTime);
        ok = masterClient->Connect();
        if (ok) {
            REPORT_ERROR(ErrorManagement::Information, "The connection with the OPCUA Server has been established successfully!");
        }
        ok = masterClient->SetTargetNodes(namespaceIndexes, paths, numberOfNodes);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Cannot find one or more signals in the Server.");
        }
    }
    if (ok && sync == "no") {
        ok = (executor.Start() == ErrorManagement::NoError);
    }
    return ok;
}

bool OPCUADSInput::AllocateMemory() {
    return true;
}

bool OPCUADSInput::GetSignalMemoryBuffer(const uint32 signalIdx,
                                         const uint32 bufferIdx,
                                         void *&signalAddress) {
    StreamString opcDisplayName;
    bool ok = GetSignalName(signalIdx, opcDisplayName);
    /* Debug info */
    //REPORT_ERROR(ErrorManagement::Information, "Searching for signal [%s]", opcDisplayName.Buffer());
    if (ok) {
        if ((types[signalIdx].type == CArray) || (types[signalIdx].type == BT_CCString) || (types[signalIdx].type == PCString)
                || (types[signalIdx].type == SString)) {
            //uint32 numberOfChars = 0u;
            //If char8 the number of elements will be 1. Correct this!
            //GetSignalNumberOfElements(signalIdx, numberOfChars);
            //clients[signalIdx].SetMaxNumberOfChar(numberOfChars);
        }
        if (ok) {
            ok = masterClient->GetSignalMemory(signalAddress, signalIdx, types[signalIdx], nElements[signalIdx], 0u);
        }
    }
    if (ok) {
        ok = (signalAddress != NULL_PTR(void *));
    }
    if (ok) {

    }
    return ok;
}

const char8 * OPCUADSInput::GetBrokerName(StructuredDataI &data,
                                          const SignalDirection direction) {
    const char8* brokerName = "";
    if (sync == "no") {
        if (direction == InputSignals) {
            brokerName = "MemoryMapInputBroker";
        }
    }
    else if (sync == "yes") {
        if (direction == InputSignals) {
            brokerName = "MemoryMapSynchronisedInputBroker";
        }
    }
    return brokerName;
}

bool OPCUADSInput::PrepareNextState(const char8 * const currentStateName,
                                    const char8 * const nextStateName) {
    return true;
}

ErrorManagement::ErrorType OPCUADSInput::Execute(ExecutionInfo & info) {
    ErrorManagement::ErrorType err = ErrorManagement::NoError;
    if (info.GetStage() != ExecutionInfo::BadTerminationStage) {
        //err.communicationError = !masterClient->Monitor();
        err.communicationError = !masterClient->Read(numberOfNodes, types, nElements);
    }
    return err;
}

bool OPCUADSInput::Synchronise() {
    bool ok = true;
    if (sync == "yes") {
        masterClient->Read(numberOfNodes, types, nElements);
    }
    return ok;
}

const char8 * OPCUADSInput::GetServerAddress() {
    return serverAddress.Buffer();
}

OPCUAClientWrapper * OPCUADSInput::GetClient() {
    return masterClient;
}

CLASS_REGISTER(OPCUADSInput, "1.0");

}

