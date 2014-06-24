lib_name := ting

subdirs :=
subdirs += src
subdirs += tests

#build docs only from linux, because there is doxygen
ifeq ($(platform),windows)

else

subdirs += docs

endif


include ./targets.mk

$(possible_targets_list):
	@$(MAKE) $(subdirs) target=$@

.PHONY: $(subdirs)
$(subdirs):
	@$(MAKE) -C $@ $(target)





#use exactly this name for the var, so that dh_auto_install could set it if needed
DESTDIR :=
PREFIX := /usr

install:
#install header files
	@for i in $(patsubst src/%,%,$(shell find src/$(lib_name) -type f -name *.hpp -or -name *.h)); do \
	    install -D src/$$i $(DESTDIR)$(PREFIX)/include/$$i; \
	done
#install library files
	@install -d $(DESTDIR)$(PREFIX)/lib/
	@install src/lib$(lib_name).* $(DESTDIR)$(PREFIX)/lib/
#install docs
	@install -d $(DESTDIR)$(PREFIX)/share/doc/lib$(lib_name)
	@install docs/doxygen/* $(DESTDIR)$(PREFIX)/share/doc/lib$(lib_name)
	