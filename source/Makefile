# ------------------------------------------------------------------------
#
#  Copyright (C) 2006-2018 Fons Adriaensen <fons@linuxaudio.org>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published
#  by the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# ------------------------------------------------------------------------


# Modify as required.
#
SUFFIX := $(shell uname -m | sed -e 's/^unknown/$//' -e 's/^i.86/$//' -e 's/^x86_64/$/64/')
PREFIX ?= /usr/local
INCDIR ?= $(PREFIX)/include
LIBDIR ?= $(PREFIX)/lib$(SUFFIX)


MAJVERS = 4
MINVERS = 0.3
VERSION = $(MAJVERS).$(MINVERS)


CPPFLAGS += -I. -D_REENTRANT -D_POSIX_PTHREAD_SEMANTICS
CPPFLAGS += -DENABLE_VECTOR_MODE 
CXXFLAGS += -fPIC -Wall -ffast-math -funroll-loops -O2
CXXFLAGS += -march=native
LDLFAGS += 
LDLIBS +=


ZITA-CONVOLVER_SO = libzita-convolver.so
ZITA-CONVOLVER_MAJ = $(ZITA-CONVOLVER_SO).$(MAJVERS)
ZITA-CONVOLVER_MIN = $(ZITA-CONVOLVER_MAJ).$(MINVERS)
ZITA-CONVOLVER_DEP = -lfftw3f -lpthread
ZITA-CONVOLVER_O = zita-convolver.o
ZITA-CONVOLVER_H = zita-convolver.h 


$(ZITA-CONVOLVER_MIN):	$(ZITA-CONVOLVER_O)
	$(CXX) -shared $(LDFLAGS) -Wl,-soname,$(ZITA-CONVOLVER_MAJ) -o $(ZITA-CONVOLVER_MIN) $(ZITA-CONVOLVER_O) $(ZITA-CONVOLVER_DEP)


install:	$(ZITA-CONVOLVER_MIN)
	install -d $(DESTDIR)$(INCDIR)
	install -d $(DESTDIR)$(LIBDIR)
	install -m 644 $(ZITA-CONVOLVER_H) $(DESTDIR)$(INCDIR)
	install -m 755 $(ZITA-CONVOLVER_MIN) $(DESTDIR)$(LIBDIR)
	ldconfig
	ln -sf $(ZITA-CONVOLVER_MIN) $(DESTDIR)$(LIBDIR)/$(ZITA-CONVOLVER_SO)

uninstall:
	rm -rf $(DESTDIR)$(INCDIR)/$(ZITA-CONVOLVER_H)
	rm -rf $(DESTDIR)$(LIBDIR)/libzita-convolver*

clean:
	/bin/rm -f *~ *.o *.a *.d *.so.*

