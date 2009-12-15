#  Copyright (C) 2006-2009 Fons Adriaensen <fons@kokkinizita.net>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published
#  by the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


# Modify as required.
#
PREFIX = /usr/local
SUFFIX := $(shell uname -m | sed -e 's/^unknown/$//' -e 's/^i.86/$//' -e 's/^x86_64/$/64/')
LIBDIR = lib$(SUFFIX)


MAJVERS = 2
MINVERS = 0.0
VERSION = $(MAJVERS).$(MINVERS)
DISTDIR = zita-convolver-$(VERSION)


CPPFLAGS += -Wall -I. -fPIC -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS -O3
CPPFLAGS += -mmmx -msse -mfpmath=sse -ffast-math -funroll-loops -fpermissive
CPPFLAGS += -march=i686
#CPPFLAGS += -DVECTORIZE 

LDLFAGS += 
LDLIBS +=


ZITA-CONVOLVER_SO = libzita-convolver.so
ZITA-CONVOLVER_MAJ = $(ZITA-CONVOLVER_SO).$(MAJVERS)
ZITA-CONVOLVER_MIN = $(ZITA-CONVOLVER_MAJ).$(MINVERS)
ZITA-CONVOLVER_DEP = -lfftw3f -lpthread
ZITA-CONVOLVER_O = zita-convolver.o
ZITA-CONVOLVER_H = zita-convolver.h 


$(ZITA-CONVOLVER_MIN):	$(ZITA-CONVOLVER_O)
	g++ -shared $(LDFLAGS) -Wl,-soname,$(ZITA-CONVOLVER_MAJ) -o $(ZITA-CONVOLVER_MIN) $(ZITA-CONVOLVER_O) $(ZITA-CONVOLVER_DEP)


install:	$(ZITA-CONVOLVER_MIN)
	install -d $(PREFIX)/include
	install -d $(PREFIX)/$(LIBDIR)
	install -m 644 $(ZITA-CONVOLVER_H) $(PREFIX)/include
	install -m 755 $(ZITA-CONVOLVER_MIN) $(PREFIX)/$(LIBDIR)
	ldconfig
	ln -sf $(ZITA-CONVOLVER_MIN) $(PREFIX)/$(LIBDIR)/$(ZITA-CONVOLVER_SO)


clean:
	/bin/rm -f *~ *.o *.a *.d *.so.*

