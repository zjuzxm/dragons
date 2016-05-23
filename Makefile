# /* LICENSE:
#   =========================================================================
#     CMDragons'02 RoboCup F180 Source Code Release
#   -------------------------------------------------------------------------
#     Copyright (C) 2002 Manuela Veloso, Brett Browning, Mike Bowling,
#                        James Bruce; {mmv, brettb, mhb, jbruce}@cs.cmu.edu
#     School of Computer Science, Carnegie Mellon University
#   -------------------------------------------------------------------------
#     This software is distributed under the GNU General Public License,
#     version 2.  If you do not have a copy of this licence, visit
#     www.gnu.org, or write: Free Software Foundation, 59 Temple Place,
#     Suite 330 Boston, MA 02111-1307 USA.  This program is distributed
#     in the hope that it will be useful, but WITHOUT ANY WARRANTY,
#     including MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#   ------------------------------------------------------------------------- */

TOPDIR=.

include $(TOPDIR)/Makefile.common

SUBDIRS = include utils soccer gui simulator logging reality config

#ifneq ($(shell /sbin/lsmod | grep videodev),)
#SUBDIRS += reality
#endif

clean all depend::
	@for i in $(SUBDIRS) ;\
	        do \
		echo "----------------------------------------------------------------------"; \
	        echo "Making" $@ "in $$i..."; \
		if ! $(MAKE) -C $$i $@; then \
			exit 1; \
		fi; \
        done
