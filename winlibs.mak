!IFNDEF VERSION
VERSION=unknown
!ENDIF

!IF "$(PHP_SDK_ARCH)" == "x64"
PLATFORM=x64
BINDIR=bin64
LIBDIR=lib64
!ELSE
PLATFORM=Win32
BINDIR=bin
LIBDIR=lib
!ENDIF

OUTPUT=$(MAKEDIR)\..\ICU-$(VERSION)-$(PHP_SDK_VS)-$(PHP_SDK_ARCH)
ARCHIVE=$(OUTPUT).zip

all:
	git checkout .
	git clean -fdx

	devenv source\allinone\allinone.sln /upgrade
# FIXME we should not disable error checking here:
	-devenv source\allinone\allinone.sln /build "Release|$(PLATFORM)"

	-rmdir /s /q $(OUTPUT)
	xcopy $(BINDIR)\* $(OUTPUT)\bin\*
	del $(OUTPUT)\bin\*test*.dll $(OUTPUT)\bin\*test*.exe
	xcopy /e include\* $(OUTPUT)\include\*
	xcopy $(LIBDIR)\* $(OUTPUT)\lib\*
	del $(OUTPUT)\lib\*test*.*

	del $(ARCHIVE)
	7za a $(ARCHIVE) $(OUTPUT)\*
