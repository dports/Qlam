//
// Created by darren on 04/04/2020.
//

#ifndef QLAM_SCANNERHEURISTICMATCH_H
#define QLAM_SCANNERHEURISTICMATCH_H

namespace Qlam {

    // Generic used when we don't recognise the specific heuristic virus name (i.e. we're lagging behind upstream changes)
    enum class ScannerHeuristicMatch {
        Generic = 0,
        BrokenExecutable,
        ExceedsMaximum,
        InvalidPartitionTableSize,

        PhishingEmailSpoofedDomain,
        PhishingSslMismatch,
        PhishingCloak,
        PhishingGeneric,

        OleGeneric,
        OleMacros,

        EncryptedArchive,
        EncryptedDoc,
        EncryptedGeneric,

        StructuredCreditCardNumber,
        StructuredSsnNormal,
        StructuredSsnStripped,
        StructuredGeneric,
    };
}

#endif //QLAM_SCANNERHEURISTICMATCH_H
