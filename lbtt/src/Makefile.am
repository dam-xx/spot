bin_PROGRAMS = lbtt lbtt-translate
lbtt_SOURCES = \
  BitArray.cc \
  BitArray.h \
  Bitset.h \
  BuchiAutomaton.cc \
  BuchiAutomaton.h \
  Config-lex.ll \
  Config-parse.yy \
  Configuration.cc \
  Configuration.h \
  DispUtil.cc \
  DispUtil.h \
  EdgeContainer.h \
  Exception.h \
  FormulaRandomizer.cc \
  FormulaRandomizer.h \
  FormulaWriter.h \
  LtlFormula.cc \
  LtlFormula.h \
  main.cc \
  ObstackAlloc.h \
  PathEvaluator.cc \
  PathEvaluator.h \
  PathIterator.cc \
  PathIterator.h \
  ProductAutomaton.cc \
  ProductAutomaton.h \
  Random.h \
  SccIterator.h \
  SharedTestData.h \
  StatDisplay.cc \
  StatDisplay.h \
  StateSpace.cc \
  StateSpace.h \
  StateSpaceRandomizer.cc \
  StateSpaceRandomizer.h \
  StringUtil.cc \
  StringUtil.h \
  TestOperations.cc \
  TestOperations.h \
  TestRoundInfo.h \
  TestStatistics.cc \
  TestStatistics.h \
  UserCommandReader.cc \
  UserCommandReader.h \
  UserCommands.cc \
  UserCommands.h

EXTRA_lbtt_SOURCES = gnu-getopt.h Config-parse.h
lbtt_LDADD = $(LIBOBJS) $(READLINELIBS)

lbtt_translate_SOURCES = \
  ObstackAlloc.h \
  BitArray.h \
  BitArray.cc \
  Exception.h \
  ExternalTranslator.h \
  ExternalTranslator.cc \
  FormulaWriter.h \
  LbtWrapper.h \
  LtlFormula.h \
  LtlFormula.cc \
  NeverClaim-parse.yy \
  NeverClaim-lex.ll \
  NeverClaimAutomaton.h \
  NeverClaimAutomaton.cc \
  SpinWrapper.h \
  SpinWrapper.cc \
  SpotWrapper.h \
  SpotWrapper.cc \
  StringUtil.h \
  StringUtil.cc \
  translate.h \
  translate.cc \
  TranslatorInterface.h

EXTRA_lbtt_translate_SOURCES = gnu-getopt.h NeverClaim-parse.h
lbtt_translate_LDADD = $(LIBOBJS)
