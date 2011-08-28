MPVERSION=8
STUDENTSRC=mp$(MPVERSION)
OCAMLC=ocamlc
OCAMLLEX=ocamllex
OCAMLYACC=ocamlyacc
GMAKE=make
RM=rm
CP=cp
LN=ln
MV=mv
TAR=tar
GZIP=gzip
MKDIR=mkdir

GRADER_NAME=grader

LIBRARY_GRADER=lib/grader.cma
MODULE_STUDENT=student
MODULE_SOLUTION=solution
MODULE_RUBRIC=rubric
MODULE_COMMON=mp8common
INTERACTIVE_PARSER=micromlIntPar

#######################################################################
# DISTFILES define what goes into mpNtest.tgz distributions
#######################################################################

all: $(GRADER_NAME) $(INTERACTIVE_PARSER) $(INTERACTIVE_PARSER)Sol

DISTFILES_SOURCE=pre-rubric.c tests Makefile $(INTERACTIVE_PARSER).ml micromllex.mll $(MODULE_COMMON).ml
DISTFILES_OBJECT=$(MODULE_COMMON).cmo $(MODULE_COMMON).cmi $(MODULE_SOLUTION).cmo $(MODULE_SOLUTION).cmi

IMPLEMENTATIONS= $(MODULE_COMMON).cmo $(MODULE_STUDENT).cmo $(MODULE_SOLUTION).cmo micromllex.cmo micromllex2.cmo

DISTFILES_OTHER=README mp8-skeleton.mly
DISTFILES=$(DISTFILES_SOURCE) $(DISTFILES_OBJECT) $(DISTFILES_OTHER)

OBJECTS=$(IMPLEMENTATIONS) $(MODULE_RUBRIC).cmo

STUDENT_CLEAN=$(MODULE_STUDENT).cm? $(MODULE_STUDENT).mli $(MODULE_STUDENT).ml $(MODULE_STUDENT).mly $(MODULE_STUDENT).output $(MODULE_RUBRIC).cm? util.o $(GRADER_NAME) \
        $(INTERACTIVE_PARSER) $(INTERACTIVE_PARSER)Sol $(INTERACTIVE_PARSER)*.cm? $(INTERACTIVE_PARSER)2*

$(GRADER_NAME): $(LIBRARY_GRADER) $(OBJECTS)
	$(OCAMLC) -o $(GRADER_NAME) $(LIBRARY_GRADER) $(OBJECTS) 

$(INTERACTIVE_PARSER): $(GRADER_NAME) micromllex.cmo $(INTERACTIVE_PARSER).ml
	$(OCAMLC) -c micromllex.cmo $(INTERACTIVE_PARSER).ml
	$(OCAMLC) -o $(INTERACTIVE_PARSER) $(IMPLEMENTATIONS) $(INTERACTIVE_PARSER).cmo 

$(INTERACTIVE_PARSER)Sol: $(GRADER_NAME) micromllex2.cmo $(INTERACTIVE_PARSER).ml
	sed 's/Student/Solution/g' $(INTERACTIVE_PARSER).ml | sed 's/Micromllex/Micromllex2/g' > $(INTERACTIVE_PARSER)2.ml
	$(OCAMLC) -c micromllex2.cmo $(INTERACTIVE_PARSER)2.ml
	$(OCAMLC) -o $(INTERACTIVE_PARSER)Sol $(IMPLEMENTATIONS) $(INTERACTIVE_PARSER)2.cmo

$(LIBRARY_GRADER):
	$(GMAKE) -C lib
	$(LN) -s lib/util.o .

$(MODULE_STUDENT).cmo: $(STUDENTSRC).mly 
	$(CP) $(STUDENTSRC).mly $(MODULE_STUDENT).mly
	$(OCAMLYACC) -v $(MODULE_STUDENT).mly
	$(OCAMLC) -c $(MODULE_STUDENT).mli
	$(OCAMLC) -c $(MODULE_STUDENT).ml

########################################################################
# if solution.mly exists, compile it.  otherwise assume solution.cm{o,i}
# exist.
########################################################################
ifeq "$(wildcard $(MODULE_SOLUTION).mly)" "$(MODULE_SOLUTION).mly"
$(MODULE_COMMON).cmo: $(MODULE_COMMON).ml
	$(OCAMLC) -c $(MODULE_COMMON).ml

$(MODULE_SOLUTION).cmo: $(MODULE_SOLUTION).mly
	$(OCAMLYACC) $(MODULE_SOLUTION).mly
	$(OCAMLC) -c $(MODULE_SOLUTION).mli
	$(OCAMLC) -c $(MODULE_SOLUTION).ml
endif

micromllex2.cmo: micromllex.ml
	sed 's/open Student/open Solution/g' micromllex.ml > micromllex2.ml
#	$(OCAMLLEX) micromllex2.mll
	$(OCAMLC) -c micromllex2.ml

micromllex.ml: micromllex.mll
	$(OCAMLLEX) micromllex.mll

micromllex.cmo: micromllex.ml
#	$(OCAMLLEX) micromllex.mll
	$(OCAMLC) -c micromllex.ml

$(MODULE_RUBRIC).cmo: $(MODULE_COMMON).cmi pre-$(MODULE_RUBRIC).c tests $(IMPLEMENTATIONS) $(LIBRARY_GRADER)
	gcc -E pre-$(MODULE_RUBRIC).c | grep -E -v "#" > $(MODULE_RUBRIC).ml
	$(OCAMLC) -c -I lib $(MODULE_RUBRIC).ml
	$(RM) -f $(MODULE_RUBRIC).ml

clean:
	$(GMAKE) -C lib clean
	$(RM) -f $(STUDENT_CLEAN)

##########################################################################
#these targets are used by staff
##########################################################################

TESTNAME=mp$(MPVERSION)grader

dist: $(DISTFILES)
	$(RM) -rf $(TESTNAME)
	$(MKDIR) $(TESTNAME)
	$(MKDIR) $(TESTNAME)/lib
	$(CP) lib/Makefile lib/*.ml $(TESTNAME)/lib
	$(CP) $(DISTFILES) $(TESTNAME)
	$(TAR) cpf $(TESTNAME).tar $(TESTNAME)
	$(RM) -rf $(TESTNAME)
	$(GZIP) -9 $(TESTNAME).tar

#if you are a student, do not make dist-clean.  it will delete
#your copy of solution.cmo and you will need to download a new
#copy.
dist-clean: clean
	$(RM) -f $(DISTFILES_OBJECT) mp$(MPVERSION)grader* $(MODULE_STUDENT).ml*
	$(RM) -f micromllex.ml micromllex.cm? micromllex.cm? micromllex.cm? micromllex.cm? micromllex.cm? micromllex.cm? micromllex.cm? micromllex2.* micromlparse.cm? micromlyacc.ml micromlyacc.mli $(MODULE_SOLUTION).ml $(MODULE_SOLUTION).mli

