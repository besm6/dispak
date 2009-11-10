SUBDIRS = . dispak simh besmtool disbesm6 examples po sbor/emd2simh
ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = config.rpath m4/ChangeLog

clean-local:
	-rm -rf *~

distclean-local:
	-rm -rf autom4te.cache

log:	.svn
	svn update
	if [ -d /usr/share/locale/en_US.UTF-8 ]; then export LC_TIME=en_US.UTF-8; \
	else export LC_TIME=en_US.utf8; fi && \
	unset LC_ALL && \
	export TZ=UTC0 && \
	svn log > ChangeLog
