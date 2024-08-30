// © 2016 and later: Unicode, Inc. and others.
// License & terms of use: http://www.unicode.org/copyright.html
//
//  rbbitblb.h
//

/*
**********************************************************************
*   Copyright (c) 2002-2016, International Business Machines
*   Corporation and others.  All Rights Reserved.
**********************************************************************
*/

#ifndef RBBITBLB_H
#define RBBITBLB_H

#include "unicode/utypes.h"

#if !UCONFIG_NO_BREAK_ITERATION

#include "unicode/uobject.h"
#include "unicode/rbbi.h"
#include "rbbidata.h"
#include "rbbirb.h"
#include "rbbinode.h"


U_NAMESPACE_BEGIN

class RBBIRuleScanner;
class RBBIRuleBuilder;
class UVector32;

//
//  class RBBITableBuilder is part of the RBBI rule compiler.
//                         It builds the state transition table used by the RBBI runtime
//                         from the expression syntax tree generated by the rule scanner.
//
//                         This class is part of the RBBI implementation only.
//                         There is no user-visible public API here.
//

class RBBITableBuilder : public UMemory {
public:
    RBBITableBuilder(RBBIRuleBuilder *rb, RBBINode **rootNode, UErrorCode &status);
    ~RBBITableBuilder();

    void     buildForwardTable();

    /** Return the runtime size in bytes of the built state table.  */
    int32_t  getTableSize() const;

    /** Fill in the runtime state table. Sufficient memory must exist at the specified location.
     */
    void     exportTable(void *where);

    /** Use 8 bits to encode the forward table */
    bool     use8BitsForTable() const;

    /**
     *  Find duplicate (redundant) character classes. Begin looking with categories.first.
     *  Duplicate, if found are returned in the categories parameter.
     *  This is an iterator-like function, used to identify character classes
     *  (state table columns) that can be eliminated.
     *  @param categories in/out parameter, specifies where to start looking for duplicates,
     *                and returns the first pair of duplicates found, if any.
     *  @return true if duplicate char classes were found, false otherwise.
     */
    bool     findDuplCharClassFrom(IntPair *categories);

    /** Remove a column from the state table. Used when two character categories
     *  have been found equivalent, and merged together, to eliminate the unneeded table column.
     */
    void     removeColumn(int32_t column);

    /**
     * Check for, and remove duplicate states (table rows).
     * @return the number of states removed.
     */
    int32_t  removeDuplicateStates();

    /** Build the safe reverse table from the already-constructed forward table. */
    void     buildSafeReverseTable(UErrorCode &status);

    /** Return the runtime size in bytes of the built safe reverse state table. */
    int32_t  getSafeTableSize() const;

    /** Fill in the runtime safe state table. Sufficient memory must exist at the specified location.
     */
    void     exportSafeTable(void *where);

    /** Use 8 bits to encode the safe reverse table */
    bool     use8BitsForSafeTable() const;

private:
    void     calcNullable(RBBINode *n);
    void     calcFirstPos(RBBINode *n);
    void     calcLastPos(RBBINode  *n);
    void     calcFollowPos(RBBINode *n);
    void     calcChainedFollowPos(RBBINode *n, RBBINode *endMarkNode);
    void     bofFixup();
    void     buildStateTable();
    void     mapLookAheadRules();
    void     flagAcceptingStates();
    void     flagLookAheadStates();
    void     flagTaggedStates();
    void     mergeRuleStatusVals();

    /**
     * Merge redundant state table columns, eliminating character classes with identical behavior.
     * Done after the state tables are generated, just before converting to their run-time format.
     */
    int32_t  mergeColumns();

    void     addRuleRootNodes(UVector *dest, RBBINode *node);

    /**
     *  Find duplicate (redundant) states, beginning at the specified pair,
     *  within this state table. This is an iterator-like function, used to
     *  identify states (state table rows) that can be eliminated.
     *  @param states in/out parameter, specifies where to start looking for duplicates,
     *                and returns the first pair of duplicates found, if any.
     *  @return true if duplicate states were found, false otherwise.
     */
    bool findDuplicateState(IntPair *states);

    /** Remove a duplicate state.
     * @param duplStates The duplicate states. The first is kept, the second is removed.
     *                   All references to the second in the state table are retargeted
     *                   to the first.
     */
    void removeState(IntPair duplStates);

    /** Find the next duplicate state in the safe reverse table. An iterator function.
     *  @param states in/out parameter, specifies where to start looking for duplicates,
     *                and returns the first pair of duplicates found, if any.
     *  @return true if a duplicate pair of states was found.
     */
    bool findDuplicateSafeState(IntPair *states);

    /** Remove a duplicate state from the safe table.
     * @param duplStates The duplicate states. The first is kept, the second is removed.
     *                   All references to the second in the state table are retargeted
     *                   to the first.
     */
    void removeSafeState(IntPair duplStates);

    // Set functions for UVector.
    //   TODO:  make a USet subclass of UVector

    void     setAdd(UVector *dest, UVector *source);
    UBool    setEquals(UVector *a, UVector *b);

    void     sortedAdd(UVector **dest, int32_t val);

public:
#ifdef RBBI_DEBUG
    void     printSet(UVector *s);
    void     printPosSets(RBBINode *n /* = nullptr */);
    void     printStates();
    void     printRuleStatusTable();
    void     printReverseTable();
#else
    #define  printSet(s)
    #define  printPosSets(n)
    #define  printStates()
    #define  printRuleStatusTable()
    #define  printReverseTable()
#endif

private:
    RBBIRuleBuilder  *fRB;
    RBBINode         *&fTree;              // The root node of the parse tree to build a
                                           //   table for.
    UErrorCode       *fStatus;

    /** State Descriptors, UVector<RBBIStateDescriptor> */
    UVector          *fDStates;            //  D states (Aho's terminology)
                                           //  Index is state number
                                           //  Contents are RBBIStateDescriptor pointers.

    /** Synthesized safe table, UVector of UnicodeString, one string per table row.   */
    UVector          *fSafeTable;

    /** Map from rule number (fVal in look ahead nodes) to sequential lookahead index. */
    UVector32        *fLookAheadRuleMap = nullptr;

    /* Counter used when assigning lookahead rule numbers.
     * Contains the last look-ahead number already in use.
     * The first look-ahead number is 2; Number 1 (ACCEPTING_UNCONDITIONAL) is reserved
     * for non-lookahead accepting states. See the declarations of RBBIStateTableRowT.   */
    int32_t          fLASlotsInUse = ACCEPTING_UNCONDITIONAL;


    RBBITableBuilder(const RBBITableBuilder &other) = delete; // forbid copying of this class
    RBBITableBuilder &operator=(const RBBITableBuilder &other) = delete; // forbid copying of this class
};

//
//  RBBIStateDescriptor - The DFA is constructed as a set of these descriptors,
//                        one for each state.
class RBBIStateDescriptor : public UMemory {
public:
    UBool            fMarked;
    uint32_t         fAccepting;
    uint32_t         fLookAhead;
    UVector          *fTagVals;
    int32_t          fTagsIdx;
    UVector          *fPositions;          // Set of parse tree positions associated
                                           //   with this state.  Unordered (it's a set).
                                           //   UVector contents are RBBINode *

    UVector32        *fDtran;              // Transitions out of this state.
                                           //   indexed by input character
                                           //   contents is int index of dest state
                                           //   in RBBITableBuilder.fDStates

    RBBIStateDescriptor(int maxInputSymbol,  UErrorCode *fStatus);
    ~RBBIStateDescriptor();

private:
    RBBIStateDescriptor(const RBBIStateDescriptor &other) = delete; // forbid copying of this class
    RBBIStateDescriptor &operator=(const RBBIStateDescriptor &other) = delete; // forbid copying of this class
};



U_NAMESPACE_END

#endif /* #if !UCONFIG_NO_BREAK_ITERATION */

#endif
