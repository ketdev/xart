
BINDIR = .\bin
COCO := $(BINDIR)\coco.exe
ATG := xalang.atg
NAMESPACE := xalang
OUTDIR := gen
COPYDIR := ..\xavm\src\lang

all: 
	@if not exist $(OUTDIR) md $(OUTDIR) fi
	@$(COCO) $(ATG) -namespace $(NAMESPACE) -frames $(BINDIR) -o $(OUTDIR)
	@copy $(OUTDIR)\Scanner.h $(COPYDIR)
	@copy $(OUTDIR)\Scanner.cpp $(COPYDIR)
	@copy $(OUTDIR)\Parser.h $(COPYDIR)
	@copy $(OUTDIR)\Parser.cpp $(COPYDIR)