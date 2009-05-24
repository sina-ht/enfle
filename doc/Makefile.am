if MAN_PAGES_ENABLED

man_MANS = enfle.1

%.1 %.7 %.8 : %.xml
	xsltproc -nonet http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl $<

endif # MAN_PAGES_ENABLED

EXTRA_DIST= enfle.xml

clean-local:
	rm -f *~ *.1
