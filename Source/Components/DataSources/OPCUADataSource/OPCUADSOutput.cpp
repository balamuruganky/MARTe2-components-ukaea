/**
 * @file OPCUADSOutput.cpp
 * @brief Source file for class OPCUADSOutput
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
 * the class OPCUADSOutput (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "OPCUADSOutput.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
/*-e909 and -e9133 redefines bool. -e578 symbol ovveride in CLASS_REGISTER*/
/*lint -save -e909 -e9133 -e578*/
namespace MARTe {

OPCUADSOutput::OPCUADSOutput() :
        DataSourceI() {
    masterClient = NULL_PTR(OPCUAClientWrapper *);
    nOfSignals = 0u;
    numberOfNodes = 0u;
    paths = NULL_PTR(StreamString *);
    namespaceIndexes = NULL_PTR(uint16 *);
    tempPaths = NULL_PTR(StreamString *);
    tempNamespaceIndexes = NULL_PTR(uint16 *);
    serverAddress = "";
    nDimensions = NULL_PTR(uint8 *);
    nElements = NULL_PTR(uint32 *);
    types = NULL_PTR(TypeDescriptor *);
}

/*lint -e{1551} No exception thrown.*/
OPCUADSOutput::~OPCUADSOutput() {
    if (nDimensions != NULL_PTR(uint8 *)) {
        delete[] nDimensions;
    }
    if (nElements != NULL_PTR(uint32 *)) {
        delete[] nElements;
    }
    if (types != NULL_PTR(TypeDescriptor *)) {
        delete[] types;
    }
    if (paths != NULL_PTR(StreamString *)) {
        delete[] paths;
    }
    if (namespaceIndexes != NULL_PTR(uint16 *)) {
        delete[] namespaceIndexes;
    }
    if (tempPaths != NULL_PTR(StreamString *)) {
        delete[] tempPaths;
    }
    if (tempNamespaceIndexes != NULL_PTR(uint16 *)) {
        delete[] tempNamespaceIndexes;
    }
    if (masterClient != NULL_PTR(OPCUAClientWrapper *)) {
        delete masterClient;
    }
}

bool OPCUADSOutput::Initialise(StructuredDataI & data) {
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        ok = data.Read("Address", serverAddress);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read the Address attribute");
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
            nOfSignals = (signalsDatabase.GetNumberOfChildren() - 1u);
            tempPaths = new StreamString[nOfSignals];
            tempNamespaceIndexes = new uint16[nOfSignals];
            for (uint32 i = 0u; (i < nOfSignals) && (ok); i++) {
                ok = signalsDatabase.MoveRelative(signalsDatabase.GetChildName(i));
                if (ok) {
                    ok = signalsDatabase.Read("Path", tempPaths[i]);
                    if (!ok) {
                        uint32 k = i;
                        REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read the Path attribute from signal %d", k);
                    }
                }
                if (ok) {
                    ok = signalsDatabase.Read("NamespaceIndex", tempNamespaceIndexes[i]);
                    if (!ok) {
                        uint32 k = i;
                        REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read the NamespaceIndex attribute from signal %d", k);
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

bool OPCUADSOutput::SetConfiguredDatabase(StructuredDataI & data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    numberOfNodes = GetNumberOfSignals();
    nDimensions = new uint8[numberOfNodes];
    nElements = new uint32[numberOfNodes];
    types = new TypeDescriptor[numberOfNodes];
    StreamString signalName;
    for (uint32 k = 0u; k < numberOfNodes; k++) {
        ok = GetSignalNumberOfDimensions(k, nDimensions[k]);
        if ((nDimensions[k] > 1u) && ok) {
            ok = GetSignalName(k, signalName);
            if (ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Signal %s has Number Of Dimensions = %d. Multidimensional arrays not supported yet.",
                             signalName.Buffer(), nDimensions[k]);
                ok = false;
            }
        }
        if (ok) {
            ok = GetSignalNumberOfElements(k, nElements[k]);
        }
        if (ok) {
            types[k] = GetSignalType(k);
        }
    }
    if (ok) {
        paths = new StreamString[numberOfNodes];
        namespaceIndexes = new uint16[numberOfNodes];
        StreamString sigName;
        StreamString pathToken;
        StreamString sigToken;
        char8 ignore;
        for (uint32 i = 0u; i < numberOfNodes; i++) {
            sigName = "";
            ok = GetSignalName(i, sigName);
            /* Getting the first name from the signal path */
            if (ok) {
                ok = sigName.Seek(0LLU);
            }
            if (ok) {
                ok = sigName.GetToken(sigToken, ".", ignore);
            }
            if (ok) {
                for (uint32 j = 0u; j < nOfSignals; j++) {
                    if (tempPaths != NULL_PTR(StreamString *)) {
                        /* This cycle will save the last token found */
                        ok = tempPaths[j].Seek(0LLU);
                        if (ok) {
                            do {
                                pathToken = "";

                                ok = tempPaths[j].GetToken(pathToken, ".", ignore);
                                if ((paths != NULL_PTR(StreamString *)) && ok) {
                                    if ((namespaceIndexes != NULL_PTR(uint16 *)) && (tempNamespaceIndexes != NULL_PTR(uint16 *))) {
                                        if (pathToken == sigToken) {
                                            paths[i] = tempPaths[j];
                                            namespaceIndexes[i] = tempNamespaceIndexes[j];
                                            ok = false; /* Exit from the cycle */
                                        }
                                    }
                                }
                            }
                            while (ok);
                        }
                    }
                }
            }

            /* Then we add to the path the remaining node names */
            StreamString dotToken = ".";
            do {
                sigToken = "";
                ok = sigName.GetToken(sigToken, ".", ignore);
                if ((paths != NULL_PTR(StreamString *)) && ok) {
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
        masterClient = new OPCUAClientWrapper();
        masterClient->SetOperationMode("Write");
        masterClient->SetServerAddress(serverAddress);
        ok = masterClient->Connect();
        if (ok) {
            REPORT_ERROR(ErrorManagement::Information, "The connection with the OPCUA Server has been established successfully!");
            ok = masterClient->SetTargetNodes(namespaceIndexes, paths, numberOfNodes);
        }
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Cannot find one or more signals in the Server.");
        }
    }
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "Error during configuration.");
    }
    return ok;
}

bool OPCUADSOutput::AllocateMemory() {
    return true;
}

/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: The signalAddress is independent of the bufferIdx.*/
bool OPCUADSOutput::GetSignalMemoryBuffer(const uint32 signalIdx,
                                          const uint32 bufferIdx,
                                          void *&signalAddress) {
    StreamString opcDisplayName;
    bool ok = GetSignalName(signalIdx, opcDisplayName);
    /* Debug only */
    //REPORT_ERROR(ErrorManagement::Information, "Searching for signal [%s]", opcDisplayName.Buffer());
    if ((types != NULL_PTR(TypeDescriptor *)) && (nElements != NULL_PTR(uint32 *)) && (masterClient != NULL_PTR(OPCUAClientWrapper *))
            && (nDimensions != NULL_PTR(uint8 *))) {
        if (ok) {
#if 0
            if ((types[signalIdx].type == CArray) || (types[signalIdx].type == BT_CCString) || (types[signalIdx].type == PCString)
                    || (types[signalIdx].type == SString)) {
                REPORT_ERROR(ErrorManagement::ParametersError, "String are not supported yet.");
                ok = false;
            }
#endif
            ok = masterClient->GetSignalMemory(signalAddress, signalIdx, types[signalIdx], nElements[signalIdx], nDimensions[signalIdx]);
        }
        if (ok) {
            ok = (signalAddress != NULL_PTR(void *));
        }
        if (ok) {
            types[signalIdx] = TypeDescriptor();
            nElements[signalIdx] = 0u;
            nDimensions[signalIdx] = 0u;
        }
    }
    return ok;
}

/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: The brokerName only depends on the direction */
const char8 * OPCUADSOutput::GetBrokerName(StructuredDataI &data,
                                           const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == OutputSignals) {
        brokerName = "MemoryMapSynchronisedOutputBroker";
    }
    return brokerName;
}

/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool OPCUADSOutput::PrepareNextState(const char8 * const currentStateName,
                                     const char8 * const nextStateName) {
    return true;
}

bool OPCUADSOutput::Synchronise() {
    bool ok = true;
    if (masterClient != NULL_PTR(OPCUAClientWrapper *)) {
        ok = masterClient->Write(numberOfNodes);
    }
    return ok;
}

const char8 * OPCUADSOutput::GetServerAddress() {
    return serverAddress.Buffer();
}

OPCUAClientWrapper * OPCUADSOutput::GetClient() {
    return masterClient;
}

CLASS_REGISTER(OPCUADSOutput, "1.0")

}
/*lint -restore*/
