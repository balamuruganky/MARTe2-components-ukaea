/**
 * @file EPICSPVAChannelWrapperTest.h
 * @brief Header file for class EPICSPVAChannelWrapperTest
 * @date 23/10/2018
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

 * @details This header file contains the declaration of the class EPICSPVAChannelWrapperTest
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef EPICSPVA_EPICSPVACHANNELWRAPPERTEST_H_
#define EPICSPVA_EPICSPVACHANNELWRAPPERTEST_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

/**
 * @brief Tests the EPICSPVAChannelWrapper public methods.
 */
class EPICSPVAChannelWrapperTest {
public:
    /**
     * @brief Tests the constructor.
     */
    bool TestConstructor();

    /**
     * @brief Tests the Setup method.
     */
    bool TestSetup();

    /**
     * @brief Tests that the Setup method fails for signals with dimensions > 1.
     */
    bool TestSetup_False_Dimensions();

    /**
     * @brief Tests that the Setup method fails with an invalid leaf.
     */
    bool TestSetup_False_InvalidLeaf();

    /**
     * @brief Tests the GetSignalMemory method.
     */
    bool TestGetSignalMemory();

    /**
     * @brief Tests the Put method.
     */
    bool TestPut();

    /**
     * @brief Tests the Monitor method.
     */
    bool TestMonitor();

    /**
     * @brief Test the GetChannelName method.
     */
    bool TestGetChannelName();

    /**
     * @brief Test the GetChannelUnaliasedName method.
     */
    bool TestGetChannelUnaliasedName();
};

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* EPICSPVA_EPICSPVACHANNELWRAPPERTEST_H_ */